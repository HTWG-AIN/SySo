#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>		//struct file_operations
#include <linux/device.h>	// class_create, device_create
#include <linux/errno.h>	// ERESTART: Interrupted system call should be restarted
#include <asm/uaccess.h>	// copy_to_user()
#include <linux/string.h>	// strncpy()
#include <linux/cdev.h>		// cdev_alloc(), cdev_del(), ...
#include <linux/wait.h>		// wait queues
#include <linux/sched.h>	// TASK_INTERRUPTIBLE used by wake_up_interruptible()
#include <linux/slab.h>		// kmalloc(), kfree()
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>

// Metainformation
MODULE_AUTHOR("Stefano Di Martno");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("buffer read write :-P");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 100
#define NUMDEVICES 1
#define DEVNAME "t12buf_threaded"
#define BUFFER_SIZE 350

static struct cdev *cdev = NULL;
static struct class *dev_class;

static wait_queue_head_t wq_read;
static wait_queue_head_t wq_write;

struct mutex mutex_buffer;

static struct workqueue_struct *worker_queue;

// function prototypes
static int __init mod_init(void);
static void __exit mod_exit(void);
static int driver_open(struct inode *inode, struct file *instance);
static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off);
static int driver_close(struct inode *inode, struct file *instance);
static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t * offset);

#define max_line_size 30

#define check_if_thread_is_valid(thread) if(thread == ERR_PTR(-ENOMEM)) \
        { \
                pr_crit("thread could not be created!\n"); \
                return -EIO; \
        }

#define check_memory(pointer) if (pointer == NULL) {\
         pr_alert("Could not allocate memory!\n");\
         return -1;\
         }

#define bytes_to_copy(count) (count) < max_line_size ? (count) : max_line_size
/*
typedef struct {
	char *user;
	size_t size;
} stack_element;*/

typedef struct {
	size_t count;
	struct task_struct *thread_write;
	atomic_t wake_up;
	wait_queue_head_t wait_queue;
	char *user;
} write_data;

typedef struct {
	size_t count;
	struct task_struct *thread_read;
	atomic_t wake_up;
	wait_queue_head_t wait_queue;
	char *user;
} read_data;

typedef struct {
	struct work_struct work;
	write_data *write_data;
	read_data *read_data;
	struct completion on_exit;
	int ret;
} private_data;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = driver_read,
	.write = driver_write,
	.open = driver_open,
	.release = driver_close,
};

#define stackTotalSize 10

typedef struct {
	void *elems;		/* Points to the objects on the stack */
	size_t elemSize;	/* Element size of the element type */
	int currSize;		/* The real amount of objects on the stack */
	struct mutex mutex;
	void (*freefn) (const void *);	/* free function for more complex data types */
} genStack;

static genStack stack;

void GenStackNew(genStack * s, size_t elemSize, void (*freefn) (const void *))
{
	/* Default initialization */
	s->elems = kmalloc(stackTotalSize, elemSize);

	if (s->elems == NULL) {
		pr_alert("Could not init stack!\n");
		return;
	}

	mutex_init(&s->mutex);
	s->elemSize = elemSize;
	s->currSize = 0;
	s->freefn = freefn;
}

int GenStackPush(genStack * s, const void *elemAddr)
{
	char *pTargetAddr;

	mutex_lock(&s->mutex);

	if (s->currSize == stackTotalSize) {
		mutex_unlock(&s->mutex);
		return -1;
	}

	/* Equivalent to &s->elems[s->currSize] */
	pTargetAddr = (char *) s->elems + s->currSize * s->elemSize;

	memcpy(pTargetAddr, elemAddr, s->elemSize);
	s->currSize++;

	mutex_unlock(&s->mutex);

	return 0;
}

int GenStackTryPop(genStack * s, void *elemAddr)
{
	char *pSourceAddr;
	int isEmpty;
	mutex_lock(&s->mutex);

	isEmpty = s->currSize == 0;

	if (isEmpty) {
		mutex_unlock(&s->mutex);
		return -1;
	}

	/* Equivalent to &s->elems[s->currSize - 1] */
	pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

	memcpy(elemAddr, pSourceAddr, s->elemSize);
	s->currSize--;

	mutex_unlock(&s->mutex);

	return 0;
}

void GenStackPop(genStack * s, void *elemAddr)
{
	char *pSourceAddr;

	mutex_lock(&s->mutex);

	/* Equivalent to &s->elems[s->currSize - 1] */
	pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

	memcpy(elemAddr, pSourceAddr, s->elemSize);
	s->currSize--;

	mutex_unlock(&s->mutex);
}

int GenStackEmpty(const genStack * s)
{
	return s->currSize == 0;
}

int GenStackFull(const genStack * s)
{
	return s->currSize == stackTotalSize;
}

void GenStackDispose(genStack * s)
{
	if (s->freefn != NULL && s->currSize > 0) {
		char *pSourceAddr;

		for (; s->currSize > 0; s->currSize--) {
			/* Equivalent to &s->elems[s->currSize - 1] */
			pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

			/* call free function of the client */
			s->freefn(pSourceAddr);
		}
	}

	mutex_destroy(&s->mutex);
	kfree(s->elems);
	s->elems = NULL;
}

static int thread_write(void *write_data)
{
	private_data *data = (private_data *) write_data;

	if (GenStackFull(&stack))	// For debug added
	{
		pr_debug("Producer is going to sleep...\n");
		if (wait_event_interruptible(wq_write, !GenStackFull(&stack)))
			return -ERESTART;
	}

	GenStackPush(&stack, data->write_data->user);

	complete_and_exit(&data->on_exit, 0);
}

static void thread_read(struct work_struct *work)
{
	size_t to_copy;
	size_t count;
	private_data *data = container_of(work, private_data, work);

	count = data->read_data->count;

	if (GenStackEmpty(&stack))	// For debug added
	{
		pr_debug("Consumer is going to sleep...\n");
		if (wait_event_interruptible(wq_read, !GenStackEmpty(&stack))) {
			data->ret = -ERESTART;
			return;
		}
	}

	to_copy = bytes_to_copy(count);

	pr_debug("thread_read: VOR kmalloc\n");

	data->read_data->user = (char *) kmalloc(sizeof(max_line_size), GFP_KERNEL);

	pr_debug("thread_read: NACH kmalloc\n");

	if (data->read_data->user == NULL) {
		pr_alert("Could not allocate memory!\n");
		data->ret = -1;
		return;
	}

	GenStackPop(&stack, data->read_data->user);
	
	to_copy = strlen(data->read_data->user);
	
	data->ret = to_copy;

	pr_debug("Wake producer and read() up...\n");

	atomic_set(&data->read_data->wake_up, 1);

	wake_up_interruptible(&data->read_data->wait_queue);
	wake_up_interruptible(&wq_write);
}

static int driver_open(struct inode *inode, struct file *instance)
{
	private_data *data;

	printk("open() called!\n");

	data = (private_data *) kmalloc(sizeof(private_data), GFP_KERNEL);
	check_memory(data);

	init_completion(&(data->on_exit));

	// Do only kmalloc() on read() and write, if write_data or read_data are NULL!
	data->write_data = NULL;
	data->read_data = NULL;

	instance->private_data = data;

	return 0;
}

static int driver_close(struct inode *inode, struct file *instance)
{
	private_data *data = (private_data *) instance->private_data;

	printk("close() called\n");

	if (data->write_data != NULL) {
		kfree(data->write_data);
	}

	if (data->read_data != NULL) {
		kfree(data->read_data);
	}

	kfree(data);

	return 0;
}



static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off)
{
	unsigned long to_copy;
	private_data *data = (private_data *) instance->private_data;

	if (data->write_data == NULL) {
		data->write_data = (write_data *) kmalloc(sizeof(write_data), GFP_KERNEL);
		check_memory(data->write_data);

		pr_debug("Create producer thread for the first time...\n");
	}

	data->write_data->count = count;
	pr_debug("Write: Call wake_up_process()\n");

	data->write_data->thread_write = kthread_create(thread_write, data, "thread_write");
	check_if_thread_is_valid(data->write_data->thread_write);

	to_copy = bytes_to_copy(count);
	
	pr_debug("Write: bytes to copy: %lu\n", to_copy);
	
	data->write_data->user = kmalloc(sizeof(max_line_size), GFP_KERNEL);
	check_memory(data->write_data->user);
	
	if (to_copy < max_line_size) {
		data->write_data->user[to_copy] = '\0';
	} else {
		data->write_data->user[max_line_size - 1] = '\0';
	}
	
	if (copy_from_user(data->write_data->user, userbuf, to_copy) != 0) {
		pr_crit("Could not copy from user space!!!\n");
		return -1;
	}

	wake_up_process(data->write_data->thread_write);
	wait_for_completion(&data->on_exit);

	pr_debug("Wake consumer up...\n");

	wake_up_interruptible(&wq_read);

	return to_copy;
}

static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t * offset)
{
	unsigned long not_copied, to_copy, copied;
	private_data *data = (private_data *) instance->private_data;
	wait_queue_head_t *wait_queue;
	atomic_t *wake_up;

	if (data->read_data == NULL) {
		data->read_data = (read_data *) kmalloc(sizeof(read_data), GFP_KERNEL);
		check_memory(data->read_data);

		init_waitqueue_head(&data->read_data->wait_queue);
		INIT_WORK(&data->work, thread_read);
		pr_debug("Create consumer thread for the first time...\n");
	}
	// Init read_data
	data->read_data->count = count;
	wait_queue = &data->read_data->wait_queue;
	wake_up = &data->read_data->wake_up;

	atomic_set(wake_up, 0);

	if (!queue_work(worker_queue, &data->work)) {
		pr_crit("queue_work not successful ...\n");
	}

	if (wait_event_interruptible(*wait_queue, atomic_read(wake_up)))
		return -ERESTART;


	if (data->ret < 0) {
		return data->ret;
	}

	to_copy = data->ret;
	not_copied = copy_to_user(user, data->read_data->user, to_copy);
	copied = to_copy - not_copied;

	pr_debug("not_copied: %lu to_copy: %lu. count %d. %lu bytes read\n", not_copied, to_copy, count, copied);

	kfree(data->read_data->user);

	return copied;
}



static void __exit mod_exit(void)
{
	printk(KERN_ALERT "buf_threaded: Goodbye, cruel world\n");
	device_destroy(dev_class, MKDEV(MAJORNUM, 0));
	class_destroy(dev_class);

	if (cdev) {
		cdev_del(cdev);
	}

	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);

	if (worker_queue) {
		destroy_workqueue(worker_queue);
		pr_debug("workqueue destroyed\n");
	}

	GenStackDispose(&stack);
}

static int __init mod_init(void)
{
	dev_t major_nummer = MKDEV(MAJORNUM, 0);

	printk(KERN_ALERT "buf_threaded: Hello, world!\n");

	if (register_chrdev_region(major_nummer, NUMDEVICES, DEVNAME)) {
		pr_warn("Device number 0x%x not available ...\n", MKDEV(MAJORNUM, 0));
		return -EIO;
	}

	pr_info("Device number 0x%x created\n", MKDEV(MAJORNUM, 0));

	cdev = cdev_alloc();

	if (cdev == NULL) {
		pr_warn("cdev_alloc failed!\n");
		goto free_devnum;
	}

	kobject_set_name(&cdev->kobj, DEVNAME);
	cdev->owner = THIS_MODULE;
	cdev_init(cdev, &fops);

	if (cdev_add(cdev, MKDEV(MAJORNUM, 0), NUMDEVICES)) {
		pr_warn("cdev_add failed!\n");
		goto free_cdev;
	}

	dev_class = class_create(THIS_MODULE, DEVNAME);
	device_create(dev_class, NULL, major_nummer, NULL, DEVNAME);

	GenStackNew(&stack, max_line_size, kfree);

	init_waitqueue_head(&wq_read);
	init_waitqueue_head(&wq_write);

	worker_queue = create_singlethread_workqueue("bufThread");

	return 0;

      free_cdev:
	kobject_put(&cdev->kobj);
	cdev = NULL;
      free_devnum:
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
	return -1;
}

module_init(mod_init);
module_exit(mod_exit);

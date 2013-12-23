#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h> //struct file_operations
#include <linux/device.h> // class_create, device_create
#include <linux/errno.h> // ERESTART: Interrupted system call should be restarted
#include <asm/uaccess.h>   // copy_to_user()
#include <linux/string.h> // strncpy()
#include <linux/cdev.h> // cdev_alloc(), cdev_del(), ...
#include <linux/wait.h> // wait queues
#include <linux/sched.h> // TASK_INTERRUPTIBLE used by wake_up_interruptible()
#include <linux/slab.h> // kmalloc(), kfree()
#include <linux/kthread.h>
#include <linux/workqueue.h>

// Metainformation
MODULE_AUTHOR("Stefano Di Martno");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("buffer read write :-P");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 100
#define NUMDEVICES 1
#define DEVNAME "t12buf_threaded"
#define BUFFER_SIZE 10

static struct cdev *cdev = NULL;
static struct class *dev_class;
static char *buffer;
static int read_position = 0;
static int write_position = 0;
static wait_queue_head_t wq;

// function prototypes
static int __init mod_init(void);
static void __exit mod_exit(void);
static int driver_open(struct inode *inode, struct file *instance);
static ssize_t driver_write(struct file *instance, const char __user *userbuf, size_t count, loff_t *off);
static int driver_close(struct inode *inode, struct file *instance);
static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t *offset);

#define free_space() (BUFFER_SIZE - write_position)
#define max_bytes_to_read() (write_position - read_position)

#define check_if_thread_is_valid(thread) if(thread == ERR_PTR(-ENOMEM)) \
	{ \
		pr_crit("thread could not be created!\n"); \
		return -EIO; \
	}

#define check_memory(pointer) if (pointer == NULL) \
		{\
	                pr_alert("Could not allocate memory!\n");\
        	        return -1;\
	        }

typedef struct {
	const char __user *userbuf;
	size_t count;
} write_data;

typedef struct {
	char *user;
	size_t count;
} read_data;

typedef struct {
	struct work_struct work;
	struct task_struct *thread_write;
	write_data *write_data;
	
	struct task_struct *thread_read;
	read_data *read_data;
	
	struct completion on_exit;
	
	int ret;
} private_data;

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.write = driver_write,
	.open= driver_open, 
	.release= driver_close,
};

static struct workqueue_struct *worker_queue;

static int thread_write(void *write_data)
{
	ssize_t to_copy, count;
	char *write_pointer;
	const char __user *userbuf;
	private_data *data = (private_data*) write_data;
	
	count = data->write_data->count;
	userbuf = data->write_data->userbuf;
	
	if (free_space() == 0) 
	{
		pr_debug("Producer is going to sleep...\n");
		if(wait_event_interruptible(wq, free_space() > 0))
			return -ERESTART;
	}
	
	if (count < free_space()) 
	{
		to_copy = count;
	}
	else
	{
		to_copy = free_space();
	}
	
	write_pointer = &buffer[write_position];
	strncpy(write_pointer, userbuf, to_copy);

	write_position += to_copy;

	pr_debug("count: %zu. %zd bytes written\n", count, to_copy);
	pr_debug("Wake consumer up...\n");
	
	wake_up_interruptible(&wq);
	
	data->ret = to_copy;
	
	complete_and_exit(&(data->on_exit), to_copy);
}

static void thread_read(struct work_struct *work)
{
	unsigned long not_copied, to_copy, copied;
	ssize_t count;
	char *read_pointer;
	char *user;
	
	private_data *data =  container_of(work, private_data, work);
	
	count = data->read_data->count;
	user = data->read_data->user;
	
	if (max_bytes_to_read() == 0)
	{
		pr_debug("Consumer is going to sleep...\n");
		if(wait_event_interruptible(wq, max_bytes_to_read() > 0))
			return; //return -ERESTART;
	}

	if(max_bytes_to_read() > count)
	{
		to_copy = count;
	}
	else
	{
		to_copy = max_bytes_to_read();
	}
	
	read_pointer = &buffer[read_position];
	buffer[write_position + 1] = '\0';// FOR DEBUG!!!
	
	pr_debug("read_position %d. pointer: %p. string: %s\n", read_position, read_pointer, read_pointer);
	
	not_copied = copy_to_user(user, read_pointer, to_copy);
	copied = to_copy - not_copied;
	
	read_position += copied;
	
	if (read_position == write_position)
	{
		read_position = 0;
		write_position = 0;
	}
	
	pr_debug("read_position %d. not_copied: %lu to_copy: %lu. count %d. %lu bytes read\n",
		read_position, not_copied, to_copy, count, copied);
	pr_debug("Wake producer up...\n");
	
	wake_up_interruptible(&wq);
	
	data->ret = copied;
	
	//complete_and_exit(&(data->on_exit), copied);
}



static int driver_open(struct inode *inode, struct file *instance)
{
	private_data *data;
	
	printk("open() called!\n");

	data = (private_data*) kmalloc(sizeof(private_data), GFP_KERNEL);
	check_memory(data);
	
	init_completion(&(data->on_exit));
		
	data->thread_write = kthread_create(thread_write, data, "thread_write");
	check_if_thread_is_valid(data->thread_write);
	
	instance->private_data = data;

	return 0;
}

static int driver_close(struct inode *inode, struct file *instance)
{
	private_data *data  = (private_data*) instance->private_data;
	
	printk("close() called\n");
	
	kfree(data);
	
	return 0;
}

static ssize_t driver_write(struct file *instance, const char __user *userbuf, size_t count, loff_t *off)
{
	private_data *data = (private_data*) instance->private_data;
	data->write_data = (write_data*) kmalloc(sizeof(write_data), GFP_KERNEL);
	
	check_memory(data->write_data);
	
	data->write_data->userbuf = userbuf;
	data->write_data->count = count;
	
	wake_up_process(data->thread_write);
	
	wait_for_completion(&(data->on_exit));
	
	kfree(data->write_data);
	
	return data->ret;
}

static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t *offset)
{
	/*private_data *data = (private_data*) instance->private_data;
	data->read_data = (read_data*) kmalloc(sizeof(read_data), GFP_KERNEL);
	
	check_memory(data->read_data);
	
	data->read_data->user = user;
	data->read_data->count = count;
	
	wake_up_process(data->thread_read);
	
	wait_for_completion(&(data->on_exit));
	
	kfree(data->read_data);
	
	return data->ret;*/
	
	private_data *data = (private_data*) instance->private_data;
	data->read_data = (read_data*) kmalloc(sizeof(read_data), GFP_KERNEL);
	
	check_memory(data->read_data);
	
	data->read_data->user = user;
	data->read_data->count = count;
	
	INIT_WORK(&data->work, thread_read);
	
	if(!queue_work(worker_queue, &data->work)) {
		pr_crit( "queue_work not successful ...\n");
	}
	
	return 0;
}



static void __exit mod_exit(void)
{
	printk(KERN_ALERT "buf_threaded: Goodbye, cruel world\n");
	device_destroy(dev_class, MKDEV(MAJORNUM, 0));
	class_destroy(dev_class);
	
	if (cdev) 
    	{
		cdev_del(cdev);
	}
	                                                  
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
	
	kfree(buffer);
	
	if(worker_queue)
	{
		destroy_workqueue(worker_queue);
		pr_debug("workqueue destroyed\n");
	}
}

static int __init mod_init(void)
{
		dev_t major_nummer = MKDEV(MAJORNUM, 0);
			
		printk(KERN_ALERT "buf_threaded: Hello, world!\n");			
				
		if (register_chrdev_region(major_nummer, NUMDEVICES, DEVNAME)) 
		{
			pr_warn("Device number 0x%x not available ...\n" , MKDEV(MAJORNUM, 0));
			return -EIO ;
		} 

		pr_info("Device number 0x%x created\n", MKDEV(MAJORNUM, 0));

		cdev = cdev_alloc();
		if (cdev == NULL) 
		{
			pr_warn("cdev_alloc failed!\n");
			goto free_devnum;
		}

		kobject_set_name(&cdev->kobj, DEVNAME);
		cdev->owner = THIS_MODULE;
		cdev_init(cdev, &fops);
		
		if (cdev_add(cdev, MKDEV(MAJORNUM,0), NUMDEVICES)) 
		{
			pr_warn("cdev_add failed!\n");
			goto free_cdev;
		}

		dev_class = class_create(THIS_MODULE, DEVNAME);
		device_create (dev_class, NULL, major_nummer, NULL, DEVNAME);
		
		buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
		check_memory(buffer);
		
		init_waitqueue_head(&wq);
		
		worker_queue = create_workqueue("bufThread");

		return 0;

	free_cdev:
		kobject_put(&cdev->kobj);
		cdev = NULL;
	free_devnum:
		unregister_chrdev_region(MKDEV(MAJORNUM,0), NUMDEVICES);
	return -1;
}

module_init(mod_init);
module_exit(mod_exit);

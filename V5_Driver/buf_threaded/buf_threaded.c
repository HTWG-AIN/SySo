#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h> //struct file_operations
#include <linux/device.h> // class_create, device_create
#include <linux/errno.h> // ERESTART: Interrupted system call should be restarted
#include <asm/uaccess.h> // copy_to_user()
#include <linux/string.h> // strncpy()
#include <linux/cdev.h> // cdev_alloc(), cdev_del(), ...
#include <linux/wait.h> // wait queues
#include <linux/sched.h> // TASK_INTERRUPTIBLE used by wake_up_interruptible()
#include <linux/slab.h> // kmalloc(), kfree()
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/atomic.h>

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

static atomic_t free_space;
static atomic_t max_bytes_to_read;

static wait_queue_head_t wq_read;
static wait_queue_head_t wq_write;

struct mutex mutex_buffer;
struct mutex write_lock;
struct mutex read_lock;

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

#define check_memory(pointer) if (pointer == NULL) {\
         pr_alert("Could not allocate memory!\n");\
         return -1;\
         }

typedef struct 
{
        size_t count;
        struct task_struct *thread_write;
} write_data;

typedef struct 
{
        size_t count;
        struct task_struct *thread_read;
} read_data;

typedef struct 
{
        write_data *write_data;
        read_data *read_data;
        struct completion on_exit;
        int ret;
} private_data;

static struct file_operations fops = 
{
        .owner= THIS_MODULE,
        .read= driver_read,
        .write = driver_write,
        .open= driver_open,
        .release= driver_close,
};

static int thread_write(void *write_data)
{
        ssize_t to_copy, count;
        private_data *data = (private_data*) write_data;
        
        count = data->write_data->count;
                        
        if (atomic_read(&free_space) == 0)
        {
                pr_debug("Producer is going to sleep...\n");
                if(wait_event_interruptible(wq_write, atomic_read(&free_space) > 0))
                        return -ERESTART;
        }
        
        if (count < atomic_read(&free_space))
        {
                to_copy = count;
        }
        else
        {
                to_copy = atomic_read(&free_space);
        }
        
        data->ret = to_copy;
        
        complete_and_exit(&data->on_exit, to_copy);
}

static int thread_read(void *read_data)
{
        ssize_t to_copy;
        ssize_t count;
        
        private_data *data = (private_data*) read_data;
        
        count = data->read_data->count;

        if (atomic_read(&max_bytes_to_read) == 0)
        {
                pr_debug("Consumer is going to sleep...\n");
                if(wait_event_interruptible(wq_read, atomic_read(&max_bytes_to_read) > 0))
                        return -ERESTART;
        }

        if(atomic_read(&max_bytes_to_read) > count)
        {
                to_copy = count;
        }
        else
        {
                to_copy = atomic_read(&max_bytes_to_read);
        }
	
        data->ret = to_copy;
        
        complete_and_exit(&data->on_exit, 0);
}



static int driver_open(struct inode *inode, struct file *instance)
{
        private_data *data;
        
        printk("open() called!\n");
	
        data = (private_data*) kmalloc(sizeof(private_data), GFP_KERNEL);
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
        private_data *data = (private_data*) instance->private_data;
        
        printk("close() called\n");

	mutex_destroy(&mutex_buffer);
        mutex_destroy(&write_lock);
        
        if (data->write_data != NULL)
        {
	        kfree(data->write_data);
        }
        
        if (data->read_data != NULL)
        {
        	kfree(data->read_data);
        }

        kfree(data);
        
        return 0;
}

static ssize_t driver_write(struct file *instance, const char __user *userbuf, size_t count, loff_t *off)
{
        ssize_t to_copy;
        char *write_pointer;
        private_data *data = (private_data*) instance->private_data;
        
        if (data->write_data == NULL)
        {
		data->write_data = (write_data*) kmalloc(sizeof(write_data), GFP_KERNEL);
		check_memory(data->write_data);
	
		pr_debug("Create producer thread for the first time...\n");
        }
        
	data->write_data->count = count;
	pr_debug("Write: Call wake_up_process()\n");

        data->write_data->thread_write = kthread_create(thread_write, data, "thread_write");
        check_if_thread_is_valid(data->write_data->thread_write);
                
        wake_up_process(data->write_data->thread_write);
        wait_for_completion(&data->on_exit);
        
        to_copy = data->ret;
                
	mutex_lock(&mutex_buffer); // BUFFER LOCK
		write_pointer = &buffer[write_position];
		strncpy(write_pointer, userbuf, to_copy);
	mutex_unlock(&mutex_buffer); // BUFFER UNLOCK
	
        mutex_lock(&write_lock); // WRITE LOCK
		write_position += to_copy;
		
		atomic_set(&free_space, free_space());
		atomic_set(&max_bytes_to_read, max_bytes_to_read());
        mutex_unlock(&write_lock); // WRITE UNLOCK        
	
        pr_debug("count: %zu. %zd bytes written\n", count, to_copy);
        pr_debug("Wake consumer up...\n");
        
        wake_up_interruptible(&wq_read);
        
        return data->ret;
}

static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t *offset)
{
        unsigned long not_copied, to_copy, copied;
        char *read_pointer;
        private_data *data = (private_data*) instance->private_data;
        
        if (data->read_data == NULL)
        {
		data->read_data = (read_data*) kmalloc(sizeof(read_data), GFP_KERNEL);
		check_memory(data->read_data);
		
		pr_debug("Create consumer thread for the first time...\n");
        }
        
	data->read_data->count = count;
	pr_debug("Read: Call wake_up_process()\n");

	data->read_data->thread_read = kthread_create(thread_read, data, "thread_read");
        check_if_thread_is_valid(data->read_data->thread_read);
        
	set_user_nice(data->read_data->thread_read, 2); // 0 is default

        wake_up_process(data->read_data->thread_read);
        wait_for_completion(&data->on_exit);
        
        to_copy = data->ret;
        
        mutex_lock(&read_lock);  // READ LOCK
        
        mutex_lock(&mutex_buffer); // BUFFER LOCK
		read_pointer = &buffer[read_position];
		not_copied = copy_to_user(user, read_pointer, to_copy);
        mutex_unlock(&mutex_buffer); // BUFFER UNLOCK
        
        copied = to_copy - not_copied;
        
        
        
        mutex_lock(&write_lock); // WRITE LOCK

		
		
			read_position += copied;
		
			if (read_position == write_position)
			{
				read_position = 0;
				write_position = 0;
				atomic_set(&free_space, free_space());
			}
			
		
		
		atomic_set(&max_bytes_to_read, max_bytes_to_read());
        
        mutex_unlock(&write_lock); // WRITE UNLOCK
        
        mutex_unlock(&read_lock);  // READ UNLOCK
        
        pr_debug("read_position %d. not_copied: %lu to_copy: %lu. count %d. %lu bytes read\n",
                read_position, not_copied, to_copy, count, copied);
                
        pr_debug("Wake producer up...\n");
        
        wake_up_interruptible(&wq_write);
        
        return copied;
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
                
                atomic_set(&free_space, free_space());
                atomic_set(&max_bytes_to_read, max_bytes_to_read());
                
                init_waitqueue_head(&wq_read);
                init_waitqueue_head(&wq_write);

		mutex_init(&mutex_buffer);
		mutex_init(&write_lock);
		mutex_init(&read_lock);

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

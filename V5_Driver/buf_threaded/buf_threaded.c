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
static ssize_t driver_write(struct file *instanz, const char __user *userbuf, size_t count, loff_t *off);
static int driver_close(struct inode *inode, struct file *instance);
static ssize_t driver_read(struct file *file, char *user, size_t count, loff_t *offset);

#define free_space() (BUFFER_SIZE - write_position)
#define max_bytes_to_read() (write_position - read_position)

// private data structure
typedef struct {
	struct task_struct *thread_id;
	struct completion *work;
} private_data;

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.write = driver_write,
	.open= driver_open, 
	.release= driver_close,
};

static int driver_open(struct inode *inode, struct file *instance)
{
	private_data *data;
	
	printk("open() called!\n");

	data = (private_data*) instance->private_data;

	kfree(data->work);
	kfree(data);
	
	return 0;
}

static int driver_close(struct inode *inode, struct file *instance)
{
	printk("close() called\n");
	
	return 0;
}

static ssize_t driver_write(struct file *instanz, const char __user *userbuf, size_t count, loff_t *off)
{
	ssize_t to_copy;
	char *write_pointer;
	
	if (free_space() == 0) 
	{
		pr_debug("Producer is going to sleep...\n");
		if(wait_event_interruptible(wq, free_space() > 0))
			return -ERESTART;
	}
	
	write_pointer = &buffer[write_position];
	
	if (count < free_space()) 
	{
		to_copy = count;
	}
	else
	{
		to_copy = free_space();
	}
	
	strncpy(write_pointer, userbuf, to_copy);

	write_position += to_copy;

	pr_debug("count: %zu\n", count);	
	pr_debug("%zd bytes written\n", to_copy);
	pr_debug("Wake consumer up...\n");
	
	wake_up_interruptible(&wq);
	
	return to_copy;
}

static ssize_t driver_read(struct file *file, char *user, size_t count, loff_t *offset)
{
	long not_copied, to_copy, copied;
	char *read_pointer;

	if (max_bytes_to_read() == 0)
	{
		pr_debug("Consumer is going to sleep...\n");
		if(wait_event_interruptible(wq, max_bytes_to_read() > 0))
			return -ERESTART;
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
	
	not_copied = copy_to_user(user, read_pointer, to_copy);
	copied = to_copy - not_copied;
	
	read_position += copied;
	
	if (read_position == write_position)
	{
		read_position = 0;
		write_position = 0;
	}
	
	pr_debug("read_position %d\n", read_position);
	pr_debug("%ld bytes read\n", copied);
	
	pr_debug("Wake producer up...\n");
	wake_up_interruptible(&wq);
	
	return copied;
}



static void __exit mod_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
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
			
		printk(KERN_ALERT "Hello, world from buf\n");			
				
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
		
		if (buffer == NULL) {
	                pr_alert("Could not allocate memory!\n");
        	        return -1;
	        }
		
		init_waitqueue_head(&wq);

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

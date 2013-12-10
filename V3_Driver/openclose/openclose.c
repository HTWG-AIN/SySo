#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/device.h> // class_create, device_create

// Metainformation
MODULE_AUTHOR("Stefano Di Martno");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A dummy driver");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 117
#define NUMDEVICES 1
#define DEVNAME "t12openclose"

static int open_count = 0;
static atomic_t v;
static struct cdev *cdev = NULL;
static struct class *dev_class;

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
}

static int driver_open(struct inode *inode, struct file *instance)
{
	printk("open() called!\n");

	if (open_count == 0)
	{
		open_count++;
		pr_debug("open_count: device is locked by process!\n");
	}
	else 
	{
		pr_debug("open_count: device already locked by another process!\n");
		pr_debug("open_count: %d process is accessing this file\n", open_count);
	}
	
	if (atomic_inc_and_test(&v)) 
	{
		pr_debug("atomic_inc_and_test: device is locked by process!\n");
	} 
	else 
	{
		pr_debug("atomic_inc_and_test: device already locked by another process!\n");
		pr_debug("atomic_inc_and_test: %d process is accessing this file\n", open_count);
		atomic_dec_and_test(&v);
		
		return -EBUSY;
	}


	return 0;
}

static ssize_t driver_write(struct file *instanz, const char __user *userbuf, size_t count, loff_t *off)
{

    pr_debug("writing count = %d\n", count);
    return count;
}

static ssize_t driver_read(struct file *file, char *user, size_t count, loff_t *offset)
{
	return 0; //EOF
}

static int driver_close(struct inode *inode, struct file *instance)
{
	printk("close() called\n");
	
	open_count--;
	
	pr_debug("open_count: %d pending processes\n", open_count);
	
	if (atomic_dec_and_test(&v))
	{
		pr_debug("atomic_dec_and_test: %d pending processes\n", open_count);
	}
	
	return 0;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.write = driver_write,
	.open= driver_open, 
	.release= driver_close,
};

static int __init mod_init(void)
{
		dev_t major_nummer = MKDEV(MAJORNUM, 0);
			
		printk(KERN_ALERT "Hello, world\n");			
				
		atomic_set(&v, -1);

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

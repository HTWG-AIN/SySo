#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/cdev.h>

// Metainformation
MODULE_AUTHOR("Stefano Di Martno");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A dummy driver");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 112
#define NUMDEVICES 12
#define DEVNAME "team12"

static int open_count = 0;
static atomic_t v;
static struct cdev *cdev = NULL;

static void __exit mod_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
	
	if (cdev) 
    	{
		cdev_del(cdev);
	}
	                                                  
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
}

static int driver_open(struct inode *inode, struct file *instance)
{
	if (atomic_inc_and_test(&v)) 
	{
		printk("open() called!\n");
		pr_debug("device locked by process!\n");
		pr_debug("%d pending processes\n", open_count);
		open_count++;
	} else 
	{
		pr_debug("device already locked by another process!\n");
		return -EBUSY;
	}


	return 0;
}

static ssize_t driver_read(struct file *file, char *user, size_t count, loff_t *offset)
{
	return 0; //EOF
}

static int driver_close(struct inode *inode, struct file *instance)
{
	printk("close() called\n");
	
	if (atomic_dec_and_test(&v))
	{
		pr_debug("%d pending processes\n", open_count);
		open_count--;
	}
	
	return 0;
}

static struct file_operations fops = {
	.owner= THIS_MODULE,
	.read= driver_read,
	.open= driver_open, 
	.release= driver_close,
};

static int __init mod_init(void)
{
		atomic_set(&v, -1);
		
		if (register_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES, DEVNAME)) 
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h> // kmalloc(), kfree()
#include <asm/uaccess.h>   // copy_to_user()

// Metainformation
MODULE_AUTHOR("Stefano Di Martno");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A dummy driver");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 113
#define NUMDEVICES 2
#define DEVNAME "team12"

static struct cdev *cdev = NULL;

static char hello_world[] = "Hello World\n";  

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
	int minor = iminor(inode);

	instance->private_data = kmalloc( sizeof(int), GFP_KERNEL);
	
	if (instance->private_data == NULL) {
		pr_alert("Could not allocate memory!\n");
		return -1;
	}
	*((int*) (instance->private_data)) = minor;

	return 0;
}

static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t *offset)
{
	int not_copied, to_copy, minor;
	char *data;
	
	minor = *((int*)(instance->private_data));
	
	if (minor == 0)
	{
		to_copy = 1;
		data = "0";
	}
	else if (minor == 1)
	{
		to_copy = strlen(hello_world)+1;
		data = hello_world;
	}
	else
		return -1; // TODO richtige Fehlermeldung!
	
	if(to_copy > count)                                  
		to_copy = count;
		
	not_copied = copy_to_user(user, data, to_copy);
	
	ssize_t len = strlen(data);
	ssize_t sent = len - not_copied;
	ssize_t remaining = to_copy - not_copied;
	
	pr_debug("Module  zero: sent %d bytes to user space \nNot copied: %d bytes\n",
			sent, not_copied);
			
	pr_debug("Remaining: %d\n", remaining);
	
	return remaining;   
}

static int driver_close(struct inode *inode, struct file *instance)
{
	kfree(instance->private_data);
	printk("close() called\n");
	
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

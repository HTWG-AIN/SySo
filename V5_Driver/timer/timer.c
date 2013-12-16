#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> // kmalloc(), kfree()
#include <asm/uaccess.h>   // copy_to_user()
 
 
MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A dummy driver");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 122
#define NUMDEVICES 2
#define DEVNAME "t12timer"



static struct cdev *cdev = NULL;

static struct class *dev_class;
static struct device *device;

static int is_open = 0;

static atomic_t v;

static ssize_t driver_open(struct inode *inode, struct file* file); 
static ssize_t driver_close(struct inode *inode, struct file* file);

static struct file_operations fops = {
	.owner= THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
};

@TODO;


static int __init mod_init(void)
{
    dev_t major_nummer = MKDEV(MAJORNUM, 0);
            
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

    device = device;

    dev_class = class_create(THIS_MODULE, DEVNAME);
    device = device_create (dev_class, NULL, major_nummer, NULL, DEVNAME);


    return 0;

free_cdev:
    kobject_put(&cdev->kobj);
    cdev = NULL;
free_devnum:
    unregister_chrdev_region(MKDEV(MAJORNUM,0), NUMDEVICES);
return -1;
}



static ssize_t driver_open(struct inode *inode, struct file* file) {
    is_open++;
    try_module_get(THIS_MODULE);
    pr_debug("Module fops:device %s was opened from device with minor no %d \n", DEVNAME , iminor(inode));
    return 0;
}


static ssize_t driver_close(struct inode *inode, struct file* file) {
    is_open--;
    module_put(THIS_MODULE);
    pr_debug("Module fops:device %s was closed \n", DEVNAME);
    return 0;
}




 
static void __exit mod_exit(void)
{
	
	if (cdev) {
		cdev_del(cdev);
	}

    device_destroy(dev_class, MKDEV(MAJORNUM, 0));
    class_destroy(dev_class);
	                                                  
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
        printk(KERN_ALERT "Goodbye, cruel world\n");
}
 
module_init(mod_init);
module_exit(mod_exit);

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/slab.h> // kmalloc(), kfree()
#include <asm/uaccess.h>   // copy_to_user()
 
 
MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mutex");

#define MAJORNUM 125
#define NUMDEVICES 2
#define DEVNAME "t12mutex"
#define DEFAULT_SLEEP_TIME_SECONDS 10
#define DEFAULT_SLEEP_TIME_MSECONDS 200



static struct cdev *cdev = NULL;


static struct class *dev_class;
static struct device *device;

static ssize_t driver_open(struct inode *inode, struct file* file); 
static ssize_t driver_close(struct inode *inode, struct file* file);


static int MODULE_EXIT = 0;



DEFINE_MUTEX(open_once);


static struct file_operations fops = {
	.owner= THIS_MODULE,
    .open = driver_open,
    .release = driver_close

};



static int __init mod_init(void)
{
    dev_t major_nummer = MKDEV(MAJORNUM, 0);
            
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


    // Device Specific operations.

    return 0;

free_cdev:
    kobject_put(&cdev->kobj);
    cdev = NULL;
free_devnum:
    unregister_chrdev_region(MKDEV(MAJORNUM,0), NUMDEVICES);
return -1;
}


static ssize_t driver_open(struct inode *inode, struct file* file) {
    pr_debug("Module fops:device %s was opened from device with minor no %d\n", DEVNAME , iminor(inode));

    if (mutex_trylock(&open_once)) {
        pr_debug("Module fops:device %s got lock falling to sleep\n", DEVNAME);
        ssleep(DEFAULT_SLEEP_TIME_SECONDS);
    } else {
        unsigned long start  = jiffies;
        while(!mutex_trylock(&open_once) && !MODULE_EXIT){
            pr_debug("Module fops:device %s waited %d msecs till mutex is unlocked \n", DEVNAME, jiffies_to_msecs(jiffies-start));
            msleep(DEFAULT_SLEEP_TIME_MSECONDS);
        }
        pr_debug("Module fops:device %s got lock after waiting %d msecs falling to sleep\n", DEVNAME, jiffies_to_msecs(jiffies-start));
        ssleep(DEFAULT_SLEEP_TIME_SECONDS);
    }
    return 0;
}


static ssize_t driver_close(struct inode *inode, struct file* file) {


    mutex_unlock(&open_once);

    pr_debug("Module fops:device %s was closed \n", DEVNAME);
    return 0;

}



 
static void __exit mod_exit(void)
{
	
	if (cdev) {
		cdev_del(cdev);
	}

    MODULE_EXIT = 1;

    if (mutex_is_locked(&open_once)) {
        printk(KERN_ALERT "Mutex is still locked\n");
        mutex_unlock(&open_once);

    }
    mutex_destroy(&open_once);

    device_destroy(dev_class, MKDEV(MAJORNUM, 0));
    class_destroy(dev_class);
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
        printk(KERN_ALERT "Goodbye, cruel world\n");
}
 
module_init(mod_init);
module_exit(mod_exit);

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



static struct cdev *cdev = NULL;


static struct class *dev_class;
static struct device *device;

static ssize_t driver_open(struct inode *inode, struct file* file); 
static ssize_t driver_close(struct inode *inode, struct file* file);

static struct timer_list timer;

static void timer_callback(unsigned long data);






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
    setup_timer( &timer, timer_callback, 0);

    return 0;

free_cdev:
    kobject_put(&cdev->kobj);
    cdev = NULL;
free_devnum:
    unregister_chrdev_region(MKDEV(MAJORNUM,0), NUMDEVICES);
return -1;
}


static ssize_t driver_open(struct inode *inode, struct file* file) {
    pr_debug("Module fops:device %s was opened from device with minor no %d \n", DEVNAME , iminor(inode));
    mod_timer( &timer, jiffies + msecs_to_jiffies(200));

    if (mutex_trylock(&open_once)) {
        ssleep(10);
        try_module_get(THIS_MODULE);
    } else {
        pr_debug("Module fops:device %s is locked \n", DEVNAME);
    }



    return 0;

}


static ssize_t driver_close(struct inode *inode, struct file* file) {

    mutex_unlock(&open_once);

    module_put(THIS_MODULE);
    pr_debug("Module fops:device %s was closed \n", DEVNAME);
    return 0;

}

static void timer_callback(unsigned long data) {

    if (mutex_is_locked(&open_once)) {
        pr_debug("Module fops:device %s mutex is Locked at %ld \n", DEVNAME, jiffies);
        
    }

    mod_timer( &timer, jiffies + msecs_to_jiffies(200));
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
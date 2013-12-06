#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h> // kmalloc(), kfree()
#include <asm/uaccess.h>   // copy_to_user()
 
 
MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A dummy driver");
MODULE_SUPPORTED_DEVICE("none");

#define MAJORNUM 115
#define NUMDEVICES 2
#define DEVNAME "null"


static struct cdev *cdev = NULL;

static int is_open = 0;



static ssize_t driver_write(struct file *instanz, const char __user *userbuf, size_t num, loff_t *off); 
static ssize_t driver_open(struct inode *inode, struct file* file); 
static ssize_t driver_close(struct inode *inode, struct file* file);

static struct file_operations fops = {
	.owner= THIS_MODULE,
    .write = driver_write,
    .open = driver_open,
    .release = driver_close

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


static ssize_t driver_write(struct file *instanz, const char __user *userbuf, size_t count, loff_t *off)
{

    pr_debug("writing count = %d", count);
    return count;
}


static ssize_t driver_open(struct inode *inode, struct file* file) {
    if (is_open) return -EBUSY;
    
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
	                                                  
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
    printk(KERN_ALERT "Goodbye, cruel world\n");
}
 
module_init(mod_init);
module_exit(mod_exit);

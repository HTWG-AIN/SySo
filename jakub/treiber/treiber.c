#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
 
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jakub Werner");
MODULE_AUTHOR("SYSO Aufgabe 4");

#define MAJORNUM 112
#define NUMDEVICES 12
#define DEVNAME "jacksdevice"

static struct cdev *driver_info = NULL;
static struct file_operations fops;

 
static int __init ModInit(void)
{
    struct file_operations fop;

    if (register_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES, DEVNAME)) 
    {
        pr_debug("Device number 0x%x not available ...\n" , MKDEV(MAJORNUM, 0));
        return -EIO ;
    } 
    else
    {
        pr_info("Device number 0x%x created", MKDEV(MAJORNUM, 0));
    }

    driver_info = cdev_alloc();
    if (driver_info == NULL) 
    {
        pr_debug("cdev_alloc failed!\n");
        goto free_devnum;
    }

    kobject_set_name(&driver_info->kobj, DEVNAME);
    driver_info->owner = THIS_MODULE;
    cdev_init(driver_info, &fops);
    if (cdev_add(driver_info, MKDEV(MAJORNUM,0), NUMDEVICES)) 
    {
        pr_debug("cdev_add failed!\n");
        goto free_cdev;
    }

    return 0;





free_cdev:
    kobject_put(&driver_info->kobj);
    driver_info = NULL;
free_devnum:
    unregister_chrdev_region(MKDEV(MAJORNUM,0), NUMDEVICES);
    return -1;
}
 
static void __exit ModExit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");

    if (driver_info) 
    {
        cdev_del(driver_info);
    }                                                       
    unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
}
 
module_init(ModInit);
module_exit(ModExit);

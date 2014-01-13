#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>		// kmalloc(), kfree()
#include <asm/uaccess.h>	// copy_to_user()
#include <asm/segment.h>
#include <linux/buffer_head.h>

MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mygpio device");
MODULE_SUPPORTED_DEVICE("none");

#define DEVNAME "mygpio"
#define MAJORNUM 150
#define NUMDEVICES 1

#define GPFSEL0 0xF2200000
#define GPFSEL1 0xF2200004
#define GPFSEL2 0xF2200008
#define GPFSEL3 0xF220000C
#define GPFSEL4 0xF2200010
#define GPFSEL5 0xF2200014
#define GPSET0  0xF220001c
#define GPSET1  0xF2200020
#define GPCLR0  0xF2200028
#define GPCLR1  0xF220002C


#define GPIO_PORT_18
#define GPIO_PORT_25


static struct cdev *cdev = NULL;
static struct class *dev_class;




// function prototypes
static int __init mod_init(void);
static void __exit mod_exit(void);
static int driver_open(struct inode *inode, struct file *instance);
static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off);
static int driver_close(struct inode *inode, struct file *instance);
static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t * offset);


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.read = driver_read,
	.write = driver_write,
	.release = driver_close
};


static ssize_t driver_read(struct file *instance, char *user, size_t count,
			   loff_t * offset)
{
    u32 *ptr = (u32 *)GPFSEL1; u32 old_value;  
    old_value = *ptr;  
	copy_to_user(user, &old_value, sizeof(old_value));
	return 0;
}

static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off) 
{
    return 0;
}

static ssize_t driver_close(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t driver_open(struct inode *inode, struct file *file)
{
	return 0;
}


static int __init mod_init(void)
{

	dev_t major_nummer = MKDEV(MAJORNUM, 0);

	printk(KERN_ALERT "mypgio: Hello, world!\n");

	if (register_chrdev_region(major_nummer, NUMDEVICES, DEVNAME)) {
		pr_warn("Device number 0x%x not available ...\n", MKDEV(MAJORNUM, 0));
		return -EIO;
	}

	pr_info("Device number 0x%x created\n", MKDEV(MAJORNUM, 0));

	cdev = cdev_alloc();

	if (cdev == NULL) {
		pr_warn("cdev_alloc failed!\n");
		goto free_devnum;
	}

	kobject_set_name(&cdev->kobj, DEVNAME);
	cdev->owner = THIS_MODULE;
	cdev_init(cdev, &fops);

	if (cdev_add(cdev, MKDEV(MAJORNUM, 0), NUMDEVICES)) {
		pr_warn("cdev_add failed!\n");
		goto free_cdev;
	}

	dev_class = class_create(THIS_MODULE, DEVNAME);
	device_create(dev_class, NULL, major_nummer, NULL, DEVNAME);


	return 0;

      free_cdev:
	kobject_put(&cdev->kobj);
	cdev = NULL;
      free_devnum:
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
	return -1;
}

static void __exit mod_exit(void)
{
	printk(KERN_ALERT "mypgio: Goodbye, cruel world\n");
	device_destroy(dev_class, MKDEV(MAJORNUM, 0));
	class_destroy(dev_class);

	if (cdev) {
		cdev_del(cdev);
	}

	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);

}

module_init(mod_init);
module_exit(mod_exit);

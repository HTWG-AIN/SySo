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
#include <linux/gpio.h>
#include <linux/io.h>
#include <asm/io.h>

MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mygpio device");
MODULE_SUPPORTED_DEVICE("none");

#define DEVNAME "mygpio"
#define MAJORNUM 150
#define NUMDEVICES 1

#define GPBASE(x) (0xF2200000 | x)

#define GPFSEL0 GPBASE(0x0000)
#define GPFSEL1 GPBASE(0x0004)
#define GPFSEL2 GPBASE(0x0008)
#define GPFSEL3 GPBASE(0x000C)
#define GPFSEL4 GPBASE(0x0010)
#define GPFSEL5 GPBASE(0x0014)
#define GPSET0 GPBASE(0x001C)
#define GPSET1 GPBASE(0x0020)
#define GPCLR0 GPBASE(0x0028)
#define GPCLR1 GPBASE(0x002C)
#define GPLEV0 GPBASE(0x0034)
#define GPLEV1 GPBASE(0x0038)


#define GPIO_PORT_18_SEL
#define GPIO_PORT_25_SEL
#define GPIO_PORT_18_SEL
#define GPIO_PORT_25_SET
#define GPIO_PORT_25_SET

 
static int read_bit(u32 n , int bit) {
    return (n & (1 << bit)) >> bit;
}

static u32 turn_on_bit(u32 n, int bitnum) {
        return n | (1 << bitnum);
}

static u32 turn_off_bit(u32 n, int bitnum) {
        return n & (~ (1 << bitnum));
}      


static struct cdev *cdev = NULL;
static struct class *dev_class;




// function prototypes
static int __init mod_init(void);
static void __exit mod_exit(void);
static int driver_open(struct inode *inode, struct file *instance);
static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off);
static int driver_close(struct inode *inode, struct file *instance);
static ssize_t driver_read(struct file *instance, char *user, size_t count, loff_t * offset);
static int read_bit(u32 n , int bit);


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
    u32 *ptr_port_button = (u32*) GPLEV0;
    u32 old_value_button = readl(ptr_port_button);
    rmb();

    char data[5];

    int size = sprintf(data, "%d\n", read_bit(old_value_button, 25));


	int not_copied = copy_to_user(user, data , size);


	return  size - not_copied;
}

static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off) 
{
    int userval;

    int not_copied = kstrtoint_from_user(userbuf, count, 0, &userval);
    u32 *ptr_port_led = NULL;

    if (userval == 1) {
        ptr_port_led = (u32*)GPSET0;
    }
    if (userval == 0) {
        ptr_port_led = (u32*)GPCLR0;
    }
    if (ptr_port_led == NULL) {
        return count;
    }
    u32 old_value_led = 0x00000000;


    wmb();
    writel(turn_on_bit(old_value_led, 18), ptr_port_led);


    u32 rread = readl(ptr_port_led);
    printk("Read from port 0x%08x\n", rread);

    
    return count - not_copied;
}


static ssize_t driver_close(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t driver_open(struct inode *inode, struct file *file)
{


    u32 *ptr_port_button = (u32*)GPFSEL2;
    u32 old_value_button = readl(ptr_port_button);
    rmb();
    // clear its fuer port 25 = FFFC7FFF
    u32 cleared_ports = old_value_button & 0xFFFC7FFF;

    // writing 
    printk("Cleared ports: 0x%08x, ptr-button: %p\n", cleared_ports, ptr_port_button);
    wmb();
    writel(cleared_ports, ptr_port_button);




    u32 *ptr_port_led = (u32*)GPFSEL1;
    u32 old_value_led = readl(ptr_port_led);
    rmb();
    // clear its fuer port 18 = F8FFFFFF
    u32 cleared_ports_led = old_value_led & 0xF8FFFFFF;
    u32 new_ports_led = cleared_ports_led | 0x01000000;
    printk("Cleared ports: 0x%08x, new-ports: 0x%08x ptr-led: %p\n", cleared_ports, new_ports_led, ptr_port_led);
    wmb();
    writel(new_ports_led, ptr_port_led);
    


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

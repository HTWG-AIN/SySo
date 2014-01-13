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


#define DEVNAME "t12mygpio"




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
	return 0;
}

static ssize_t driver_write(struct file *instance, const char __user * userbuf, size_t count, loff_t * off) 
{
    return 0
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

	if (register_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES, DEVNAME)) {
		pr_warn("Device number 0x%x not available ...\n",
			MKDEV(MAJORNUM, 0));
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

	device = device;

	dev_class = class_create(THIS_MODULE, DEVNAME);
	device = device_create(dev_class, NULL, major_nummer, NULL, DEVNAME);

	// Device Specific operations.

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
	if (cdev) {
		cdev_del(cdev);
	}

	device_destroy(dev_class, MKDEV(MAJORNUM, 0));
	class_destroy(dev_class);
	unregister_chrdev_region(MKDEV(MAJORNUM, 0), NUMDEVICES);
}

static int gpio_set_value(char *port, int value){
    FILE *gpio_port = NULL;
    size_t write_count;
    int fclose_ret = 0;
    char value_string_path[100];
    char int_string_value[4];
    sprintf(int_string_value, "%d", value);
    sprintf(value_string_path, "/sys/class/gpio/gpio%s/value", port);
    gpio_port = fopen(value_string_path, "w");
    if (gpio_port == NULL) {
        printf("failed get write permission to set port %s's value\n", port);
        return -1;
    }
    write_count = fwrite(&int_string_value, sizeof(char), 1, gpio_port);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(gpio_port);
    if (fclose_ret == EOF) {
        printf("failed to close value file\n");
        return -1;
    }

    return 1;
}


static int gpio_set_direction(char *port, char *direction) {
    FILE *gpio_port = NULL;
    size_t write_count;
    int fclose_ret = 0;
    char direction_string_path[100];
    sprintf(direction_string_path, "/sys/class/gpio/gpio%s/direction", port);
    gpio_port = fopen(direction_string_path, "w");
    if (gpio_port == NULL) {
        printf("failed get write permission to set port %s's direction", port);
        return -1;
    }
    write_count = fwrite(direction, sizeof(char), strlen(direction) + 1, gpio_port);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(gpio_port);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;

}
static int gpio_export_port(char *port) {
    FILE *export = NULL;
    size_t write_count;
    int fclose_ret = 0;
    export = fopen("/sys/class/gpio/export", "w");
    if (export == NULL) {
        printf("failed get write permission to export port %s", port);
        return -1;
    }
    write_count = fwrite(port, sizeof(char), strlen(port) + 1, export);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(export);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;
}

static int gpio_unexport_port(char *port) {
    FILE *unexport = NULL;
    size_t write_count;
    int fclose_ret = 0;
    unexport = fopen("/sys/class/gpio/unexport", "w");
    if (unexport == NULL) {
        printf("failed get write permission to unexport port %s", port);
        return -1;
    }
    write_count = fwrite(port, sizeof(char), strlen(port) + 1, unexport);
    if (write_count > 0 ) {
        ;
    }

    fclose_ret = fclose(unexport);
    if (fclose_ret == EOF) {
        return -1;
    }

    return 1;
}

module_init(mod_init);
module_exit(mod_exit);

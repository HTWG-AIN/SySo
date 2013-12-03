#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
 
 
MODULE_LICENSE("GPL");

static unsigned int chardev_major = 12;
static const char * chardev_name = "so its the name";
 
static int __init ModInit(void)
{
    struct file_operations fop;

    register_chrdev(chardev_major, chardev_name, &fop);
    printk(KERN_ALERT "Hello, world\n");
    return 0;
}
 
static void __exit ModExit(void)
{
    unregister_chrdev(chardev_major, chardev_name);
    printk(KERN_ALERT "Goodbye, cruel world\n");
}
 
module_init(ModInit);
module_exit(ModExit);

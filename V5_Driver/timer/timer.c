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
MODULE_DESCRIPTION("Timer");
MODULE_SUPPORTED_DEVICE("none");

#define DEVNAME "t12timer"



static struct timer_list timer;

static void timer_callback(unsigned long data);

typedef struct timer_data_t {
    unsigned long jiffies_stamp;
 } timer_data;


static int __init mod_init(void)
{
    timer_data *data;
            
    
    data = kmalloc(sizeof(timer_data), GFP_KERNEL);
    data->jiffies_stamp = 0;
    setup_timer( &timer, timer_callback, (unsigned long) data);
    mod_timer( &timer, jiffies + msecs_to_jiffies(2000));

    return 0;

}

static void timer_callback(unsigned long data) {
    timer_data *d = (timer_data *) data;
    unsigned long time_diff = jiffies - d->jiffies_stamp;
    unsigned long min = 0, max = 0;

    min = min(min,time_diff);
    max = max(max,time_diff);



    if (d->jiffies_stamp)
        printk(DEVNAME " called at (%ld) time since the last call = %ld, clockcyles min: %ld, max %ld.\n", jiffies/HZ, time_diff/HZ,min/HZ, max/HZ);
    else 
        printk(DEVNAME " called first time (%ld).\n", jiffies);

    mod_timer( &timer, jiffies + msecs_to_jiffies(2000));
    d->jiffies_stamp = jiffies;
}





 
static void __exit mod_exit(void)
{
	
    int ret;



    ret = del_timer( &timer );
    if (ret) printk("The timer is still in use...\n");
}
 
module_init(mod_init);
module_exit(mod_exit);

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h> // kmalloc(), kfree()
 
 
MODULE_AUTHOR("Jakub Werner");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Workqueue");

#define DEVNAME "t12workqueue"
#define WORKQUEUE_SIZE 10



static void workqueue_callback(struct work_struct *data);

typedef struct timer_data_t {
    unsigned long jiffies_stamp;
 } timer_data;

typedef struct {
    struct work_struct work;
    timer_data *timer;
    int work_number;
} work_data;


static struct workqueue_struct *wq;

static work_data *work[WORKQUEUE_SIZE];




static int __init mod_init(void)
{

    int i = 0;

    wq = create_workqueue("my_workqueue");
    if (wq) {
        for (i = 0; i < WORKQUEUE_SIZE; i++) {
            work[i] = (work_data *) kmalloc(sizeof(work_data), GFP_KERNEL);
            if (work[i]) {
               INIT_WORK( (struct work_struct *) work[i], workqueue_callback);
               work[i]->timer = (timer_data *) kmalloc(sizeof(timer_data), GFP_KERNEL);
               if (work[i]->timer) {
                   work[i]->timer->jiffies_stamp = 0;
                   work[i]->work_number = i;
                   queue_work(wq, (struct work_struct*) work[i]);
               }
            }
         }

    }


    return 0;
}

static void workqueue_callback(struct work_struct *data) {
    work_data *my_data = (work_data*) data;
    timer_data *d = my_data->timer;
    unsigned long time_diff = jiffies - d->jiffies_stamp;
    unsigned long min = 0, max = 0;

    min = min(min,time_diff);
    max = max(max,time_diff);


    if (d->jiffies_stamp)
        printk(DEVNAME "-%d called at (%ld) time since the last call = %ld, clockcyles min: %ld, max %ld.\n",my_data->work_number, jiffies/HZ, time_diff/HZ,min/HZ, max/HZ);
    else 
        printk(DEVNAME "-%d called first time (%ld).\n",my_data->work_number, jiffies);

    d->jiffies_stamp = jiffies;

    ssleep(2);
    queue_work(wq, (struct work_struct*) data);


    return;
}


static void __exit mod_exit(void)
{

    int i = 0;
    pr_debug("mod_exit called\n");
    for (i = 0; i < WORKQUEUE_SIZE; i++) { 
        cancel_work_sync(&(work[i]->work));
    }
    flush_scheduled_work();
    flush_workqueue(wq);
    destroy_workqueue(wq);

    return;
	
}
 
module_init(mod_init);
module_exit(mod_exit);

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/sched.h> // TASK_INTERRUPTIBLE used by wake_up_interruptible()
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefano Di Martno");
MODULE_DESCRIPTION("kthread sample");
MODULE_SUPPORTED_DEVICE("none");

static struct task_struct *thread_id;
static wait_queue_head_t wq;
static DECLARE_COMPLETION(on_exit);

static int thread_code(void *data)
{
	unsigned long timeout;

	//daemonize("MySySoKThread");
	allow_signal(SIGTERM);

	while (kthread_should_stop() == 0) 
	{
		timeout = 2 * HZ; // wait 2 second
		timeout = wait_event_interruptible_timeout(wq, (timeout == 0), timeout);

		pr_debug("thread_code: woke up ...\n");

		if(timeout == -ERESTARTSYS) 
		{
			pr_info("got signal, break\n");
			break;
		}
	}

	complete_and_exit(&on_exit, 0);
}

static int __init kthread_init(void)
{
	printk(KERN_ALERT "kthread: Hello, world\n");

	init_waitqueue_head(&wq);
	thread_id = kthread_create(thread_code, NULL, "MySySoKThread");

	if(thread_id == 0)
	{
		pr_crit("kthread could not be created!\n");
		return -EIO;
	}

	wake_up_process(thread_id);
	
	return 0;
}

static void __exit kthread_exit(void)
{
	kill_pid(task_pid(thread_id), SIGTERM, 1);
	wait_for_completion(&on_exit);
	
	printk(KERN_ALERT "kthread: Goodbye, cruel world\n");
}

module_init(kthread_init);
module_exit(kthread_exit);

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

static int thread_id=0;
static wait_queue_head_t wq;
static DECLARE_COMPLETION(on_exit);
static struct task_struct *my_kthread;

static int thread_code(void *data)
{
	unsigned long timeout;

	//daemonize("MySySoKThread");
	allow_signal(SIGTERM);

	while (1) 
	{
		timeout = 2 * HZ; // wait 2 second
		timeout = wait_event_interruptible_timeout(wq, (timeout == 0), timeout);

		pr_info("thread_code: woke up ...\n");

		if(timeout == -ERESTARTSYS || kthread_should_stop()) 
		{
			printk("got signal, break\n");
			break;
		}
	}

	thread_id = 0;
	complete_and_exit(&on_exit, 0);
}

static int __init kthread_init(void)
{
	printk(KERN_ALERT "Hello, world\n");

	init_waitqueue_head(&wq);
	// thread_id = kernel_thread(thread_code, NULL, CLONE_KERNEL);

	my_kthread = kthread_run(thread_code, NULL, "MySySoKThread");

	if(my_kthread == ERR_PTR(-ENOMEM))
	{
		pr_crit("kthread could not be created!\n");
		return -EIO;
	}
	
	return 0;
}

static void __exit kthread_exit(void)
{
	int ret;
	printk(KERN_ALERT "Goodbye, cruel world\n");

	//if(thread_id) kill_proc(thread_id, SIGTERM, 1);
	
	if(thread_id)
	{ 
		ret = kthread_stop(my_kthread);
		
		if(ret == -EINTR)
		{
			pr_crit("kthread could not be stoped!\n");
		}
		else if (ret < 0)
		{
			pr_crit("kthread stoped with error %d!\n", ret);
		}
		else
		{
			pr_debug("kthread stopped!\n");
		}
	}
	
	wait_for_completion( &on_exit );
}

module_init(kthread_init);
module_exit(kthread_exit);

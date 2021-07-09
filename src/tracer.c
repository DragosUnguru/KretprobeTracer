#include "tracer.h"

MODULE_DESCRIPTION("kprobe tracer");
MODULE_AUTHOR("Dragos Unguru");
MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry;

static int list_proc_show(struct seq_file *m, void *v)
{
	struct list_head *p = NULL;
	struct list_node *node;
	char buff[BUFSIZ];

	/* Start critical section */
	read_lock(&lock);

	seq_puts(m, table_header);

	list_for_each(p, &head) {
		/* Populate buffer with data from list */
		node = list_entry(p, struct list_node, list);
		sprintf(buff, "%d %d %d %d %d %d %d %d %d %d\n",
				node->pid, node->kmalloc, node->kfree,
				node->kmalloc_mem, node->kfree_mem,
				node->schedule, node->up, node->down,
				node->lock, node->unlock);

		seq_puts(m, buff);
	}

	/* End critical section */
	read_unlock(&lock);
	return 0;
}

static int tracer_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static long tracer_cdev_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int remains;
	pid_t *pid;

	switch (cmd) {
	case TRACER_ADD_PROCESS:
		add_process((pid_t) arg);
		break;
	case TRACER_REMOVE_PROCESS:
		remove_process((pid_t) arg);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct proc_ops tracer_ops = {
	.proc_open = tracer_proc_open,
	.proc_read = seq_read,
};

static const struct file_operations tracer_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl = tracer_cdev_ioctl,
};

static struct miscdevice misc_dev = {
	.minor	= TRACER_DEV_MINOR,
	.name	= TRACER_DEV_NAME,
	.fops	= &tracer_fops,
};

static int tracer_init(void)
{
	int err;
	int i;
	int j;

	INIT_LIST_HEAD(&head);
	INIT_LIST_HEAD(&mem_head);

	// Initialize probes
	for (i = 0; i < func_no; ++i) {
		probes[i].kp.symbol_name = probed_func_names[i];
		err = register_kretprobe(&probes[i]);

		if (err < 0) {
			pr_info("failed to init probe %s\n",
				probed_func_names[i]);
			goto unregister;
		}
	}

	// Initialize char device and return error code
	err = misc_register(&misc_dev);
	if (err < 0) {
		pr_info("failed to init /dev/tracer\n");
		goto unregister;
	}

	proc_entry = proc_create(TRACER_DEV_NAME, 0, NULL, &tracer_ops);
	if (proc_entry != NULL)
		return 0;

	/* proc_create failed, deregister misc and kretprobes */
	pr_info("failed to init /proc/tracer\n");
	misc_deregister(&misc_dev);
unregister:

	/* Unregister all devices up to i that managed
	 * to be successfully registered
	 */
	for (j = 0; j < i; ++j)
		unregister_kretprobe(&probes[j]);

	return -EFAULT;
}

static void tracer_exit(void)
{
	int i;

	for (i = 0; i < func_no; ++i)
		unregister_kretprobe(&probes[i]);

	purge_lists();
	misc_deregister(&misc_dev);
	proc_remove(proc_entry);
}

module_init(tracer_init);
module_exit(tracer_exit);

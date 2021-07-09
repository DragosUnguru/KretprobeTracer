#include "kretprobe.h"

/* Functions to probe */
const char *probed_func_names[] = { "__kmalloc", "kfree", "schedule",
		"up", "down_interruptible", "mutex_lock_nested",
		"mutex_unlock", "do_exit" };
EXPORT_SYMBOL(probed_func_names);

const int func_no = ARRAY_SIZE(probed_func_names);
EXPORT_SYMBOL(func_no);

int kmalloc_probe_entry_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	struct memory_map *data;

	/* Update kmalloc call counter */
	update_node(instance->task->pid, KMALLOC_FIELD, 1);

	/* So far, we only have the size. Alloc entry without an address
	 * and save the size on the instance
	 */
	data = (struct memory_map *) instance->data;
	data->size = regs->ax;

	/* Also update kmalloc_mem for the current pid */
	update_node(instance->task->pid, KMALLOC_MEM_FIELD, regs->ax);

	return 0;
}

int kmalloc_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	struct memory_map *data;

	/* Now we also get the return address. Add it to our structure
	 * and add the structure to the global list
	 */
	data = (struct memory_map *) instance->data;

	data->address = regs->ax;
	map_address(data);

	return 0;
}

int kfree_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	unsigned long size_freed;

	size_freed = get_and_pop_address(regs->ax);

	update_node(instance->task->pid, KFREE_MEM_FIELD, size_freed);
	update_node(instance->task->pid, KFREE_FIELD, 1);

	return 0;
}

int schedule_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	update_node(instance->task->pid, SCHEDULE_FIELD, 1);
	return 0;
}

int up_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	update_node(instance->task->pid, UP_FIELD, 1);
	return 0;
}

int down_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	update_node(instance->task->pid, DOWN_FIELD, 1);
	return 0;
}

int lock_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	update_node(instance->task->pid, LOCK_FIELD, 1);
	return 0;
}

int unlock_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	update_node(instance->task->pid, UNLOCK_FIELD, 1);
	return 0;
}

int exit_probe_handler(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	remove_process(instance->task->pid);
	return 0;
}

struct kretprobe probes[] = {
	/* __kmalloc probe */
	{
		.entry_handler = kmalloc_probe_entry_handler,
		.handler = kmalloc_probe_handler,
		.data_size = sizeof(struct memory_map),
		.maxactive = MAX_ACTIVE,
	},

	/* kfree probe */
	{
		.entry_handler = kfree_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* schedule probe */
	{
		.entry_handler = schedule_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* up probe */
	{
		.entry_handler = up_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* down probe */
	{
		.entry_handler = down_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* lock probe */
	{
		.entry_handler = lock_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* unlock probe */
	{
		.entry_handler = unlock_probe_handler,
		.maxactive = MAX_ACTIVE,
	},

	/* Also probe "do_exit()" calls in order to determine
	 * when a process terminates to remove it from our list
	 */
	{
		.handler = exit_probe_handler,
		.maxactive = MAX_ACTIVE,
	}
};
EXPORT_SYMBOL(probes);

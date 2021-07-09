#include "list_util.h"

/* List of processes and their probed metrics */
DEFINE_RWLOCK(lock);
EXPORT_SYMBOL(lock);

struct list_head head;
EXPORT_SYMBOL(head);

/* List of alloc'd memory addresses and their sizes */
DEFINE_SPINLOCK(mem_map_lock);

struct list_head mem_head;
EXPORT_SYMBOL(mem_head);

void map_address(struct memory_map *data)
{
	struct memory_map *list_entry;

	list_entry = kmalloc(sizeof(*list_entry), GFP_ATOMIC);
	if (list_entry == NULL)
		return;

	list_entry->size = data->size;
	list_entry->address = data->address;

	spin_lock(&mem_map_lock);
	list_add(&list_entry->list, &mem_head);
	spin_unlock(&mem_map_lock);
}
EXPORT_SYMBOL(map_address);

unsigned long get_and_pop_address(unsigned long addr)
{
	struct list_head *p = NULL, *q = NULL;
	struct memory_map *data;
	unsigned long address;
	unsigned long mem_size = 0;

	/* Start critical section */
	spin_lock(&mem_map_lock);

	list_for_each_safe(p, q, &mem_head) {
		data = list_entry(p, struct memory_map, list);
		address = data->address;

		if (address == addr) {
			mem_size = data->size;
			list_del(p);

			spin_unlock(&mem_map_lock);
			return mem_size;
		}
	}

	/* End critical section */
	spin_unlock(&mem_map_lock);
	return 0;
}
EXPORT_SYMBOL(get_and_pop_address);

static struct list_node *init_new_node(pid_t pid) {
	struct list_node *node;

	node = kmalloc(sizeof(*node), GFP_KERNEL);
	if (node == NULL)
		return NULL;

	node->pid = pid;
	node->kmalloc = 0;
	node->kfree = 0;
	node->kmalloc_mem = 0;
	node->kfree_mem = 0;
	node->schedule = 0;
	node->up = 0;
	node->down = 0;
	node->lock = 0;
	node->unlock = 0;

	return node;
}

static struct list_node *get_process(pid_t pid)
{
	struct list_head *p = NULL;
	struct list_node *node;

	read_lock(&lock);

	list_for_each(p, &head) {
		node = list_entry(p, struct list_node, list);

		if (node->pid == pid) {
			read_unlock(&lock);
			return node;
		}
	}

	read_unlock(&lock);
	return NULL;
}

void update_node(pid_t pid, int field_offt, int value)
{
	struct list_node *node;
	u32 *target_field;

	node = get_process(pid);

	if (node != NULL)  {
		write_lock(&lock);

		target_field = (u32 *) node + field_offt;
		*target_field = *target_field + value;

		write_unlock(&lock);
	}
}
EXPORT_SYMBOL(update_node);

void add_process(pid_t pid)
{
	struct list_node *node;

	node = get_process(pid);

	if (node != NULL)
		/* PID already in list */
		return;

	node = init_new_node(pid);
	if (node == NULL)
		return;

	/* Safely add to list */
	write_lock(&lock);
	list_add(&node->list, &head);
	write_unlock(&lock);
}
EXPORT_SYMBOL(add_process);

void remove_process(pid_t pid)
{
	struct list_head *p = NULL, *q = NULL;
	struct list_node *node;

	/* Start critical section */
	write_lock(&lock);

	list_for_each_safe(p, q, &head) {
		node = list_entry(p, struct list_node, list);

		if (node->pid == pid) {
			list_del(p);

			write_unlock(&lock);
			return;
		}
	}
	/* End critical section */
	write_unlock(&lock);
}
EXPORT_SYMBOL(remove_process);
NOKPROBE_SYMBOL(remove_process);

void purge_lists(void)
{
	struct list_head *p = NULL, *q = NULL;
	struct list_node *node;
	struct memory_map *data;

	/* Free list of processes and probed data */
	write_lock(&lock);

	list_for_each_safe(p, q, &head) {
		node = list_entry(p, struct list_node, list);
		list_del(p);
		kfree(node);
	}
	write_unlock(&lock);

	/* Free list of addresses to sizes mapping */
	spin_lock(&mem_map_lock);

	list_for_each_safe(p, q, &mem_head) {
		data = list_entry(p, struct memory_map, list);
		list_del(p);
		kfree(data);
	}
	spin_unlock(&mem_map_lock);
}
EXPORT_SYMBOL(purge_lists);

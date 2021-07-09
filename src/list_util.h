#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/kprobes.h>

/* Field offsets in the node's struct */
#define KMALLOC_FIELD 		1
#define KFREE_FIELD			2
#define KMALLOC_MEM_FIELD 	3
#define KFREE_MEM_FIELD		4
#define SCHEDULE_FIELD		5
#define UP_FIELD			6
#define DOWN_FIELD			7
#define LOCK_FIELD			8
#define UNLOCK_FIELD		9

/* List node describing the probed data
 * for each PID
 */
struct list_node {
	pid_t pid;
	u32 kmalloc;
	u32 kfree;
	u32 kmalloc_mem;
	u32 kfree_mem;
	u32 schedule;
	u32 up;
	u32 down;
	u32 lock;
	u32 unlock;
	struct list_head list;
};

/* List node containing the association
 * between starting address of the alloc'd
 * space and it's size, as a result of the
 * __kmalloc call
 */
struct memory_map {
	unsigned long address;
	unsigned long size;
	struct list_head list;
};

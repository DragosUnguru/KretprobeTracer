#include <linux/kprobes.h>
#include "list_util.h"

#define MAX_ACTIVE  64

/* Expects a list of processes with its update and remove functions */
extern struct list_head head;

extern void update_node(pid_t pid, int field_offt, int value);
extern void remove_process(pid_t pid);

/* Expects a list of (memory_address, size_alloc'd) mappings */
extern struct list_head mem_head;

extern void map_address(struct memory_map *data);
extern unsigned long get_and_pop_address(unsigned long addr);
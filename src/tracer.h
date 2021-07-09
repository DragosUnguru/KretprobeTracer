/*
 * SO2 kprobe based tracer header file
 *
 * this is shared with user space
 */

#ifndef TRACER_H__
#define TRACER_H__ 1

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <asm/ioctl.h>

#include "kretprobe.h"

#ifndef __KERNEL__
#include <sys/types.h>
#endif /* __KERNEL__ */

const char table_header[] = "PID kmalloc kfree "
		"kmalloc_mem kfree_mem sched "
		"up down lock unlock\n";

#ifndef BUFSIZ
#define BUFSIZ					128
#endif

#define TRACER_DEV_MINOR		42
#define TRACER_DEV_NAME			"tracer"

#define TRACER_ADD_PROCESS		_IOW(_IOC_WRITE, 42, pid_t)
#define TRACER_REMOVE_PROCESS	_IOW(_IOC_WRITE, 43, pid_t)

/* List dependencies */
extern rwlock_t lock;
extern struct list_head head;
extern struct list_head mem_head;

extern void add_process(pid_t pid);
extern void remove_process(pid_t pid);
extern void purge_lists(void);

/* Kretprobe dependencies */
extern const char *probed_func_names[];
extern const int func_no;

extern struct kretprobe probes[];

#endif /* TRACER_H_ */
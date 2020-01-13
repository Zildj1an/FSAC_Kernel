/*
 *  FSAC initialization code. 
 *  @author Carlos Bilbao Muñoz 
 *  cbilbao@ucm.es
 *  Sept 2019-2020
 */

#ifndef _LINUX_FSAC_H_
#define _LINUX_FSAC_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stop_machine.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/rwsem.h>
#include <fsac/fsac_plugin.h>
#include <fsac/fsac_proc.h>


#define is_fsac(t)    ((t)->policy == SCHED_FSAC)

static inline lt_t fsac_clock(void) {
	return ktime_to_ns(ktime_get());
}

long fsac_admit_task(struct task_struct *tsk);
int fsac_is_real_time(struct task_struct *tsk);
void fsac_exit_task(struct task_struct* tsk);
void fsac_do_exit(struct task_struct *tsk); /* Called by ln.745 /kernel/exit.c */

void fsac_plugin_switch_disable(void);
void fsac_plugin_switch_enable(void);

// TODO? Not sure if something left


#endif

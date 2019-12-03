#ifndef _LINUX_FSAC_H_
#define _LINUX_FSAC_H_

/*
	FSAC initialization code.
	@author Carlos Bilbao Muñoz
	cbilbao@ucm.es
	2019
*/
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/fsac_plugin.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#define is_fsac(t)    ((t)->policy == SCHED_FSAC)

static inline lt_t fsac_clock(void) {
	return ktime_to_ns(ktime_get());
}

long fsac_admit_task(struct task_struct *tsk);
void fsac_exit_task(struct task_struct* tsk);

//TODO switch_sched_plugin y demas


#endif
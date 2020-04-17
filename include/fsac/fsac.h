/*
 *  FSAC initialization code.
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 *  Sept 2019-2020
 */

#ifndef _LINUX_FSAC_H_
#define _LINUX_FSAC_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stop_machine.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/rwsem.h>
#include <linux/sysrq.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <fsac/fsac_plugin.h>
#include <fsac/fsac_proc.h>
#include <fsac/fsac_param.h>

#ifdef CONFIG_RELEASE_MASTER
extern atomic_t release_master_cpu;
#endif


static inline int in_list(struct list_head* list)
{

	return !( (list->next == LIST_POISON1 && list->prev == LIST_POISON2) || (list->next == list && list->prev == list));
}


#define is_fsac(t)    ((t)->policy == SCHED_FSAC)

//#define tsk_fsac(t)   (&(t)->fsac_param)

extern inline
struct fsac_param* tsk_fsac(struct task_struct* t)
{
	return &(t->fsac_param);
}

struct task_struct* __waitqueue_remove_first(wait_queue_head_t *wq);

#define NO_CPU			0xffffffff

void fsac_fork(struct task_struct *p);

void fsac_exec(void);

void fsac_clear_state(struct task_struct *dead_tsk);

void exit_fsac(struct task_struct *dead_tsk);

static inline unsigned long long fsac_clock(void) 
{
	return ktime_to_ns(ktime_get());
}

long fsac_admit_task(struct task_struct *tsk);

int  fsac_is_real_time(struct task_struct *tsk);

int  fsac_is_rt(void);

void fsac_exit_task(struct task_struct* tsk);

void fsac_do_exit(struct task_struct *tsk); /* Called by ln.745 /kernel/exit.c */

void fsac_dealloc(struct task_struct *tsk);

void fsac_plugin_switch_disable(void);

void fsac_plugin_switch_enable(void);

extern inline void fsac_task_new(struct task_struct *p,int a,int b);

extern inline void fsac_finish_switch(struct task_struct *p);

extern inline struct task_struct* fsac_schedule_prev(struct task_struct *prev);

extern inline int  fsac_should_wait_for_stack(struct task_struct *next);

extern inline void fsac_next_became_invalid(struct task_struct *next);

extern inline void fsac_task_wake_up(struct task_struct *p);

extern inline void fsac_task_block(struct task_struct *p);

extern inline int  fsac_post_migration_validate(struct task_struct *next);

extern inline void fsac_task_block(struct task_struct *p);

/* Done at /fsac/fsac_plugin.c */
void preempt_if_preemptable(struct task_struct* t, int cpu);

static inline int is_present(struct task_struct* t)
{
	return t && tsk_fsac(t)->present;
}

#endif

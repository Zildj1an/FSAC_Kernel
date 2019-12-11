/*
    This is the new FSAC scheduling class.
    It only does the most basic stuff and delegates on the active plugin.
    @author Carlos Bilbao Muñoz
    cbilbao@ucm.es
*/

#include "sched.h"

#include <fsac/fsac.h>
#include <fsac/sched_plugin.h>

static struct task_struct* fsac_schedule(struct rq *rq,
		struct task_struct *prev){
	
	struct task_struct *next;
	
	/* The FSAC plugin schedules */
	next = fsac->schedule(prev);

#ifdef CONFIG_SMP
	//TODO ? que leches es todo esto.
#endif
	/* Check if the task became invalid */
	if (next && !is_fsac(next)) {
		printk(KERNEL_NOTICE "[%llu] The task (pid %d) became invalid.\n",
				fsac_clock(),tsk->pid);
		next = NULL;
	}

 return next;
}

static void enqueue_task_fsac(struct rq *rq, struct task_struct *p,
		int flags){

	if (flags & ENQUEUE_WAKEUP){
		fsac->task_wake_up(p);
	}
	else {
	     printk(KERN_INFO "[%llu] Ignoring an enqueue (task %d), not a wake up.\n",
				fsac_clock(),tsk->pid);
	     //p->se.exec_start = rq->clock; ? TODO
	}
}

static void dequeue_task_fsac(struct rq *rq, struct task_struct *p,
                int flags){

	if (flags & DEQUEUE_SLEEP){
		fsac->task_block(p);
		//TODO ?
	} 
	else {
	   printk(KERN_INFO "[%llu] Ignoring Denqueue (task %d), didn't go to sleep.\n"
			   fsac_clock(),tsk->pid);
	}
}

static void put_prev_task_fsac(struct rq *rq, struct task_struct *p){}

/* pick_next_task_fsac() -> fsac_schedule() function
*
*  returns the next task to be scheduled
*/
static struct task_struct *pick_next_task_fsac(struct rq *rq,
	struct task_struct *prev, struct pin_cookie cookie){

	struct task_struct *next;

	//TODO ?
	next = fsac_schedule(rq,prev);

	/* The other Linux scheduling classes call this before the scheduling,
	 * when they have checked there are queued tasks. As FSAC delegates in
	 * plugins, there is no easy way to tell and hence its easier just to
	 * check if we have something next (backwards) */

	if (next)
		put_prev_task(rq,prev);

return NULL;
}

const struct sched_class fsac_sched_class = {

	.next = &stop_sched_class,
	.enqueue_task = enqueue_task_fsac,
	.dequeue_task = dequeue_task_fsac,
	.pick_next_task = pick_next_task_fsac,
	.put_prev_task = put_prev_task_fsac,
};

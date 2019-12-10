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
 //TODO
 return NULL;
}

static void enqueue_task_fsac(struct rq *rq, struct task_struct *p,
		int flags){
//TODO
}

static void dequeue_task_fsac(struct rq *rq, struct task_struct *p,
                int flags){
//TODO
}

/* pick_next_task_fsac() - fsac_schedule() function
*
*  returns the next task to be scheduled
*/
static struct task_struct *pick_next_task_fsac(struct rq *rq,
	struct task_struct *prev, struct pin_cookie cookie){
//TODO
return NULL;
}

const struct sched_class fsac_sched_class = {

	.next = &stop_sched_class,
	.enqueue_task = enqueue_task_fsac,
	.dequeue_task = dequeue_task_fsac,
	.pick_next_task = pick_next_task_fsac,
};

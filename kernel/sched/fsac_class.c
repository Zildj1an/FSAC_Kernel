/*
 *  This is the new FSAC scheduling class (/kernel/sched.fsac.c). 
 *  Only does the most basic stuff and delegates on the active plugin.
 *  @author Carlos Bilbao Muñoz 
 *  GitHub: https://github.com/Zildj1an
 */

#include "sched.h"

#include <fsac/fsac.h>
#include <fsac/fsac_preempt.h>
#include <fsac/sched_plugin.h>
#include <fsac/fsac_np.h>

static struct task_struct* fsac_schedule(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next;

#ifdef CONFIG_SMP
	struct rq* other_rq;
	int from_cpu;
	long was_running;
#endif
	/* The FSAC plugin schedules */
	next = fsac->schedule(prev);
	/* Check if the plugin has updated the preemption state machine */
	sched_state_plugin_check();

#ifdef CONFIG_SMP
	/* Check if the global plugin took the task from a different RQ 
	 * If so, I need to migrate the task. */
	if (next && task_rq(next) != rq){
		
		other_rq = task_rq(next);
		from_cpu = other_rq->cpu;
		printk(KERN_WARNING "[%llu] Migration from %d\n", fsac_clock(),from_cpu);
		
		/* The previous task could have changed its state */
		BUG_ON(prev != current);
		was_running = is_current_running();

		/* Avoid a concurrent switch race (this could deadlock in
		 * the case of cross or circular migrations).
		 */
		if (next->fsac_param.stack_in_use != NO_CPU) {
			printk(KERN_INFO "Waiting to deschedule...\n");
		}

		raw_spin_unlock(&rq->lock);

		while (next->fsac_param.stack_in_use != NO_CPU) 
		{
			cpu_relax();
			mb();

			if (next->fsac_param.stack_in_use == NO_CPU)
				printk(KERN_INFO,"Descheduled! Done.\n");

			if (!fsac->should_wait_for_stack(next)){
				/* The plugin does not want to wait for the stack !! */
				printk(KERN_INFO 
					"[%llu] The plugin gave up waiting for thread's stack.\n",
					fsac_clock());
				next = NULL;
				fsac_reschedule_local();
				raw_spin_lock(&rq->lock);
				goto end;
			}

			if (from_cpu != task_rq(next)->cpu){
				/* This should not happen, plugin is misbehaving */
				printk(KERN_WARNING 
					"[%llu]The next tsk chose by the plugin is from other RQ!\n",
					fsac_clock());
				raw_spin_lock(&rq->lock);
				fsac->next_became_invalid(next);
				fsac_reschedule_local();
				next = NULL;
				goto end;
			}

			double_rq_lock(rq, other_rq);

			if (other_rq == task_rq(next) && next->fsac_param.stack_in_use == NO_CPU){

				set_task_cpu(next,rq->cpu); /* Migrate */
				raw_spin_unlock(&other_rq->lock);
			}
			else {
				/* Two possible reasons: The tsk moved from his rq or the stack was in use.
				 * In any case, migration has to be aborted. */
				printk(KERN_INFO "[%llu] Migration for new tsk aborted.\n",fsac_clock());
				raw_spin_unlock(&other_rq->lock);
				fsac->next_became_invalid(next);
				next = NULL;
				goto end;
			}

			if (!fsac->post_migration_validate(next)){
				printk(KERN_INFO 
					"[%llu] Plugin changed his mind about the next task!\n",
					fsac_clock());
				fsac_reschedule_local();
				next = NULL
			}
	}

#endif
	
	/* Check if the task became invalid */
	if (next && !is_fsac(next)) {
		printk(KERNEL_NOTICE "[%llu] The task (pid %d) became invalid.\n",
				fsac_clock(),tsk->pid);
		fsac->next_became_invalid(next);
		fsac_reschedule_local();
		next = NULL;
	}

	if (next){

#ifdef CONFIG_SMP:
		next->fsac_param.stack_in_use == rq->cpu;
#else 
		next->fsac_param.stack_in_use == 0;
#endif
		next->se.exec_start = rq->clock;		
	}

end:
 	return next;
}

static void enqueue_task_fsac(struct rq *rq, struct task_struct *p,
		int flags){

	tsk_fsac()->present = 1;

	if (flags & ENQUEUE_WAKEUP){
		p->state = TASK_RUNNING;
		fsac->task_wake_up(p);
	}
	else {
	     printk(KERN_INFO "[%llu] Ignoring an enqueue(task %d),not a wake up.\n",
				fsac_clock(),tsk->pid);
	     p->se.exec_start = rq->clock;
	}
}

static void dequeue_task_fsac(struct rq *rq, struct task_struct *p, int flags){

	if (flags & DEQUEUE_SLEEP){
		if (fsac->is_real_time){
			tsk_fsac(p)->last_suspension = fsac_clock();
		}
		fsac->task_block(p);
		tsk_rt(p)->present = 0;
		// rq->fsac.nr_running--; Creo innecesario (?)
	} 
	else {
	   printk(KERN_INFO "[%llu] Ignoring Denqueue(task %d),didn't go to sleep.\n"
			   fsac_clock(),tsk->pid);
	}
}

/* Yield task is used for delayed preemption. */
static void yield_task_fsac(){

	// Flags (Creo innecesario)
	BUG_ON(rq->curr != current);
	//clear_exit_fsac_np(current); /* Future versions */
	fsac_reschedule_local();
}

/* These two functions are plugin-dependent */
static void check_preempt_curr_fsac(struct rq *rq, struct task_struct *p, int flags){}
static void put_prev_task_fsac(struct rq *rq, struct task_struct *p){}

/* pick_next_task_fsac() -> fsac_schedule() function
*
*  returns the next task to be scheduled
*/
static struct task_struct *pick_next_task_fsac(struct rq *rq,
	struct task_struct *prev, struct pin_cookie cookie){

	struct task_struct *next;

	lockdep_unpin_lock(&rq->lock,cookie);
	next = fsac_schedule(rq,prev); /* For real-time, this could be improved */
	lockdep_repin_lock(&rq->lock,cookie);

	/* The other Linux scheduling classes call this before the scheduling,
	 * when they have checked there are queued tasks. As FSAC delegates in
	 * plugins, there is no easy way to tell and hence its easier just to
	 * check if we have something next (backwards) */

	if (next)
		put_prev_task(rq,prev);

return next;
}

static void task_tick_fsac(struct rq *rq, struct task_struct *p, int queued){
	if (fsac->is_real_time){
			tsk_fsac(p)->last_tick = fsac_clock();
	}
}

static void switched_to_fsac(struct rq *rq, struct task_struct *p){}
static void prio_changed_fsac(struct rq *rq, struct task_struct *p,int oldprio){}
unsigned int get_rr_interval_fsac(struct rq *rq, struct task_struct *p){ return 0;}

/* This is the function called when a task becomes FSAC, this can happen because:
   A. Somehow, there was a class transition to t->policy == SCHED_FSAC
   B. The task inherited it due to a PI mutex (PRIO-INHERIT).
   Prior work such as LITMUS-RT did not bother to check scenario B and FSAC is
   way less ambitious.
*/
static void set_curr_task_fsac(struct rq *rq){
	rq->curr->se.exec_start = rq->clock;
}

#ifdef CONFIG_SMP
/* fSAC does not care about scheduling domains or rebalancing*/
static int select_task_rq_fsac(struct task_struct *p, int cpu, int sd_flag, int flags){
	return task_cpu(p);
}
#endif

static void update_curr_fsac(struct rq *rq){

	struct task_struct *p = rq->curr;

	if (!is_fsac(p)) return;
	/* In future versions, register something useful about this if plugin is RT */
}

const struct sched_class fsac_sched_class = {

	.next               = &stop_sched_class,
	.enqueue_task       = enqueue_task_fsac,
	.dequeue_task       = dequeue_task_fsac,
	.yield_task         = yield_task_fsac,
	.pick_next_task     = pick_next_task_fsac,
	.check_preempt_curr = check_preempt_curr_fsac,
	.put_prev_task      = put_prev_task_fsac,
	.task_tick          = task_tick_fsac,
	.switched_to        = switched_to_fsac,
	.prio_changed       = prio_changed_fsac,
	.get_rr_interval	= get_rr_interval_fsac,
	.set_curr_task      = set_curr_task_fsac,
#ifdef CONFIG_SMP
	.select_task_rq     = select_task_rq_fsac,
#endif
	.update_curr        = update_curr_fsac,
};

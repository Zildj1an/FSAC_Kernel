/*
	Extra scheduling class for the F.S.A.C (Framework For Scheduling
	Algorithms) Kernel.
	Will do the strictly necessary operations and then
        delegate on the active FSAC plugin.
*/

// update_time_fsac ?
// TODO other helper functions

static struct task_struct *
fsac_schedule(struct rq *rq, struct task_struct *prev)
{}


static void enqueue_task_fsac(struct rq *rq, struct task_struct *p,
				int flags)
{//TODO complete all
}

static void dequeue_task_fsac(struct rq *rq, struct task_struct *p,
				int flags)
{}

static void yield_task_litmus(struct rq *rq)
{}

/* Plugins are responsible for this. */
static void check_preempt_curr_litmus(struct rq *rq, struct task_struct *p, int flags)
{}

static void put_prev_task_fsac(struct rq *rq, struct task_struct *p)
{}

/* pick_next_task_fsac() calls fsac_schedule() function (TODO)
 *
 * return the next task to be scheduled
 */
static struct task_struct *pick_next_task_fsac(struct rq *rq,
	struct task_struct *prev, struct pin_cookie cookie)
{}

static void task_tick_fsac(struct rq *rq, struct task_struct *p, int queued)
{}

static void switched_to_fsac(struct rq *rq, struct task_struct *p)
{}

static void prio_changed_fsac(struct rq *rq, struct task_struct *p,
				int oldprio)
{}

unsigned int get_rr_interval_fsac(struct rq *rq, struct task_struct *p)
{}

// set_curr_task_fsac ?
// select_task_rq_fsac ?

static void update_curr_fsac(struct rq *rq)
{}

/* All the scheduling class methods: */
const struct sched_class fsac_sched_class = {

	.next			= &stop_sched_class, //TODO check
	.enqueue_task		= enqueue_task_fsac,
	.dequeue_task		= dequeue_task_fsac,
	.yield_task		= yield_task_fsac,
	.check_preempt_curr	= check_preempt_curr_fsac,
	.pick_next_task		= pick_next_task_fsac,
/*
#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_litmus,
#endif TODO ?
*/
	.put_prev_task		= put_prev_task_fsac, 
	.set_curr_task          = set_curr_task_fsac, //TODO read ?
	.task_tick		= task_tick_fsac,
	.get_rr_interval	= get_rr_interval_fsac,
	.prio_changed		= prio_changed_fsac,
	.switched_to		= switched_to_fsac,
	.update_curr		= update_curr_fsac,
};



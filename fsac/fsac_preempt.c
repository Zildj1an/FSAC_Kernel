/**
 * Also check /include/fsac/preempt.h
 */
#include <fsac/preempt.h>
#include <fsac/fsac.h>

/* Already defined in the header */

DEFINE_PER_CPU(bool, fsac_preemption_in_progress);
/* The rescheduling state of each processor. */
DEFINE_PER_CPU_SHARED_ALIGNED(atomic_t, resched_state);

void sched_state_will_schedule(struct task_struct* tsk){

	/* It is not safe to rely on non-local processor invocations of
	 * set_tsk_need_resched() that set the flag remotely*/
	if (likely(task_cpu(tsk) == smp_processor_id())){
		
		VERIFY_SCHED_STATE(TASK_SCHEDULED | SHOULD_SCHEDULE | 
				TASK_PICKED | WILL_SCHEDULE);
		
		if (is_in_sched_state(TASK_PICKED | PICKED_WRONG_TASK))
			set_sched_state(PICKED_WRONG_TASK);
		else {
			set_sched_state(WILL_SCHEDULE);
		}
	}
	else {
		BUG_ON(is_fsac(tsk));
	}
}

/* Called by the IPI handler after another CPU called smp_send_resched(). */
void sched_state_ipi(void) {

	/* According to the LITMUS-RT project, if the IPI was slow, we might be in 
	 * any state right now. The IPI is only meaningful if we are in 
	 * SHOULD_SCHEDULE. */
	if (is_in_sched_state(SHOULD_SCHEDULE)) {
		/* This causes the scheduler to be invoked and a transition
		 * to WILL_SCHEDULE state (view previous function).
		 */
		set_tsk_need_resched(current);
		printk(KERN_INFO "[CPU %d] IPI -> set_tsk_need_resched \n",
		      smp_processor_id());
	}
	else {
		printk(KERN_INFO "Ignoring IPI signal in state %x (%s)\n",
			get_sched_state(), sched_state_name(get_sched_state()));
	}
}

/* Called by plugins to cause a CPU to reschedule. The caller must hold the lock
 * used to serialize scheduling decisions. Also called by FSAC scheduling class */
void fsac_reschedule(int cpu) {

	int to_picked_wrong = 0;
	int to_should_schedule = 0;

	/* The (remote) CPU could be in any state.The critical states are 
	* TASK_PICKED and TASK_SCHEDULED, as the CPU
	* is not aware of the need to reschedule at this point. */

	/* Also checks if there is a context switch in progress */
	if (cpu_is_in_sched_state(cpu, TASK_PICKED)) {
		/* Atomic CAS ~ Compare and Swap */
		to_picked_wrong = sched_state_transition_on(cpu, TASK_PICKED,
			       	PICKED_WRONG_TASK);
	}

	if (!to_picked_wrong && cpu_is_in_sched_state(cpu, TASK_SCHEDULED)) {
		/* We either raced with the end of the context switch, or the
		 * CPU was in TASK_SCHEDULED anyway. */
		to_should_schedule = sched_state_transition_on(
			cpu, TASK_SCHEDULED, SHOULD_SCHEDULE);
	}

	/* If the CPU had state TASK_SCHEDULED,the scheduler should be invoked.*/
	if (to_should_schedule) {
		/* If it is in this core */
		if (smp_processor_id() == cpu) {
			set_tsk_need_resched(current);
			preempt_set_need_resched();
		} else {
			smp_send_reschedule(cpu);
		}
	}

	printk(KERN_INFO "[CPU %d] picked-ok:%d sched-ok:%d\n",cpu,
		picked_transition_ok, scheduled_transition_ok);
}

void fsac_reschedule_local(void) {

	if (is_insched_state(TASK_PICKED)) {
		set_sched_state(PICKED_WRONG_TASK);
	}
	else if (is_in_sched_state(TASK_SCHEDULED | 
				SHOULD_SCHEDULE | PICKED_WRONG_TASK)) {
		set_sched_state(WILL_SCHEDULE);
		set_tsk_need_resched(current);
		preempt_set_need_resched();
	}
}

#ifdef CONFIG_DEBUG_KERNEL

void sched_state_plugin_check(void) {

	if (!is_in_sched_state(TASK_PICKED | PICKED_WRONG_TASK)){
		printk(KERN_ALERT "The plugin didn't call sched_state_task_pìcked
		   ()! That is mandatory, fix it.\n");
		set_sched_state(TASK_PICKED);
	}
}
EXPORT_SYMBOL(sched_state_plugin_check);
#endif

/* While '##' is the token-pasting operator, as in /fsac/fsac_plugin.c,
 * '#' operator is usually referred to as the stringizing operator.
 * */
#define NAME_CHECK(x) case x: return #x
const char* sched_state_name(int s){

	switch(s) {
		NAME_CHECK(TASK_SCHEDULED);
		NAME_CHECK(SHOULD_SCHEDULE);
		NAME_CHECK(WILL_SCHEDULE);
		NAME_CHECK(TASK_PICKED);
		NAME_CHECK(PICKED_WRONG_TASK);
	default:
		return "UNKNOWN";
	};
}




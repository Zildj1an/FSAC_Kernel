/*
 * Per-CPU preemption states for the FSAC plugins.
 * Check /fsac/fsac_preempt.c for more info.
 */
#ifndef FSAC_PREEMPT_H
#define FSAC_PREEMPT_H

#include <fsac/sched.h>
#include <fsac/fsac.h>

#include <linux/types.h>
#include <linux/cache.h>
#include <linux/percpu.h>
#include <asm/atomic.h>

DECLARE_PER_CPU(bool, fsac_preemption_in_progress);

/* is_current_running is legacy macro, things are managed differently nowadays
 * for the __schedule() in the most recent Linux kernel versions */

#define is_current_running() \
	((current)->state = TASK_RUNNING || \
	 this_cpu_read(fsac_preemption_in_progress))

/* Per-CPU preemption/scheduling state */
DECLARE_PER_CPU_SHARED_ALIGNED(atomic_t, resched_state);

const char* sched_state_name(int s);

#define VERIFY_SCHED_STATE(x) 					 			\
	do { int __state = get_sched_state();			 		\
	     if ((__state & (x)) == 0)				 			\
		printk(KERN_WARNING "INVALID state 0x%x (%s)\n", 	\
		   __state,sched_state_name(__state));		 		\
	} while(0)

#define TRACE_SCHED_STATE_CHANGE(x, y, cpu)			 	          			\
    printk(KERN_INFO 														\
    	"[%llu][CPU %d] Preemption state change: [0x%x](%s) ->[0x%x](%s)\n" \
    	,fsac_clock(),cpu,x,sched_state_name(x),y,sched_state_name(y))  
		
typedef enum scheduling_state {

	/* The currently scheduled task is the one that it should, and the
	 * processor does not plan to invoke schedule(). */
	TASK_SCHEDULED = (1 << 0),
	/* A remote processor has determined that processor should reschedule,
	 * but this has not been communicated yet (IPI pending)*/
	SHOULD_SCHEDULE = (1 << 1),
	/* The processor has noticed that it has to reschedule and will do
	 * so shortly */
	WILL_SCHEDULE = (1 << 2),
	/* The processor is currently executing schedule(), has selected a new
	 * task to schedule, but the context switch is still pending.
	 */
	TASK_PICKED = (1 << 3);
	/* The processor has not yet performed the context switch, but a remote
	 * processor has already determined that a higher-priority task
	 * should be the chosen one, after the task was picked*/
	PICKED_WRONG_TASK = (1 << 4),
	
} sched_state_t;

/* Get preemption state for a given processor */
static inline sched_state_t get_sched_state_on(int cpu){
   return atomic_read(&per_cpu(resched_state,cpu));
}

/* Get preemption state for invoking processor */
static inline sched_state_t get_sched_state(void){
   return atomic_read(&this_cpu_ptr(resched_state));
}

static inline int is_in_sched_state(int possible_states){
   return get_sched_state() & possible_states;
}

static inline int cpu_is_in_sched_state(int cpu, int possible_states){
	return get_sched_state_on(cpu) & possible_states;
}

/* Change preemption state for invoking processor */
static inline void set_sched_state(sched_state_t state){
	TRACE_SCHED_STATE_CHANGE(get_sched_state(),state,smp_processor_id());
	atomic_set(this_cpu_ptr(&resched_state),state);
}

/* cmpxchng is a CAS (Compare And Swap), the state of the preemption will
 * only be updated if the previous value of it equals 'from' */
static inline int sched_state_transition(sched_state_t from, sched_state_t to){

	sched_state_t old_state;

	/* cmpxchng = Compare and exchange */
	old_state = atomic_cmpxchng(this_cpu_ptr(&resched_state), from, to);

	if (old_state == from){
		TRACE_SCHED_STATE_CHANGE(from,to,smp_processor_id());
		return 1;
	}
	return 0;
}

/* Just like sched_state_transition but for a given core */
static inline int sched_state_transition_on(int cpu,
					sched_state_t from,sched_state_t to)
{
	sched_state_t old_state;

	old_state = atomic_cmpxchg(&per_cpu(resched_state, cpu), from, to);
	
	if (old_state == from) {
		TRACE_SCHED_STATE_CHANGE(from, to, cpu);
		return 1;
	}
	return 0;
}


/*  Important function for the plugins!
 *  Plugins should call this function after a scheduling decision regarding which
 *  one is the next running task is made.
 */
static inline void sched_state_task_picked(void){

	VERIFY_SCHED_STATE(WILL_SCHEDULE);
	set_sched_state(TASK_PICKED);
}

static inline void sched_state_entered_schedule(void){

	set_sched_state(WILL_SCHEDULE);
}

/* Called by schedule() to check if the scheduling decision is still valid
*  after a context switch. Return value is 1 if the core needs to reschedule. */
static inline int sched_state_validate_switch(void) {

	int decision = 0;

	VERIFY_SCHED_STATE(PICKED_WRONG_TASK | TASK_PICKED | WILL_SCHEDULE);

	if (is_in_sched_state(TASK_PICKED)){
		decision = sched_state_transition(TASK_PICKED, TASK_SCHEDULED);	
	}

	if (!decision){
		printk(KERN_WARNING "[CPU %d] Scheduling state validation failed\n",
		    smp_processor_id());
	}

	return !decision;
}

/* State transition events. See /fsac/preempt.c for details. */
void sched_state_will_schedule(struct task_struct* tsk);
void sched_state_ipi(void);
/* Cause a CPU (remote or local) to reschedule. */
void fsac_reschedule(int cpu);
void fsac_reschedule_local(void);
 
#ifdef CONFIG_DEBUG_KERNEL
   void sched_state_plugin_check(void);
#else
   #define sched_state_plugin_check() /* no check */
#endif
 

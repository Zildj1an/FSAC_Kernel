/*
 *  Extra parameter for the struct task_struct, have fun!
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 */

#ifndef _FSAC_PARAM_H_
#define _FSAC_PARAM_H_

typedef unsigned long long fsac_time;

struct fsac_param {

	/* This is used by the FSAC class to check if the task's stack
	 * is currently in use and avoid deadlocks. It is updated by the 
	 * FSAC core.
	 */
	volatile int stack_in_use;

	/* This is true if the task can be scheduled (In the FSAC
	* class enqueue_task_fsac for example)*/
	unsigned int present:1;

	/* Just in case it is real-time */
	fsac_time last_suspension;
	fsac_time last_tick;

	unsigned int kernel_np;
};

#endif
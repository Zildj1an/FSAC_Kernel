#ifndef _FSAC_NP_H_
#define _FSAC_NP_H_

#include <fsac/fsac.h>

/* This COULD INCLUDE functions to control preemption in the non-preemptive sections,
   as for preempt_if_preemptable from fsac_plugin.c, in future versions.
 */

/* Function that could be used at yield_tasj_fsac() at FSAC class
static inline void clear_exit_fsac_np(struct task_struct *t)
{
	if (likely(tsk_fsac(t)->ctrl_page))
		tsk_fsac(t)->ctrl_page->sched.np.preempt = 0;
}

static inline int is_kernel_np(struct task_struct *t)
{
	return tsk_fsac(t)->kernel_np;
}

static inline int is_user_np(struct task_struct *t)
{
	return tsk_fsac(t)->ctrl_page ? tsk_fsac(t)->ctrl_page->sched.np.flag : 0;
}

static inline int is_np_fsac(struct task_struct *t)
{
	return unlikely(is_kernel_np(t) || is_user_np(t));
}

*/

#endif
/*
 *  Starts everything, including the proc entries for FSAC Kernel
 *  and registers /fsac/fsac_plugin.c (Dummy default plugin)
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 *  2020
 */

#include <fsac/fsac.h>
#include <fsac/fsac_plugin.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/sched/rt.h>

/* Number of uploaded tasks that exist in the system */
static atomic_t fsac_task_count = ATOMIC_INIT(0);

static DECLARE_RWSEM(plugin_switch_mutex);

void fsac_plugin_switch_disable(void){
        down_read(&plugin_switch_mutex);
}

void fsac_plugin_switch_enable(void){
        up_read(&plugin_switch_mutex);
}

/* p is a FSAC task. */
static void reinit_fsac_state(struct task_struct* p, int restore){

	/* Cleanup */
	memset(&p->fsac_param, 0, sizeof(p->fsac_param));

	if (restore){
		p->fsac_param.present = 1;
		p->fsac_param.last_suspension = -1;
		p->fsac_param.last_tick = -1;
	}
}

static long __fsac_admit_task(struct task_struct *tsk) {

        long err;

        preempt_disable();

        if (!(err = fsac->admit_task(tsk))){
                atomic_inc(&fsac_task_count);
        }
        preempt_enable();

        return err;
}

/* Called after a fork */
void fsac_fork(struct task_struct *p){

	if (is_fsac(p)){
		reinit_fsac_state(p,1);

		if (fsac->fork_task(p)){
			if (__fsac_admit_task(p)){
				p->sched_reset_on_fork = 1;
			}
		}
		else {
			reinit_fsac_state(p,0);
			printk(KERN_WARNING 
				"[%llu] FSAC fork denied for task %d.\n",
				fsac_clock(),p->pid);
		}
	}
}

/* Called upon execve() -- current is doing the exec. */
void fsac_exec(void){

	struct task_struct* p = current;

	if (is_fsac(p)) {
		/* Nothing at this point... */
	}
}

void fsac_clear_state(struct task_struct *tsk){

	atomic_dec(&fsac_task_count);
	reinit_fsac_state(tsk,1);
}

void exit_fsac(struct task_struct *dead_tsk){

	/* Free things form the fsac_param in future versions */
	BUG_ON(is_fsac(dead_tsk));
}

/* Whenever the kernel checks if the task is real-time to avoid
   delaying them, the FSAC plugin (if real-time) should also be resumed.
*/
int fsac_is_real_time(struct task_struct *tsk) {

	if(is_fsac(tsk)) {
		 BUG_ON(fsac->is_real_time != 0 &&
			fsac->is_real_time != 1);
 		 return fsac->is_real_time;
	}

	return 0;
}

/* Invoked by /sched/core.c */
long fsac_admit_task(struct task_struct *tsk) {

    long ret = 0;
    int retval;

    BUG_ON(is_fsac(tsk));

    retval = __fsac_admit_task(tsk);

    if (retval) {
 	printk(KERN_INFO "[%llu] Task with pid %d admitted to FSAC plugin %s.\n",
 		fsac_clock(),tsk->pid,fsac->plugin_name);
     } else {
         printk(KERN_INFO "[%llu] Task with pid %d NOT admitted to FSAC plugin %s(%d).\n",
 		fsac_clock(), tsk->pid,fsac->plugin_name, retval);
     }

  return ret;
}

/* called from sched_setscheduler() */
void fsac_exit_task(struct task_struct* tsk){
	if (is_fsac(tsk)) {
		fsac->task_exit(tsk);
	}
}

static atomic_t ready_to_switch;

static int __do_plugin_switch(struct sched_plugin* plugin){

	int ret;

	/* Do not switch if there are active FSAC tasks */
	if(atomic_read(&fsac_task_count) == 0) {

		printk(KERN_INFO "[%llu] Deactivating plugin %s\n",fsac_clock(),
		      fsac->plugin_name);
		if ((ret = fsac->deactivate_plugin()) != 0) goto out;

		printk(KERN_INFO "[%llu] Activating plugin %s\n",
			fsac_clock(),plugin->plugin_name);

		if ((ret = plugin->activate_plugin()) != 0) {
			printk(KERN_WARNING "Could not activate %s (return %d)\n.",
				plugin->plugin_name, ret);
			plugin = &fsac_sched_plugin;
		}

		printk(KERN_INFO "[%llu] Switching to FSAC plugin %s.\n",
			fsac_clock(), plugin->plugin_name);
		fsac = plugin;
	} else
		ret = -EBUSY;
out:
	printk(KERN_INFO "[%llu] do_plugin_switch() => %d\n.", fsac_clock(),ret);

 return ret;
}

static int do_plugin_switch(void *_plugin){

	int ret = 0;
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();
	hard_irq_disable();

	if (atomic_dec_and_test(&ready_to_switch)) {
		ret = __do_plugin_switch((struct sched_plugin*)_plugin);
		atomic_set(&ready_to_switch, INT_MAX);
	}

	do {
		/* This is __asm__ __volatile__("rep;nop": : :"memory"); */
		cpu_relax();
	} while (atomic_read(&ready_to_switch) != INT_MAX);

	local_irq_restore(flags);

 return ret;
}

/* Important and tricky function.
 * Possible deadlocks need to be avoided.
 */
int switch_sched_plugin(struct sched_plugin* plugin) {

	int err;

	BUG_ON(!plugin);

	if (atomic_read(&fsac_task_count) == 0) {
		down_write(&plugin_switch_mutex);
		/* Used to inhibit cpu hotplug operations */
		get_online_cpus();
		atomic_set(&ready_to_switch, num_online_cpus());
		err = stop_cpus(cpu_online_mask, do_plugin_switch, plugin);
		put_online_cpus();
		up_write(&plugin_switch_mutex);
		return err;
	}
	else
		return -EBUSY;
}

void fsac_do_exit(struct task_struct *tsk){

	/* tsk called do_exit(), so just make it FIFO and forget about it */

	struct sched_param params;

	printk(KERN_INFO "[%llu] Task with pid %d moved to SCHED_FIFO\n",
			fsac_clock(),tsk->pid);
	sched_setscheduler_nocheck(tsk,SCHED_FIFO,&params);
}

/* Called from core.c */
void fsac_dealloc(struct task_struct *tsk){

	printk(KERN_INFO "Deallocating task from FSAC.\n");
	fsac->task_cleanup(tsk);
	fsac_clear_state(tsk);
}

inline void fsac_task_new(struct task_struct *p,int a,int b){
        if (is_fsac(p)){
                fsac->task_new(p,a,b);
	}
}

inline void fsac_finish_switch(struct task_struct *p){
 	fsac->finish_switch(p);
}

inline struct task_struct* fsac_schedule_prev(struct task_struct *prev){
 	return fsac->schedule(prev);
}

inline int fsac_should_wait_for_stack(struct task_struct *next){
	return fsac->should_wait_for_stack(next);
}

inline void fsac_next_became_invalid(struct task_struct *next){
	fsac->next_became_invalid(next);
}

inline void fsac_task_wake_up(struct task_struct *p){
	fsac->task_wake_up(p);
}

inline void fsac_task_block(struct task_struct *p){
	fsac->task_block(p);
}

inline int fsac_post_migration_validate(struct task_struct *next){
	return fsac->post_migration_validate(next);
}

/* Wow, this function is important! */
static int __init _init_fsac(void){

	printk(KERN_WARNING "Starting FSAC kernel...\n");
	printk(KERN_WARNING "Author: Carlos Bilbao (2020)\n");
	printk(KERN_WARNING "GitHub: https://github.com/Zildj1an\n");

	register_sched_plugin(&fsac_sched_plugin);

	init_fsac_proc();

	return 0;
}

static void _exit_fsac(void){

	// TODO quitar el plugin?
	exit_fsac_proc();
}

module_init(_init_fsac);
module_exit(_exit_fsac);



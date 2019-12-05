/*
	Starts everything, including the proc entries for FSAC Kernel
	and registers /fsac/fsac_plugin.c
	@author Carlos Bilbao Muñoz
	cbilbao@ucm.es
*/
#include <fsac/fsac.h>

/* Number of uploaded tasks that exist in the system */
unsigned long fsac_task_count = ATOMIC_INIT(0);


/* Whenever the kernel checks if the task is real-time -to avoid
   delaying them- the FSAC plugin (if real-time) should also be resumed.
*/
int fsac_is_real_time(struct task_struct *tsk) {

	if(is_fsac(t)) {
 		 return fsac->is_real_time;
	}
	else return 0;
}

static long __fsac_admit_task(struct task_struct *tsk) {

	long err;

	/* TODO (?)*/
	//INIT_LIST_HEAD(&tsk_fsac(tsk)->list);

	preempt_disable();
	if (!(err = fsac->admit_task(tsk)){
		atomic_inc(&fsac_task_count);
	}
	preempt_enable();

	return err;
}

/* Invoked by /sched/core.c */
long fsac_admit_task(struct task_struct *tsk) {

    long ret = 0;

    BUG_ON(is_fsac(tsk));

    retval = __fsac_admit_task(tsk);

    if (retval) {
 	printk(KERN_INFO "[%llu] Task with pid %d accepted to FSAC plugin %s.\n",
 		fsac_clock(),tsk->pid,fsac->plugin_name);
     } else {
         printk(KERN_INFO "[%llu] Task with pid %d accepted to FSAC plugin %s.\n",
 		fsac_clock(), tsk->pid,fsac_>plugin_name);
     }

  return ret;
}

/* called from sched_setscheduler() */
void fsac_exit_task(struct task_struct* tsk){
	if (is_fsac(tsk)) {
		fsac->task_exit(tsk);
	}
}

static atomic_t ready_to_switch; //TODO

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
	unsigned long *flags;

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

static DECLARE_RWSEM(plugin_switch_mutex);

void fsac_plugin_switch_disable(void){
	down_read(&plugin_switch_mutex);
}

void fsac_plugin_switch_enable(void){
	up_read(&plugin_switch_mutex);
}

/*
 *  In this file, mainly two things are done:
 *  (1) The dummy plugin -default- is developed and registered.
 *  (2) The list of plugins is managed.
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 */

#include <fsac/fsac_plugin.h>
//#include <fsac/np.h>
//#include <linux/uacces.h>


/* Triggers preemption in local or remote CPU for scheduler plugins.
 * This function is non-preemptive section aware and does NOT invoke the scheduler
 * or send IPIs (if executed at remote core) if the task to be preempted is non-preemptive.
 */
void preempt_if_preemptable(struct task_struct* task, int cpu) 
{

	/* task is a FSAC task executing on CPU cpu. If task is NULL, then cpu
	 * is currently executing background work. */
	int reschedule = 1;
/*
	THESE CHECKS FOR NON-PREEMPTIVE SCENARIOS ARE LEFT FOR A FUTURE VERSION.

	if (!task){
		reschedule = 1;
	}

	else {
		if (smp_processor_id() == cpu){
			// Local CPU 
			// Check if we need to poke userspace TODO /fsac/np.h
			//  (np for non-preemptive)
			//
			if (is_user_np(task)){
				// Poke it. Doesn't have to be atomic since the
				// task is definitely not executing.
				//
				request_exit_np(task);
			}
			else if (!is_kernel_np(task)){
				// Only if we are allowed to preempt the currently
				// executing task 
				reschedule = 1;
			}
		}
		else {
			// Remote CPU. Only notify if it is not a kernel NP section
			// and if we did not set the user-space flag 
			reschedule = !(is_kernel_np(task) || request_exit_np_atomic(task));
		}
	}
*/
	if (likely(reschedule)) {
		fsac_reschedule(cpu);
	}
}
EXPORT_SYMBOL(preempt_if_preemptable);


/* (1) Dummy plugin functions */

static long fsac_dummy_activate_plugin(void)
{ 
	return 0;
}

static long fsac_dummy_deactivate_plugin(void)
{
 return 0;
}


static void fsac_dummy_task_new(struct task_struct *t,int on_rq,int running)
{}


static void fsac_dummy_task_wake_up(struct task_struct *task)
{}


static void fsac_dummy_task_block(struct task_struct *task)
{}


static void fsac_dummy_task_exit(struct task_struct *task)
{}


static ssize_t fsac_dummy_plugin_read(char *buf)
{ 
	return 0;
}


static struct task_struct* fsac_dummy_schedule(struct task_struct * prev)
{
	sched_state_task_picked(); 
	return NULL;
}


static long fsac_dummy_admit_task(struct task_struct* tsk)
{
	printk(KERN_INFO "Dummy FSAC plugin rejects %s/%d.\n",tsk->comm, tsk->pid);
	return -EINVAL;
}


static int fsac_dummy_fork_task(struct task_struct* tsk)
{ 
	return 0;
}


static int fsac_dummy_should_wait_for_stack(struct task_struct *next)
{
	return 1; /* Wait indefinitely */
}

static int fsac_dummy_post_migration_validate(struct task_struct *next)
{ 
	return 1; 
}

static void fsac_dummy_next_became_invalid(struct task_struct *next)
{}

static void fsac_dummy_finish_switch(struct task_struct * prev)
{}

static void fsac_dummy_task_cleanup(struct task_struct *task)
{}

struct fsac_plugin fsac_sched_plugin = {
	.plugin_name = "FSAC",
	.is_real_time = 0, /* Not really needed as this is default */
	.activate_plugin = fsac_dummy_activate_plugin,
	.deactivate_plugin = fsac_dummy_deactivate_plugin,
	.schedule = fsac_dummy_schedule,
	.admit_task = fsac_dummy_admit_task,
	.fork_task = fsac_dummy_fork_task,
	.task_new = fsac_dummy_task_new,
	.task_wake_up = fsac_dummy_task_wake_up,
	.task_block = fsac_dummy_task_block,
	.task_exit = fsac_dummy_task_exit,
	.plugin_read = fsac_dummy_plugin_read,
	/* For FSAC class:  */
	.should_wait_for_stack = fsac_dummy_should_wait_for_stack,
	.post_migration_validate = fsac_dummy_post_migration_validate,
	.next_became_invalid = fsac_dummy_next_became_invalid,
	.finish_switch = fsac_dummy_finish_switch,
	.task_cleanup = fsac_dummy_task_cleanup,
};


/* The current plugin */
struct fsac_plugin *fsac = &fsac_sched_plugin;


/* (2)  And now, manage the list of registered plugins. */


/* Avoid NULL pointers on run-time
   ## = Concatenate args in macro
*/
#define CHECK(func) { \
   if(!plugin->func) \
        plugin->func = fsac_dummy_ ## func; }


struct fsac_plugin* find_sched_plugin(const char* name) 
{

	struct fsac_plugin *plugin = NULL;

	raw_spin_lock(&proc_plugins_lock);
	plugin = proc_find_node(0,name,&proc_loaded_plugins);
	raw_spin_unlock(&proc_plugins_lock);

	return plugin;
}
EXPORT_SYMBOL(find_sched_plugin);


int register_sched_plugin(struct fsac_plugin* plugin)
{

	int err = 0;
	struct fsac_plugin *aux;

	/* Make sure a plugin module is not uploaded more than once */
	aux = find_sched_plugin(plugin->plugin_name);

	if (unlikely(aux != NULL)){

		printk(KERN_ALERT "The FSAC plugin %s is already registered.\n",
			plugin->plugin_name);
		err = -EPERM;
		goto out_reg;
    }

	if (plugin->is_real_time != 0 && plugin->is_real_time != 1) {

		printk(KERN_ALERT "1/2 FSAC plugin not real-time(1) or other(0).\n");
        	printk(KERN_ALERT "2/2 Default assumed (NOT real-time)\n");
		plugin->is_real_time = 0;
	}

	printk(KERN_INFO "[%llu] Registering FSAC plugin %s.\n",
		fsac_clock(),plugin->plugin_name);

	CHECK(activate_plugin);
	CHECK(deactivate_plugin);
	CHECK(schedule);
	CHECK(admit_task);
	CHECK(task_new);
	CHECK(task_wake_up);
	CHECK(task_exit);
	CHECK(plugin_read);
	CHECK(should_wait_for_stack);
	CHECK(post_migration_validate);
	CHECK(next_became_invalid);
	CHECK(task_cleanup);
	CHECK(fork_task);

	raw_spin_lock(&proc_plugins_lock);
	add_plugin_proc(plugin);
	raw_spin_unlock(&proc_plugins_lock);

out_reg:
    return err;
}
EXPORT_SYMBOL(register_sched_plugin);

int unregister_sched_plugin(struct fsac_plugin* plugin)
{

	int unregister = 0;

	if (strcmp(fsac->plugin_name, plugin->plugin_name) != 0) {

		unregister = 1;
		raw_spin_lock(&proc_plugins_lock);
		remove_plugin_proc(plugin->plugin_name);
		raw_spin_unlock(&proc_plugins_lock);
	}
	else
	    printk(KERN_ALERT "Currently active plugin %s can not be removed\n",
			plugin->plugin_name);
  return unregister;
}
EXPORT_SYMBOL(unregister_sched_plugin);


void print_sched_plugins(void)
{

	//TODO cambiar params?
	struct list_head *pos, *n;
	struct fsac_plugin *plugin;

	raw_spin_lock(&proc_plugins_lock);
	list_for_each_safe(pos,n,&proc_loaded_plugins) {
		
		plugin = list_entry(pos, struct fsac_plugin, list);
		printk(KERN_INFO, "%s\n", plugin->plugin_name);
	}
	raw_spin_unlock(&proc_plugins_lock);
}
EXPORT_SYMBOL(print_sched_plugins);


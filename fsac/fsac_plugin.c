/*
 *  In this file, two things are done:
 *  (1) The dummy plugin -default- is developed and registered.
 *  (2) The list of plugins is managed.
 *  @author Carlos Bilbao Muñoz
 *  cbilbao@ucm.es
 */

#include <fsac/fsac_plugin.h>

/* (1) Dummy plugin functions */

static long fsac_dummy_activate_plugin(void){ return 0;}
static long fsac_dummy_deactivate_plugin(void){ return 0;}
static void fsac_dummy_task_new(struct task_struct *t,int on_rq,int running){}
static void fsac_dummy_task_wake_up(struct task_struct *task){}
static void fsac_dummy_task_block(struct task_struct *task){}
static void fsac_dummy_task_exit(struct task_struct *task){}
static ssize_t fsac_dummy_read(char *buf){ return 0;}

static struct task_struct* fsac_dummy_schedule(struct task_struct * prev){
	sched_state_task_picked(); //TODO
	return NULL;
}

static long fsac_dummy_admit_task(struct task_struct* tsk){
	printk(KERN_INFO "Dummy FSAC plugin rejects %s/%d.\n",tsk->comm, tsk->pid);
	return -EINVAL;
}

struct fsac_plugin fsac_sched_plugin = {
	.plugin_name = "FSAC",
	.is_real_time = 0, /* Not really needed as this is default */
	.activate_plugin = fsac_dummy_activate_plugin,
	.deactivate_plugin = fsac_dummy_deactivate_plugin,
	.schedule = fsac_dummy_schedule,
	.admit_task = fsac_dummy_admit_task,
	.task_new = fsac_dummy_task_new,
	.task_wake_up = fsac_dummy_task_wake_up,
	.task_block = fsac_dummy_task_block,
	.task_exit = fsac_dummy_task_exit,
	.plugin_read = fsac_dummy_read,
};

/* The current plugin */
struct sched_plugin *fsac = &fsac_sched_plugin;

/* (2)  And now, manage the list of registered plugins. */

/* Avoid NULL pointers on run-time
   ## = Concatenate args in macro
*/
#define CHECK(func) { \
   if(!plugin->func) \
        plugin->func = fsac_dummy_ ## func; }

struct sched_plugin* find_sched_plugin(const char* name) {

	struct sched_plugin *plugin = NULL;

	raw_spin_lock(&proc_plugins_lock);
	plugin = proc_find_node(0,name,&proc_loaded_plugins);
	raw_spin_unlock(&proc_plugins_lock);

	return plugin;
}
EXPORT_SYMBOL(find_sched_plugin);

int register_sched_plugin(struct sched_plugin* plugin){

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
        	printk(KERN_ALERT "2/2 Default asummed (NOT real-time)\n");
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

	raw_spin_lock(&proc_plugins_lock);
	add_plugin_proc(&plugin->plugin_name);
	raw_spin_unlock(&proc_plugins_lock);

out_reg:
    return err;
}
EXPORT_SYMBOL(register_sched_plugin);

int unregister_sched_plugin(struct sched_plugin* plugin){

	int unregister = 0;

	if (strcmp(fsac->plugin_name, plugin->plugin_name) != 0) {
		unregister = 1;
		raw_spin_lock(&sched_plugins_lock);
		remove_plugin_proc(&plugin->plugin_name);
		raw_spin_unlock(&sched_plugins_lock);
	}
	else
	    printk(KERN_ALERT "Currently active plugin %s can not be removed\n",
			plugin->plugin_name);
  return unregister;
}
EXPORT_SYMBOL(unregister_sched_plugin);

void print_sched_plugins(struct seq_file *m){

	//TODO cambiar params?
	struct list_head *pos;
	struct sched_plugin *plugin;

	raw_spin_lock(&proc_plugins_lock);
	list_for_each(pos, &sched_plugins) {
		plugin = list_entry(pos, struct sched_plugin, links);
		seq_printf(m, "%s\n", plugin->plugin_name);
	}
	raw_spin_unlock(&proc_plugins_lock);
}
EXPORT_SYMBOL(print_sched_plugins);


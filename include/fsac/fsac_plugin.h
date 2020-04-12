/*
 *  Header of any FSAC plugin.
 *  For example of /fsac/fsac_plugin.c
 *  @author Carlos Bilbao Muñoz 
 *  GitHub: https://github.com/Zildj1an
 */

#ifndef _LINUX_FSAC_PLUGIN_H_
#define _LINUX_FSAC_PLUGIN_H_

#include <linux/printk.h>
#include <fsac/fsac.h>
#include <fsac/fsac_list.h>

/* List of registered plugins */
static LIST_HEAD(proc_loaded_plugins);
DEFINE_RAW_SPINLOCK(proc_plugins_lock);

typedef long (*activate_plugin_t) (void);

typedef long (*deactivate_plugin_t) (void);

typedef struct task_struct* (*schedule_t)(struct task_struct * prev);

typedef long (*admit_task_t)(struct task_struct* tsk);

typedef void (*task_new_t) (struct task_struct *task,
			    int on_rq,
			    int running);

/* Called to re-introduce a task after blocking */
typedef void (*task_wake_up_t) (struct task_struct *task);

/* Called to notify the plugin of a blocking task */
typedef void (*task_block_t)  (struct task_struct *task);

typedef void (*task_exit_t)    (struct task_struct *);

typedef ssize_t (*plugin_read_t) (char *buf);

/* When waiting for the stack of the selected task to be 
*  available, in fsac_schedule (with fsac_param.stack_in_use)
*  this is invoked just in case the plugin wants to cancel the
*  wait.
*  */
typedef int (*should_wait_for_stack_t)(struct task_struct *next);

/* This is used by the FSAC scheduling class too. It is just a way
*  to let the plugin validate that the task is still the one to
*  choose (Which, unless the plugin is real-time, it will most
*  likely be).
*/
typedef int (*post_migration_validate_t)(struct task_struct *next);

/* The tsk state might have changed, or perhaps the plugin chose a
 * tsk from another run-queue, which is a conceptual mistake.
*/
typedef void (*next_became_invalid_t) (struct task_struct *next);

/* Called after each task switch */
typedef void (*finish_switch_t)(struct task_struct *prev);

/* Called with no locks acquired */
typedef void (*task_cleanup_t)	(struct task_struct *);

/* return false to indicate that the plugin does not support forking */
typedef int (*fork_task_t)(struct task_struct* tsk);

struct fsac_plugin {

	struct list_head 	list;
	schedule_t          schedule;

	/* Name used within the /proc entries */
	char 			*plugin_name;
	plugin_read_t    plugin_read; 

	/* Extra time-related precautions are taken if it's R-T */
	int 			is_real_time;

	/*	Setup	   */
	activate_plugin_t	activate_plugin;
	deactivate_plugin_t	deactivate_plugin;
	
	/*	Task state changes 	*/
	admit_task_t		admit_task;
	fork_task_t         fork_task;
	task_new_t 		    task_new;
	task_wake_up_t		task_wake_up;
	task_block_t		task_block;
	task_exit_t 		task_exit;

	/* Waiting for the next task stack */
	should_wait_for_stack_t should_wait_for_stack;

	/* To make sure plugin is sure */
	post_migration_validate_t post_migration_validate;

	/* Something went wrong */
	next_became_invalid_t next_became_invalid;

	/* Called at fsac_dealloc(), invoked from core.c */
	task_cleanup_t		task_cleanup;

	finish_switch_t finish_switch;

} __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

/* Pointer to current scheduling plugin */
extern struct sched_plugin *fsac;

/* Linked list of loaded plugins */
struct list_item {
	struct sched_plugin *plugin;
    struct list_head links;
};

int register_sched_plugin(struct sched_plugin* plugin);
int unregister_sched_plugin(struct sched_plugin* plugin);
struct sched_plugin* find_sched_plugin(const char* name);
void print_sched_plugins(struct seq_file *m);

/* Plugin to the dummy default plugin */
extern struct sched_plugin fsac_sched_plugin;

#endif


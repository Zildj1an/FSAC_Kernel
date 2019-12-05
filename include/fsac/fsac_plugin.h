/*
	Header of any FSAC plugin.
	For example of /fsac/fsac_plugin.c
	@author Carlos Bilbao Muñoz
	cbilbao@ucm.es
*/

#ifndef _LINUX_FSAC_PLUGIN_H_
#define _LINUX_FSAC_PLUGIN_H_

#include <linux/printk.h>
#include <linux/uacces.h>    /* copy_from_user */
#include <fsac/fsac.h>
#include <fsac/fsac_list.h>

typedef long (*activate_plugin_t) (void);
typedef long (*deactivate_plugin_t) (void);
typedef struct task_struct* (*schedule_t)(struct task_struct * prev);
typedef long (*admit_task_t)(struct task_struct* tsk);
typedef ssize_t (*plugin_read_t) (char *buf);

struct fsac_plugin {

	struct list_head 	list;
	char 			*plugin_name;
	int 			is_real_time;

	/*	Setup	*/
	activate_plugin_t	activate_plugin;
	deactivate_plugin_t	deactivate_plugin;
	schedule_t 		schedule;
	/*	Task state changes 	*/
	admit_task_t		admit_task;
	task_new_t 		task_new;
	task_wake_up_t		task_wake_up;
	task_exit_t 		task_exit;
	plugin_read_t		plugin_read;

} __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

/* Pointer to current scheduling plugin */
extern struct sched_plugin *fsac;

int register_sched_plugin(struct sched_plugin* plugin);
int unregister_sched_plugin(struct sched_plugin* plugin);
struct sched_plugin* find_sched_plugin(const char* name);
void print_sched_plugins(struct seq_file *m);

/* Plugin to the dummy default plugin */
extern struct sched_plugin fsac_sched_plugin;

#endif


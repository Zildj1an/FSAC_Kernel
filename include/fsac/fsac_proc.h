/*
	Header of /fsac/fsac_proc.c
        More details about this file in it.
	@author Carlos Bilbao Muñoz
	cbilbao@ucm.es
*/

#ifndef _LINUX_FSAC_PROC_H_
#define _LINUX_FSAC_PROC_H_

#include <linux/proc_fs.h>
#include <linux/uacces.h>    /* copy_from_user */
#include <fsac/fsac_plugin.h>
#include <fsac/fsac_list.h>
#include <fsac/fsac.h>

/* My /proc file entry */
static struct proc_dir_entry *my_proc_entry;

int __init init_fsac_proc(void);
void exit_fsac_proc(void);
/* For the remaining function lock must be adquired prior invoked
   If you want the sched_plugin n = 0, if the list_item n != 0
*/
void* proc_find_node(int n,char *c, struct list_head* head);
void add_plugin_proc(struct sched_plugin *plugin);
void remove_plugin_proc(char *name);



#endif

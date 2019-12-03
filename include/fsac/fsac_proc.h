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

int __init start_fsac_proc(void);
void remove_fsac_proc(void);
void add_plugin_proc(char *name);
int remove_plugin_proc(char *name);



#endif

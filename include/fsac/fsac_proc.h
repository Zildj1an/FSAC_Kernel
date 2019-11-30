/*
	Header of /fsac/fsac_proc.c
        More details about this file in it.
*/

#ifndef _LINUX_FSAC_PROC_H_
#define _LINUX_FSAC_PROC_H_

#include <linux/proc_fs.h>
#include <fsac/fsac_plugin.h>

/* My /proc file entry */
static struct proc_dir_entry *my_proc_entry;

int __init start_fsac_proc(void);
void remove_fsac_proc(void);
int add_plugin_proc(char *name);
int remove_plugin_proc(char *name);

/* Safely copying contents to user array*/
int fsac_copy_safe(char *kbuf, unsigned long ksize,
		   __user const char* ubuf, unsigned long ulength);


#endif

/*
 *  Header of /fsac/fsac_proc.c
 *  More details about this file on it.
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 */

#ifndef _LINUX_FSAC_PROC_H_
#define _LINUX_FSAC_PROC_H_

#include <linux/proc_fs.h>
#include <fsac/fsac_plugin.h>
#include <fsac/fsac_list.h>
#include <fsac/fsac.h>

int __init init_fsac_proc(void);
void exit_fsac_proc(void);

/* For the remaining functions, lock must be adquired prior invokation
   If you want as return value the fsac_plugin n = 0, else if the list_item n != 0
*/
void* proc_find_node(int n,char *c, struct list_head* head);
void add_plugin_proc(struct fsac_plugin* plugin);
void remove_plugin_proc(char *name);


#endif

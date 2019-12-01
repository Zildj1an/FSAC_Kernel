/*
	Header of /fsac/fsac_list.c
        More details about this file in it.
*/

#ifndef _LINUX_FSAC_LIST_H_
#define _LINUX_FSAC_LIST_H_

#include <linux/list.h>

void fsac_remove_list(struct list_head* ghost_node);
int fsac_print_list(struct list_head* list, char members*);

#endif

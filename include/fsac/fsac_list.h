/*
	Header of /fsac/fsac_list.c
	Auxiliar functions for managing linked list
*/

#ifndef _LINUX_FSAC_LIST_H_
#define _LINUX_FSAC_LIST_H_

#include <linux/list.h>

void fsac_remove_list(struct list_head* ghost_node);
/* Return value is number of elems copied into members[] */
int fsac_print_list(struct list_head* list, char members*);
/* On success,returns pointer to list node, otherwise NULL
   If you use a spinlock/other mechanism for the list, lock it yourselve
*/
struct list_head* fsac_find_node(int n, char *c, struct list_head* head);
/* Safely copying contents to user array */
int fsac_copy_safe(char *kbuf, unsigned long ksize,__user const char* ubuf,
		    unsigned long ulength);

#endif

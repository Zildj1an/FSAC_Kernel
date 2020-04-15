/*
 *  Some aid for managing plugin special linked lists.
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 */

#include <fsac/fsac_list.h>
#include <fsac/fsac_plugin.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

void fsac_remove_list(struct list_head* ghost_node){

        struct list_head* cur_node = NULL;
        struct list_head* aux = NULL;
        struct fsac_plugin* item = NULL;

        list_for_each_safe(cur_node, aux, ghost_node) {

                item = list_entry(cur_node, struct fsac_plugin, list);
                list_del(&item->list);
				/* Free extra memory if it had dynamic char arrays */
				vfree(item);
        }
}
EXPORT_SYMBOL(fsac_remove_list);

int fsac_print_list(struct list_head* list, char* members){

	struct fsac_plugin* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	int read = 0;
	char* aux_c;

	list_for_each_safe(cur_node,aux, list) { /* while cur_node != list*/

		item = list_entry(cur_node, struct fsac_plugin, list);

		if(read + sizeof(item->plugin_name) < sizeof(members) - 1){

			aux_c = item->plugin_name;
			while((members[read++] = *aux_c) != '\n' &&
					read < sizeof(members) - 1){
			    ++aux_c;
			}
		}

	}
	return read;
}
EXPORT_SYMBOL(fsac_print_list);

struct list_head* fsac_find_node(int n, char *c, struct list_head* head){

	struct list_head* pos = NULL;
	struct list_head* aux = NULL;
	struct fsac_plugin* item = NULL;
	int find = 0;

	 list_for_each_safe(pos, aux, head) {

		item = list_entry(pos, struct fsac_plugin, list);
		if ((find = (strcmp(c,item->plugin_name) == 0))){
			return aux;
		}
	}

	return NULL;
}
EXPORT_SYMBOL(fsac_find_node);

/* Safely copying contents to user array*/
int fsac_copy_safe(char *kbuf, unsigned long ksize,
                   __user const char* ubuf, unsigned long ulength){

        BUG_ON(!ksize);

        ksize--; /* Space for null byte */

        if (ksize > ulength)
                ksize = ulength;

        if(copy_from_user(kbuf, ubuf, ksize))
                return -EFAULT;

        kbuf[ksize] = '\0';

        if (ksize > 0 && kbuf[ksize - 1] == '\n')
                kbuf[ksize - 1] = '\0';

        return ksize;
}
EXPORT_SYMBOL(fsac_copy_safe);


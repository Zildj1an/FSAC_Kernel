/*
 *  Some aid for managing special linked lists.
 *  @author Carlos Bilbao Muñoz
 *  GitHub: https://github.com/Zildj1an
 */

#include <fsac/fsac_list.h>
#include <linux/vmalloc.h>

/* Node */
struct list_item {
   void* data;
   struct list_head links;
};

void fsac_remove_list(struct list_head* ghost_node){

        struct list_head* cur_node = NULL;
        struct list_head* aux = NULL;
        struct list_item* item = NULL;

        list_for_each_safe(cur_node, aux, ghost_node) {

                item = list_entry(cur_node, struct list_item, links);
                list_del(&item->links);
				/* Free extra memory if it had dynamic char arrays */
				if (safe_char(item->data))
					vfree(item->data);
				vfree(item);
        }
}
EXPORT_SYMBOL(fsac_remove_list);

int fsac_print_list(struct list_head* list, char* members){

	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	int read = 0;
	char* aux;

	list_for_each(cur_node, list) { /* while cur_node != list*/

		item = list_entry(cur_node, struct list_item, links);

		if(read < sizeof(members) - 1){
			if (safe_char(item->data)){
				aux = item->data;
				while((members[read++] = *aux) != '\n' &&
						read < sizeof(members) - 1){
				    ++aux;
				}
			}
			else {
				read += sprintf(&members[read],"%i\n",
					item->data);
                members[read++] = '\n';
			}
	}
	return read;
}
EXPORT_SYMBOL(fsac_print_list);

struct list_head* fsac_find_node(int n, char *c, struct list_head* head){

	struct list_head* pos = NULL;
	struct list_head* aux = NULL;
	struct list_item* item = NULL;
	int find = 0;

	for (pos = (head)->next; pos != (head) && find == 0; pos = pos->next) {

		item = list_entry(pos, struct list_item, links);
		 if (safe_char(item->data)){
			find = (strcmp(c,item->data) == 0);
		} else {
			find = (item->data == n);
		}
		aux = pos;
	}

	if (find)
		return aux;
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


#include <fsac/fsac_list.h>

void fsac_remove_list(struct list_head* ghost_node){

        struct list_head* cur_node = NULL;
        struct list_head* aux = NULL;
        struct list_item* item = NULL;
	numElems = 0;

        list_for_each_safe(cur_node, aux, &ghost_node) {

                item = list_entry(cur_node, struct list_item, links);
                list_del(&item->links);
		if(__builtin_types_compatible_p(typeof(item->data), char*))
			vfree(item->data);
		vfree(item);
        }
}
EXPORT_SYMBOL(fsac_remove_list);

int fsac_print_list(struct list_head* list, char members*){

	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	int read = 0;
	char* aux;

	list_for_each(cur_node, list) { /* while cur_node != list*/

		item = list_entry(cur_node, struct list_item, links);

		if(read < sizeof(members)){
			if(__builtin_types_compatible_p(typeof(item->data), char*)){
				aux = item->data;
				while((members[read++] = *aux) != '\n' &&
						read < sizeof(members)) {++aux;}
			}
			else {
				read += sprintf(&members[read],"%i\n",item->data);
                        	members[read++] = '\n';
			}
	}
	return read;
}
EXPORT_SYMBOL(fsac_print_list);

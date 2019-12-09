/*
	Change active FSAC plugin.
	View list of registered FSAC plugins.
	View active FSAC plugins.

	+- /proc/fsac
	   |
      	   +- loaded (read operation)
	   |
	   +- active_plugin (read/write operations)
	   |
	   +- stats_active (read, from module)

	 @author Carlos Bilbao Muñoz
	 cbilbao@ucm.es
*/

#include <fsac/fsac_proc.h>

static struct proc_dir_entry *fsac_dir = NULL,
	*loaded  = NULL,
	*active_plugin = NULL,
	*stats_active = NULL;

/* PROC FUNCTIONS */
static ssize_t loaded_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {

	char kbuf[MAX_SIZE_LOADED];
	int read = 0;

	if((*off) > 0) return 0; /* Previously invoked */

	raw_spin_lock(&proc_plugins_lock);
	read = fsac_print_list(&proc_loaded_plugins,kbuf);
	raw_spin_unlock(&proc_plugins_lock);

	kbuf[read++] = '\0';

	if (copy_to_user(buf,kbuf,read) > 0) return -EFAULT;
	(*off) += read;

  return read;
}

static ssize_t active_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {

	int read = 0;

	if ((*off) > 0) return 0; /* Previously invoked */
	read = strlen(fsac->plugin_name);
	if (copy_to_user(buf,fsac->plugin_name,read) > 0) return -EFAULT;
	(*off) += read;

 return read;
}

static ssize_t active_write(struct file *filp, const char __user *buf,
		size_t len, loff_t *off) {

	char name[60];
	struct sched_plugin* found;
	ssize ret = -EINVAL;
	int err;

	if((ret = fsac_copy_safe(name,sizeof(name),buf,len) < 0)
		return ret;

	found = fsac_find_node(0,name,&proc_loaded_plugins);

	if (found){
		if(err = switch_sched_plugin(found)) {
			printk(KERN_INFO "Could not switch plugin: %d.\n",err);
			ret = err;
		}
	} else {
		printk(KERN_INFO "Plugin '%s' is unknown.\n", name);
		ret = -ESRCH;
	}

	return 0;
}

static ssize_t stats_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {

	char kbuf[MAX_PLUGIN_READ];
	int read = 0;

	if ((*off) > 0) return 0; /* Previously invoked */
	read = fsac->plugin_read(kbuf);
	if (copy_to_user(buf,kbuf,read) > 0) return -EFAULT;
	(*off) += read;

 return read;
}

/* PROC File Operations */
static const struct file_operations fsac_loaded_fops = {
	.read = loaded_read,
};

static const struct file_operations fsac_active_fops = {
        .read = active_read,
	.write = active_write,
};

static const struct file_operations fsac_stats_fops = {
        .read = stats_read,
};

int __init start_fsac_proc(void) {

	char fsac_name[] = "FSAC";

	fsac_dir = proc_mkdir("fsac", NULL);
	loaded = proc_create("loaded",0644,fsac_dir,&fsac_loaded_fops);
	active_plugin = proc_create("active_plugin",0644,
					fsac_dir,&fsac_active_fops);
        stats_active = proc_create("stats_active",0644,fsac_dir,&fsac_stats_fops);

	if (!fsac_dir || !loaded || !stats_active) goto mem_err;

//	add_plugin_proc(&fsac_name); TODO no va aqui? -> TB INICIALIZAR LA LISTA DE PLUGINS

  return 0;

mem_err:

  printk(KERN_ERR "Could not create FSAC proc entries.\n");
  return -ENOMEM;
}

void remove_fsac_proc(void) {

        if (stats_active)  remove_proc_entry("stats_active",fsac_dir);
	if (active_plugin) remove_proc_entry("active_plugin",fsac_dir);
	if (loaded)        remove_proc_entry("loaded",fsac_dir);
	if (fsac_dir)      remove_proc_entry("fsac",NULL);
}

/* You have to adquire the lock in advance */
void add_plugin_proc(char *name) {

	struct list_item *new_item = NULL;

	if (!(new_item = fsac_find_node(0,name,&proc_loaded_plugins))) {

		new_item = vmalloc(sizeof(struct list_item));
		new_item->data = vmalloc(strlen(name) + 1);
		strcpy(new_item->data,name);
        	list_add_tail(&new_item->links, &proc_loaded_plugins);
	}
}

/* Adquire the lock in advance */
int remove_plugin_proc(char *name) {

	struct list_item *new_item = NULL;

	if ((new_item = fsac_find_node(0,name,&proc_loaded_plugins))) {
                list_del(&new_item->links);
	}
}



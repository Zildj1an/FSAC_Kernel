/*
	Change active FSAC plugin.
	View list of registered FSAC plugins.
	View active FSAC plugins.

	+- /proc/fsac
	   |
      	   +- loaded
	   |
	   +- active_plugin
	   |
	   +- stats_active
*/

#include <fsac/fsac_proc.h>

static DEFINE_RAW_SPINLOCK(proc_plugins_lock);
static LIST_HEAD(proc_loaded_plugins);

static struct proc_dir_entry *fsac_dir = NULL,
	*loaded  = NULL,
	*active_plugin = NULL,
	*stats_active = NULL;

struct list_item {
	char* plugin_name;
	struct list_head links;
};

struct list_head ghost_node;

/* PROC FUNCTIONS */

static ssize_t loaded_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {
}

static ssize_t active_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {
}

static ssize_t active_write(struct file *filp,
      const char __user *buf, size_t len, loff_t *off) {
}

static ssize_t stats_read(struct file *filp,
             char __user *buf, size_t len, loff_t *off) {
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

	fsac_dir = proc_mkdir("fsac", NULL);
	loaded = proc_create("loaded",0644,fsac_dir,&fsac_loaded_fops);
	active_plugin = proc_create("active_plugin",0644,
					fsac_dir,&fsac_active_fops);
        stats_active = proc_create("stats_active",0644,fsac_dir,&fsac_stats_fops);

	if (!fsac_dir || !loaded || !stats_active) goto mem_err;

  return 0;

mem_err:

  printk(KERN_ERR "Could not create FSAC proc entries.\n");
  return -ENOMEM;
}

void remove_fsac_proc(void) {

        if(stats_active)  remove_proc_entry("stats_active",fsac_dir);
	if(active_plugin) remove_proc_entry("active_plugin",fsac_dir);
	if(loaded)        remove_proc_entry("loaded",fsac_dir);
	if(fsac_dir)      remove_proc_entry("fsac",NULL);
}

void add_plugin_proc(char *name) {

	struct list_item *new_item = NULL;

	new_item = vmalloc(sizeof(struct list_item));
	new_item->data = vmalloc(strlen(name) + 1);
	strcpy(new_item->data,name);

	raw_spin_lock(&proc_plugins_lock);
        list_add_tail(&new_item->links, &proc_loaded_plugins);
	raw_spin_unlock(&proc_plugins_lock);
}

int remove_plugin_proc(char *name) {

		//TODO
		raw_spin_lock(&proc_plugins_lock);
                list_del(&);
                raw_spin_unlock(&proc_plugins_lock);
}

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


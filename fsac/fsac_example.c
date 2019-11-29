/*
	Dummy plugin, first registered
*/

#include <fsac/fsac_plugin.h>

/* Dummy plugin functions */

static long fsac_dummy_activate_plugin(void) {
	return 0;
}

static long fsac_dummy_deactivate_plugin(void) {
	return 0;
}

struct fsac_plugin fsac_sched_plugin = {
	.plugin_name = "FSAC",
	.activate_plugin = ,
	.deactivate_plugin = ,
};

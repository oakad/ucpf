/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#include <gcc-plugin.h>
#include <diagnostic.h>
#include <tree.h>
#include <intl.h>

#include <stdio.h>
#include <unistd.h>

int plugin_is_GPL_compatible;

static plugin_info info = {
	.version = "1.0",
	.help = "aaaa\nbbbb\ncccc\n"
};

int plugin_init(
	plugin_name_args *plugin_args,
	plugin_gcc_version *version
)
{
	const char *plugin_name = plugin_args->base_name;
	printf("plugin %s loaded\n", plugin_name);
	for (int pos(0); pos < plugin_args->argc; ++pos) {
		plugin_argument *arg = plugin_args->argv + pos;
		printf("  arg %s = %s\n", arg->key, arg->value ? arg->value : "<null>");
	}

	register_callback(plugin_name, PLUGIN_INFO, NULL, &info);
	return 0;
}

extern "C" {
__attribute__ ((noreturn))
void __manual_init(void)
{
	printf("Aaaaa!\n");
	_exit(0);
}
}

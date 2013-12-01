/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <cstddef>
#include <cstdio>
#include <sqlite3.h>

namespace {

#include "ubb_base.hpp"

struct base_file {
	sqlite3_file base;
};

int base_open(
	sqlite3_vfs *vfs, char const *name, sqlite3_file *file,
	int flags, int *out_flags
)
{
	return SQLITE_CANTOPEN;
}

int base_delete(sqlite3_vfs *vfs, char const *path, int sync)
{
	return SQLITE_OK;
}

int base_access(sqlite3_vfs *vfs, char const *path, int flags, int *res_out)
{
	switch(flags) {
	case SQLITE_ACCESS_EXISTS:
	case SQLITE_ACCESS_READ:
		*res_out = 1;
		break;
	case SQLITE_ACCESS_READWRITE:
		*res_out = 0;
		break;
	default:
		*res_out = 0;
	};
	return SQLITE_OK;
}

int base_full_pathname(
	sqlite3_vfs *vfs, char const *path_in,
	int path_out_sz, char *path_out
)
{
	path_out[0] = '\0';
	return SQLITE_OK;
}

void *base_dlopen(sqlite3_vfs *vfs, char const *path){
	return nullptr;
}

void base_dlerror(sqlite3_vfs *vfs, int count, char *err_msg)
{
	err_msg[0] = '\0';
}

void (*base_dlsym(sqlite3_vfs *vfs, void *handle, const char *z))(void)
{
	return nullptr;
}

void base_dlclose(sqlite3_vfs *vfs, void *handle)
{}

int base_randomness(sqlite3_vfs *vfs, int count, char *data){
	return SQLITE_OK;
}

int base_sleep(sqlite3_vfs *vfs, int usec)
{
	return usec;
}

int base_current_time(sqlite3_vfs *vfs, double *julian_day)
{
	return SQLITE_OK;
}

struct sqlite3_vfs base_vfs = {
	.iVersion = 1,
	.szOsFile = sizeof(base_file),
	.mxPathname = 0,
	.pNext = nullptr,
	.zName = "ubb_base",
	.pAppData = nullptr,
	.xOpen = base_open,
	.xDelete = base_delete,
	.xAccess = base_access,
	.xFullPathname = base_full_pathname,
	.xDlOpen = base_dlopen,
	.xDlError = base_dlerror,
	.xDlSym = base_dlsym,
	.xDlClose = base_dlclose,
	.xRandomness = base_randomness,
	.xSleep = base_sleep,
	.xCurrentTime = base_current_time
};

}

namespace ubb {

void test()
{
	sqlite3_vfs_register(&base_vfs, 0);
	sqlite3 *db;
	sqlite3_open_v2("", &db, SQLITE_OPEN_READONLY, "ubb_base");
	sqlite3_close(db);
	sqlite3_vfs_unregister(&base_vfs);
	printf("%s\n", ubb_base_data::data);
}

}

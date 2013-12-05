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

namespace ubb {


}

namespace {



struct memfs {
	static int open(
		sqlite3_vfs *vfs, char const *name, sqlite3_file *file,
		int flags, int *out_flags
	);

} memfs_base;

struct memfs_file {
	sqlite3_file base;
};

struct sqlite3_io_methods memfs_io = {
	.iVersion = 1,
	.xClose = &memfs::close,
	.xRead = &memfs::read,
	.xWrite = &memfs::write,
	.xTruncate = &memfs::truncate,
	.xSync = &memfs::sync,
	.xFileSize = &memfs::filesize,
	.xLock = &memfs::lock,
	.xUnlock = &memfs::unlock,
	.xCheckReservedLock = &memfs::check_reserved_lock,
	.xFileControl = &memfs::fctl,
	.xSectorSize = &memfs::sector_size,
	.xDeviceCharacteristics = &memfs::device_characteristics
};

int memfs::open(
	sqlite3_vfs *vfs, char const *name, sqlite3_file *file,
	int flags, int *out_flags
)
{
	if (flags & (
		SQLITE_OPEN_DELETEONCLOSE
		| SQLITE_OPEN_CREATE
		| SQLITE_OPEN_READWRITE
	))
		return SQLITE_READONLY;

	auto *p(reinterpret_cast<memfs_file *>(vfs));
	p->base.pMethods = &memfs_io;
	return SQLITE_OK;
}

int memfs_delete(sqlite3_vfs *vfs, char const *path, int sync)
{
	return SQLITE_OK;
}

int memfs_access(sqlite3_vfs *vfs, char const *path, int flags, int *res_out)
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

int memfs_full_pathname(
	sqlite3_vfs *vfs, char const *path_in,
	int path_out_sz, char *path_out
)
{
	std::strncpy(path_out, path_in, path_out_sz);
	return SQLITE_OK;
}

void *memfs_dlopen(sqlite3_vfs *vfs, char const *path){
	return nullptr;
}

void memfs_dlerror(sqlite3_vfs *vfs, int count, char *err_msg)
{
	err_msg[0] = '\0';
}

void (*memfs_dlsym(sqlite3_vfs *vfs, void *handle, const char *z))(void)
{
	return nullptr;
}

void memfs_dlclose(sqlite3_vfs *vfs, void *handle)
{}

int memfs_randomness(sqlite3_vfs *vfs, int count, char *data)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<unsigned char> dis;
	for (int c(0); c < count; ++c)
		data[c] = dis(gen);

	return SQLITE_OK;
}

int memfs_sleep(sqlite3_vfs *vfs, int usec)
{
	std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

/* Same approach as SQLite's unixCurrentTime */
int memfs_current_time(sqlite3_vfs *vfs, double *tv)
{
	constexpr int64_t unix_epoch(24405875LL * 8640000LL);

	struct timespec tp;
	if (clock_gettime(CLOCK_REALTIME, &tp))
		return SQLITE_ERROR;

	int64_t itv(
		unix_epoch + 1000 * int64_t(tp.tv_sec) + (tp.tv_nsec / 1000000)
	);

	tv = itv;;
	tv /= 86400000.0;

	return SQLITE_OK;
}

struct sqlite3_vfs memfs_vfs = {
	.iVersion = 1,
	.szOsFile = sizeof(memfs_file),
	.mxPathname = 8,
	.pNext = nullptr,
	.zName = "ubb_base",
	.pAppData = nullptr,
	.xOpen = &memfs::open,
	.xDelete = memfs_delete,
	.xAccess = memfs_access,
	.xFullPathname = memfs_full_pathname,
	.xDlOpen = memfs_dlopen,
	.xDlError = memfs_dlerror,
	.xDlSym = memfs_dlsym,
	.xDlClose = memfs_dlclose,
	.xRandomness = memfs_randomness,
	.xSleep = memfs_sleep,
	.xCurrentTime = memfs_current_time
};

}

namespace ubb {

void test()
{
	sqlite3_vfs_register(&memfs_vfs, 0);
	sqlite3 *db;
	sqlite3_open_v2("", &db, SQLITE_OPEN_READONLY, "ubb_base");
	sqlite3_close(db);
	sqlite3_vfs_unregister(&memfs_vfs);
}

}

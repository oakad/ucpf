/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "memfd.hpp"
#include "sqlite.hpp"

#include <random>
#include <thread>

namespace ubb {


}

namespace {

struct memfs {
	sqlite3_file base;
	ubb::memfd *storage;

	static int open(
		sqlite3_vfs *vfs, char const *name, sqlite3_file *file,
		int flags, int *out_flags
	);

	static int fdelete(sqlite3_vfs *vfs, char const *path, int sync)
	{
		ubb::memfd::remove(std::string(path));
		return SQLITE_OK;
	}

	static int access(
		sqlite3_vfs *vfs, char const *path, int flags, int *res_out
	)
	{
		auto s(ubb::memfd(std::string(path)).fstat());

		switch(flags) {
		case SQLITE_ACCESS_EXISTS:
		case SQLITE_ACCESS_READ:
			*res_out = s.exists ? 1 : 0;
			break;
		case SQLITE_ACCESS_READWRITE:
			*res_out = (s.exists && !s.read_only) ? 1 : 0;
			break;
		default:
			*res_out = 0;
		};
		return SQLITE_OK;
	}

	static int full_pathname(
		sqlite3_vfs *vfs, char const *path_in,
		int path_out_sz, char *path_out
	)
	{
		std::strncpy(path_out, path_in, path_out_sz);
		return SQLITE_OK;
	}

	static void *dlopen(sqlite3_vfs *vfs, char const *path)
	{
		return nullptr;
	}

	static void dlerror(sqlite3_vfs *vfs, int count, char *err_msg)
	{
		err_msg[0] = '\0';
	}

	static void (*dlsym(sqlite3_vfs *vfs, void *handle, const char *z))()
	{
		return nullptr;
	}

	static void dlclose(sqlite3_vfs *vfs, void *handle)
	{}

	static int randomness(sqlite3_vfs *vfs, int count, char *data)
	{
		static std::random_device src;
		std::mt19937 gen(src());
		std::uniform_int_distribution<unsigned char> dis;
		for (int c(0); c < count; ++c)
			data[c] = dis(gen);

		return SQLITE_OK;
	}

	static int sleep(sqlite3_vfs *vfs, int usec)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(usec));
	}

	/* Same approach as SQLite's unixCurrentTime */
	static int current_time(sqlite3_vfs *vfs, double *tv)
	{
		constexpr int64_t unix_epoch(24405875LL * 8640000LL);

		struct timespec tp;
		if (clock_gettime(CLOCK_REALTIME, &tp))
			return SQLITE_ERROR;

		int64_t itv(
			unix_epoch + 1000 * int64_t(tp.tv_sec)
			+ (tp.tv_nsec / 1000000)
		);

		*tv = itv;;
		*tv /= 86400000.0;

		return SQLITE_OK;
	}

	static sqlite3_io_methods const io_methods;
	static sqlite3_vfs vfs;
};

sqlite3_io_methods const memfs::io_methods = {
	.iVersion = 1,
	.xClose = &close,
	.xRead = &read,
	.xWrite = &write,
	.xTruncate = &truncate,
	.xSync = &sync,
	.xFileSize = &filesize,
	.xLock = &lock,
	.xUnlock = &unlock,
	.xCheckReservedLock = &check_reserved_lock,
	.xFileControl = &fctl,
	.xSectorSize = &sector_size,
	.xDeviceCharacteristics = &device_characteristics
};

sqlite3_vfs memfs::vfs = {
	.iVersion = 1,
	.szOsFile = sizeof(memfs),
	.mxPathname = 16,
	.pNext = nullptr,
	.zName = "memfs",
	.pAppData = nullptr,
	.xOpen = &open,
	.xDelete = &fdelete,
	.xAccess = &access,
	.xFullPathname = &full_pathname,
	.xDlOpen = &dlopen,
	.xDlError = &dlerror,
	.xDlSym = &dlsym,
	.xDlClose = &dlclose,
	.xRandomness = &randomness,
	.xSleep = &sleep,
	.xCurrentTime = &current_time
};

}

namespace ubb {

void test()
{
	sqlite3_vfs_register(&memfs::vfs, 0);
	sqlite3 *db;
	sqlite3_open_v2("", &db, SQLITE_OPEN_READONLY, "ubb_base");
	sqlite3_close(db);
	sqlite3_vfs_unregister(&memfs::vfs);
}

}

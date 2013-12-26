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

namespace {

struct memfs {
	sqlite3_file base;
	char priv alignas(alignof(ubb::memfd)) [sizeof(ubb::memfd)];
	int (*busy_handler)(void *);
	void *busy_ctx;

	static int open(
		sqlite3_vfs *vfs, char const *name, sqlite3_file *file,
		int flags, int *out_flags
	)
	{
		auto self(reinterpret_cast<memfs *>(file));
		self->base.pMethods = &io_methods;
		printf("xx open %p, %s, %x\n", self, name, flags);
		new (self->priv) ubb::memfd(std::string(name));
		auto &f(*reinterpret_cast<ubb::memfd *>(self->priv));
		auto s(f.fstat());
		if (!s.exists)
			return SQLITE_CANTOPEN;

		if (!(flags & SQLITE_OPEN_READONLY) && s.read_only)
			return SQLITE_READONLY;

		*out_flags = flags & ~SQLITE_OPEN_MAIN_DB;
		return SQLITE_OK;
	}

	static int fdelete(sqlite3_vfs *vfs, char const *path, int sync)
	{
		printf("xx delete\n");
		ubb::memfd::remove(std::string(path));
		return SQLITE_OK;
	}

	static int access(
		sqlite3_vfs *vfs, char const *path, int flags, int *res_out
	)
	{
		printf("xx access %s, %x\n", path, flags);
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
		printf("xx full_pathname %s\n", path_in);
		std::strncpy(path_out, path_in, path_out_sz);
		return SQLITE_OK;
	}

	static void *dlopen(sqlite3_vfs *vfs, char const *path)
	{
		printf("xx dlopen\n");
		return nullptr;
	}

	static void dlerror(sqlite3_vfs *vfs, int count, char *err_msg)
	{
		printf("xx dlerror\n");
		err_msg[0] = '\0';
	}

	static void (*dlsym(sqlite3_vfs *vfs, void *handle, const char *z))()
	{
		printf("xx dlsym\n");
		return nullptr;
	}

	static void dlclose(sqlite3_vfs *vfs, void *handle)
	{
		printf("xx dlclose\n");
	}

	static int randomness(sqlite3_vfs *vfs, int count, char *data)
	{
		printf("xx randomness\n");
		static std::random_device src;
		std::mt19937 gen(src());
		std::uniform_int_distribution<unsigned char> dis;
		for (int c(0); c < count; ++c)
			data[c] = dis(gen);

		return SQLITE_OK;
	}

	static int sleep(sqlite3_vfs *vfs, int usec)
	{
		printf("xx sleep\n");
		std::this_thread::sleep_for(std::chrono::microseconds(usec));
	}

	/* Same approach as SQLite's unixCurrentTime */
	static int current_time(sqlite3_vfs *vfs, double *tv)
	{
		printf("xx current_time\n");
		namespace ch = std::chrono;

		constexpr int64_t unix_epoch(24405875LL * 8640000LL);
		auto t(ch::system_clock::now().time_since_epoch());
		auto s(ch::duration_cast<ch::seconds>(t));
		auto ms(ch::duration_cast<ch::milliseconds>(t - s));

		int64_t itv(
			unix_epoch + int64_t(s.count()) * 1000 + ms.count()
		);

		*tv = itv;;
		*tv /= 86400000.0;

		return SQLITE_OK;
	}

	static int close(sqlite3_file *file)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx close %p\n", self);
		reinterpret_cast<ubb::memfd *>(self->priv)->~memfd();
		return SQLITE_OK;
	}

	static int read(
		sqlite3_file *file, void *buf, int count, sqlite3_int64 offset
	)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx read %p, %d, %lld\n", self, count, offset);
		auto &f(*reinterpret_cast<ubb::memfd *>(self->priv));
		auto rc(f.read(buf, count, offset));
		if (rc < count) {
			std::memset(
				reinterpret_cast<char *>(buf) + rc,
				0, count - rc
			);
			return SQLITE_IOERR_SHORT_READ;
		} else
			return SQLITE_OK;
	}

	static int write(
		sqlite3_file *file, void const *buf, int count,
		sqlite3_int64 offset
	)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx write %p\n", self);
		return SQLITE_OK;
	}

	static int truncate(sqlite3_file *file, sqlite3_int64 size)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx truncate %p\n", self);
		return SQLITE_OK;
	}

	static int sync(sqlite3_file *file, int flags)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx sync %p\n", self);
		return SQLITE_OK;
	}

	static int filesize(sqlite3_file *file, sqlite3_int64 *size)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx filesize %p\n", self);
		auto s(reinterpret_cast<ubb::memfd *>(self->priv)->fstat());
		*size = s.size;
		return SQLITE_OK;
	}

	static int lock(sqlite3_file *file, int state)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx lock %p\n", self);
		return SQLITE_OK;
	}

	static int unlock(sqlite3_file *file, int state)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx unlock %p, %d\n", self, state);
		return SQLITE_OK;
	}

	static int check_reserved_lock(sqlite3_file *file, int *state)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx check_reserved_lock %p\n", self);
		return SQLITE_OK;
	}

	static int fcntl(sqlite3_file *file, int op, void *arg)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx fcntl %p, %d\n", self, op);
		switch (op) {
		case SQLITE_FCNTL_BUSYHANDLER: {
			auto a(reinterpret_cast<void **>(arg));
			self->busy_handler = reinterpret_cast<
				decltype(busy_handler)
			>(a[0]);
			self->busy_ctx = a[1];
			printf("xx fcntl busy %p, %p\n", self->busy_handler, self->busy_ctx);
			break;
		}
		};
		return SQLITE_OK;
	}

	static int sector_size(sqlite3_file *file)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx sector_size %p\n", self);
		auto s(reinterpret_cast<ubb::memfd *>(self->priv)->fstat());
		return s.block_size;
	}

	static int device_characteristics(sqlite3_file *file)
	{
		auto self(reinterpret_cast<memfs *>(file));
		printf("xx device_characteristics %p\n", self);
		return SQLITE_IOCAP_ATOMIC | SQLITE_IOCAP_SAFE_APPEND
		       | SQLITE_IOCAP_SEQUENTIAL
		       | SQLITE_IOCAP_POWERSAFE_OVERWRITE;
	}

	static sqlite3_io_methods const io_methods;
	static sqlite3_vfs vfs;
};

sqlite3_vfs memfs::vfs = {
	.iVersion = 1,
	.szOsFile = sizeof(memfs),
	.mxPathname = 32,
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
	.xFileControl = &fcntl,
	.xSectorSize = &sector_size,
	.xDeviceCharacteristics = &device_characteristics
};

}

struct ubb_base_data {
	static char const data alignas(16) [];
	static unsigned long const size;
};

namespace ubb {

void sqlite_error_reporter(
	void *ctx, int err, char const *message
)
{
	printf("sqlite (%p): %s (%d)\n", ctx, message, err);
}

bool sqlite_init()
{
	auto rc = sqlite3_config(SQLITE_CONFIG_MULTITHREAD);

	if (rc != SQLITE_OK) {
		fprintf(
			stderr,
			"Error %d setting SQLite multithreading mode\n", rc
		);
		return false;
	}

	sqlite3_config(
		SQLITE_CONFIG_LOG, &sqlite_error_reporter, nullptr
	);

	rc = sqlite3_initialize();
	if (rc != SQLITE_OK) {
		fprintf(
			stderr,
			"Error %d initializing sqlite\n", rc
		);
		return false;
	}
	//ubb_base-journal
	//ubb_base-wal
	auto f(memfd::create<true>(
		std::string("ubb_base"), ubb_base_data::data,
		ubb_base_data::size
	));
	rc = sqlite3_vfs_register(&memfs::vfs, 1);
	if (rc != SQLITE_OK) {
		fprintf(
			stderr,
			"Error %d registering in-memory vfs\n", rc
		);
		return false;
	}
	return true;
}

int test_cb(void *ctx, int col_cnt, char **rows, char **headers)
{
	printf("---cb %p, %d\n", ctx, col_cnt);
	for (auto c(0); c < col_cnt; ++c)
		printf("%s: %s\n", headers[c], rows[c]);

	return SQLITE_OK;
}

void test()
{
	sqlite3 *db;
	auto rc(sqlite3_open_v2(
		"ubb_base", &db, SQLITE_OPEN_READONLY, "memfs"
	));
	printf("open %d\n", rc);
	char *msg(nullptr);
	rc = sqlite3_exec(db, "select * from config;", test_cb, (void *)0x123, &msg);
	printf("exec %d, %s\n", rc, msg);
	sqlite3_free(msg);
	sqlite3_close(db);
}

}

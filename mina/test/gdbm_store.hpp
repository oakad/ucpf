/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TEST_GDBM_STORE_20141111T2100)
#define UCPF_MINA_TEST_GDBM_STORE_20141111T2100

#include <gdbm.h>
#include <fcntl.h>

#include <string>

namespace ucpf { namespace mina { namespace test {

struct gdbm_store {
	typedef std::size_t size_type;

	gdbm_store(char const *db_path_)
	: db_path(db_path_), db(nullptr)
	{}

	~gdbm_store()
	{
		if (db)
			::gdbm_close(db);
	}

	void start_restore()
	{
		db = ::gdbm_open(
			db_path.c_str(), sysconf(_SC_PAGESIZE),
			GDBM_READER, O_RDONLY, nullptr
		);
	}

	void end_restore()
	{
		::gdbm_close(db);
		db = nullptr;
	}

	void start_save()
	{
		db = ::gdbm_open(
			db_path.c_str(), sysconf(_SC_PAGESIZE),
			GDBM_WRITER, O_RDWR, nullptr
		);
	}

	void end_save()
	{
		::gdbm_close(db);
		db = nullptr;
	}

	template <typename SinkFunc>
	size_type restore_all(SinkFunc &&sink)
	{
		size_type pair_cnt(0);
		auto key(::gdbm_firstkey(db));
		while (key.dptr) {
			auto value(::gdbm_fetch(db, key));
			++pair_cnt;
			if (sink(
				key.dptr, key.dsize, value.dptr, value.dsize
			)) {
				free(key.dptr);
				free(value.dptr);
				break;
			}

			auto n_key(::gdbm_nextkey(db, key));
			free(key.dptr);
			free(value.dptr);
			key = n_key;
		}

		return pair_cnt;
	}

	void erase(char *key, int key_size)
	{
		datum x_key{
			.dptr = key,
			.dsize = key_size
		};
		::gdbm_delete(db, x_key);
	}

	void save(
		char *key, int key_size,
		char *value, int value_size
	)
	{
		datum x_key{
			.dptr = key,
			.dsize = key_size
		}, x_value{
			.dptr = value,
			.dsize = value_size
		};

		::gdbm_store(db, x_key, x_value, GDBM_REPLACE);
	}

	std::string db_path;
	GDBM_FILE db;
};

}}}
#endif


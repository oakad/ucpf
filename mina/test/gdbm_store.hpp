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
#include <string>

namespace ucpf { namespace mina { namespace test {

struct gdbm_store {
	typedef std::size_t size_type;

	gdbm_store(char const *db_path_)
	: db_path(db_path_)
	{}

	template <typename SinkFunc>
	size_type load_all(SinkFunc &&sink)
	{
		auto db(::gdbm_open(
			db_path.c_str(), sysconf(_SC_PAGESIZE),
			GDBM_READER, O_RDONLY, nullptr
		));
		if (!db)
			return 0;

		size_type pair_cnt(0);
		auto key(::gdbm_firstkey(db));
		while (key.dptr) {
			auto val(::gdbm_fetch(db, key));
			++pair_cnt;
			if (sink(
				key.dptr, key.dptr + key.dsize,
				val.dptr, val.dptr + val.dsize
			)) {
				free(key.dptr);
				free(val.dptr);
				break;
			}

			auto n_key(::gdbm_nextkey(db, key));
			free(key.dptr);
			free(val.dptr);
			key = n_key;
		}

		::gdbm_close(db);
		return pair_cnt;
	}

	std::string db_path;
};

}}}
#endif


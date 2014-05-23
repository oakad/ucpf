/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DUMPING_STORE_20140523T1800)
#define UCPF_MINA_DUMPING_STORE_20140523T1800

namespace ucpf { namespace mina {

struct dumping_store {
	bool start_save();
	void end_save();

	bool start_scan()
	{
		return false;
	}

	void merge_scan()
	{}

	bool start_restore()
	{
		return false;
	}

	void end_restore()
	{}

	void push_level(char const *name);
	void pop_level();

	template <typename T>
	void sync_value(char const *name, T &&value);
};

}}
#endif

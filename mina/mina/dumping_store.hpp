/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DUMPING_STORE_20140523T1800)
#define UCPF_MINA_DUMPING_STORE_20140523T1800

#include <list>
#include <iostream>

namespace ucpf { namespace mina {

struct dumping_store {
	bool start_save()
	{
		levels.push_back(level{std::string(), 0});
	}

	void end_save()
	{
		levels.clear();
	}

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

	void push_level(char const *name)
	{
		if (name)
			levels.push_back(level{std::string(name), 0});
		else {
			std::string s_name("value");
			s_name += std::to_string(levels.back().child_name_cnt);
			++levels.back().child_name_cnt;
			levels.push_back(level{s_name, 0});
		}
	}

	void pop_level()
	{
		levels.pop_back();
	}

	template <typename T>
	void sync_value(char const *name, T &&value)
	{
		auto iter(levels.begin());
		for (++iter; iter != levels.end(); ++iter)
			std::cout << iter->name << '.';

		std::cout << name << ": " << value << '\n';
	}

	struct level {
		std::string name;
		unsigned int child_name_cnt;
	};

	std::list<level> levels;
};

}}
#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TEXT_STORE_ADAPTOR_20140523T1800)
#define UCPF_MINA_TEXT_STORE_ADAPTOR_20140523T1800

#include <yesod/string_map.hpp>

namespace ucpf { namespace mina {

template <typename StoreType, typename Alloc>
struct text_store_adaptor {

	bool start_save()
	{
		if (sync_not_present) {
			store.load_all(
			[this](
				auto key_first, auto key_last,
				auto value_first, auto value_last
			) -> bool {
				items.emplace(
					key_first, key_last,
					items.get_allocator()
				);
				return false;
			}
		}

		levels.push_back(level{std::string(), 0});
		return true;
	}

	void end_save()
	{
		levels.clear();
		items.clear();
	}

	bool start_restore()
	{
		store.load_all(
			[this](
				auto key_first, auto key_last,
				auto value_first, auto value_last
			) -> bool {
				items.emplace(
					key_first, key_last,
					value_first, value_last,
					items.get_allocator()
				);
				return false;
			}
		);
		return false;
	}

	void end_restore()
	{
		levels.clear();
		items.clear();
	}

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

	typedef std::basic_string<
		char, std::char_traits<char>, Alloc
	> string_type;

	struct level {
		string_type name;
		unsigned int child_name_cnt;
	};

	struct string_map_policy : yesod::string_map_default_policy {
		typedef Alloc allocator_type;
	};

	yesod::string_map<
		char, std::basic_string<char, std::char_traits<char>, Alloc>,
		string_map_policy
	> items;
	std::list<level, Alloc> levels;
	StoreType &store;
	bool sync_not_present;
};

}}
#endif

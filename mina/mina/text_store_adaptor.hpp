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
#include <mina/fixed_string.hpp>
#include <yesod/iterator/seq_range.hpp>

namespace ucpf { namespace mina {

template <typename StoreType, typename Alloc>
struct text_store_adaptor {
	text_store_adaptor(StoreType &store_, Alloc const &a)
	: items(a), levels(a), store(store_), sync_not_present(false)
	{}

	~text_store_adaptor()
	{
		clear();
	}

	void clear()
	{
		items.clear(&fixed_string::destroy);
		auto a(items.get_allocator());
		for (auto &l: levels)
			fixed_string::destroy(a, l.name);

		levels.clear();
	}

	bool start_save()
	{
		if (sync_not_present) {
			auto a(items.get_allocator());
			store.load_all(
				[this, &a](
					auto key_first, auto key_last,
					auto value_first, auto value_last
				) -> bool {
					items.emplace(
						key_first, key_last,
						fixed_string::make(
							a, 1, '1'
						)
					);
					return false;
				}
			);
		}

		levels.push_back(level{fixed_string::make(), 0});
		return true;
	}

	void end_save()
	{
		clear();
	}

	bool start_restore()
	{
		auto a(items.get_allocator());
		store.load_all(
			[this, &a](
				auto key_first, auto key_last,
				auto value_first, auto value_last
			) -> bool {
				items.emplace(
					key_first, key_last,
					fixed_string::make(
						a, value_first, value_last
					)
				);
				return false;
			}
		);
		return false;
	}

	void end_restore()
	{
		clear();
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

private:
	struct string_map_policy : yesod::string_map_default_policy {
		typedef Alloc allocator_type;
	};

	yesod::string_map<char, fixed_string, string_map_policy> items;

	struct level {
		typename decltype(items)::locus loc;
		fixed_string name;
		uint32_t child_name_cnt;
	};

	std::vector<level, Alloc> levels;
	StoreType &store;
	bool sync_not_present;
};

}}
#endif

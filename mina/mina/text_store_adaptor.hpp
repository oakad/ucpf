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
#include <yesod/iterator/range.hpp>

#include <mina/fixed_string.hpp>

namespace ucpf { namespace mina {

template <typename StoreType, typename Alloc>
struct text_store_adaptor {
	constexpr static char key_separator = '.';

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
		namespace yi = ucpf::yesod::iterator;

		auto a(items.get_allocator());
		fixed_string next_name;
		bool inc_child_cnt(false);
		auto &c_level(levels.back());
		bool first_level_name(c_level.name.empty());
		auto next_loc(c_level.loc);

		auto deleter = [&a](fixed_string *s) -> void {
			fixed_string::destroy(a, *s);
		}

		std::unique_ptr<
			fixed_string, decltype(deleter)
		> s_ptr(nullptr, deleter);

		if (!first_level_name && next_loc)
			next_loc = items.locate_rel(
				next_loc, yi::make_range(&key_separator, 1)
			);

		if (name) {
			auto x_name(yi::str(name));

			if (first_level_name)
				next_name = fixed_string::make(a, x_name);
			else
				next_name = fixed_string::make(
					a, levels.back().name,
					yi::make_range(&key_separator, 1),
					x_name
				);

			if (next_loc)
				next_loc = items.locate_rel(next_loc, x_name);

			s_ptr.reset(&next_name);
		} else {
			auto x_name(to_fixed_string(
				yi::str("value_"),
				levels.back().child_name_cnt, a
			));

			s_ptr.reset(&x_name);
			if (levels.back().name.empty())
				next_name = fixed_string::make(
					a, x_name
				);
			else
				next_name = fixed_string::make(
					a, levels.back().name,
					yi::make_range(&key_separator, 1),
					x_name
				);

			if (next_loc)
				next_loc = items.locate_rel(next_loc, x_name);

			s_ptr.reset(&next_name);
			inc_child_cnt = true;
		}

		levels.push_back(level{next_loc, next_name, 0});
		if (inc_child_cnt)
			++c_level.child_name_cnt;
		s_ptr.release();
	}

	void pop_level()
	{
		auto a(items.get_allocator());
		fixed_string::destroy(a, levels.back().name);
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
	fixed_string make_next_name(char const *suffix);

	struct string_map_policy : yesod::string_map_default_policy {
		typedef Alloc allocator_type;
	};

	yesod::string_map<char, fixed_string, string_map_policy> items;

	struct level {
		typename decltype(items)::locus loc;
		fixed_string name;
		uint32_t child_name_cnt;
	};

	std::collector<level, 8, true, Alloc> levels;
	StoreType &store;
	bool sync_not_present;
};

}}
#endif

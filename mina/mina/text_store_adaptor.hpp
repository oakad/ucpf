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

#include <mina/fixed_string_utils.hpp>

namespace ucpf { namespace mina {

template <typename StoreType, typename Alloc>
struct text_store_adaptor {
	text_store_adaptor(StoreType &store_ref_, Alloc const &a)
	: items(a), levels(a), store_ref(store_ref_), key_separator("."),
	  sync_not_present(false), in_save(false)
	{}

	~text_store_adaptor()
	{
		clear();
	}

	void clear()
	{
		auto a(items.get_allocator());
		items.clear([&a](auto &val) -> void {
			fixed_string::destroy(a, val);
		});

		for (auto &l: levels)
			fixed_string::destroy(a, l.name);

		levels.clear();
	}

	bool start_save()
	{
		store_ref.writer_open();
		if (sync_not_present) {
			auto a(items.get_allocator());
			store_ref.load_all(
				[this, &a](
					auto key, auto key_size,
					auto value, auto value_size
				) -> bool {
					items.emplace(
						key, key + key_size,
						fixed_string::make(
							a, 1, '1'
						)
					);
					return false;
				}
			);
			levels.emplace_back(
				decltype(level::loc)::root(),
				fixed_string::make()
			);

		} else
			levels.emplace_back();

		in_save = true;
		return true;
	}

	void end_save()
	{
		if (sync_not_present) {
			items.for_each(
				[this](
					auto key_first, auto key_last,
					auto value
				) -> bool {
					if (value[0] == '1')
						store_ref.erase(
							key_first,
							key_last - key_first
						);

					return false;
				}
			);
		}
		store_ref.close();
		clear();
		in_save = false;
	}

	bool start_restore()
	{
		store_ref.reader_open();
		auto a(items.get_allocator());
		store_ref.load_all(
			[this, &a](
				auto key, auto key_size,
				auto value, auto value_size
			) -> bool {
				items.emplace(
					key, key + key_size,
					fixed_string::make_r(
						a, value, value + value_size
					)
				);
				return false;
			}
		);

		levels.emplace_back();

		return true;
	}

	void end_restore()
	{
		store_ref.close();
		clear();
	}

	void push_level(char const *name)
	{
		namespace yi = ucpf::yesod::iterator;

		auto a(items.get_allocator());
		auto next_name(
			name ? make_next_name(name) : make_next_auto_name()
		);

		auto deleter = [&a](fixed_string *s) -> void {
			fixed_string::destroy(a, *s);
		};

		std::unique_ptr<
			fixed_string, decltype(deleter)
		> s_ptr(&next_name.first, deleter);

		typename decltype(items)::locus next_loc;
		auto &c_level(levels.back());

		if (c_level.loc)
			next_loc = items.locate_rel(
				c_level.loc,
				next_name.second, next_name.first.end()
			);


		levels.emplace_back(next_loc, next_name.first);
		if (!name)
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
	bool sync_value(char const *name, T &&value)
	{
		auto a(items.get_allocator());
		auto next_name(
			name ? make_next_name(name) : make_next_auto_name()
		);

		auto deleter = [&a](fixed_string *s) -> void {
			fixed_string::destroy(a, *s);
		};

		std::unique_ptr<
			fixed_string, decltype(deleter)
		> s_ptr(&next_name.first, deleter);

		auto last_loc(levels.back().loc);
		auto val_ptr(last_loc ? items.find_rel(
			last_loc, next_name.second, next_name.first.end()
		) : nullptr);

		if (in_save) {
			if (sync_not_present && val_ptr)
				(*val_ptr)[0] = '0';

			fixed_string str_val;
			to_fixed_string(str_val, value, a);
			decltype(s_ptr) v_ptr(&str_val, deleter);
			store_ref.store(
				next_name.first.begin(),
				next_name.first.size(),
				str_val.begin(), str_val.size()
			);
		} else if (val_ptr)
			from_fixed_string(value, *val_ptr, a);
		else
			return false;

		return true;
	}

private:
	std::pair<
		fixed_string, fixed_string::iterator
	> make_next_name(char const *suffix)
	{
		namespace yi = ucpf::yesod::iterator;
		auto const &prefix(levels.back().name);
		fixed_string next_name;

		if (prefix.empty()) {
			next_name = fixed_string::make_s(
				items.get_allocator(), suffix
			);

			return std::make_pair(next_name, next_name.begin());
		} else {
			next_name = fixed_string::make(
				items.get_allocator(), prefix,
				yi::str(key_separator), yi::str(suffix)
			);

			return std::make_pair(
				next_name, next_name.begin() + prefix.size()
			);
		}
	}

	std::pair<
		fixed_string, fixed_string::iterator
	> make_next_auto_name()
	{
		namespace yi = ucpf::yesod::iterator;
		auto const &prefix(levels.back().name);
		fixed_string next_name;
		auto a(items.get_allocator());

		auto deleter = [&a](fixed_string *s) -> void {
			fixed_string::destroy(a, *s);
		};

		std::unique_ptr<
			fixed_string, decltype(deleter)
		> s_ptr(nullptr, deleter);

		fixed_string suffix;
		to_fixed_string(suffix, levels.back().child_name_cnt, a);

		s_ptr.reset(&suffix);

		if (prefix.empty()) {
			next_name = fixed_string::make(
				items.get_allocator(), yi::str("value_"),
				suffix
			);

			return std::make_pair(next_name, next_name.begin());
		} else {
			next_name = fixed_string::make(
				items.get_allocator(), prefix,
				yi::str(key_separator), suffix
			);

			return std::make_pair(
				next_name, next_name.begin() + prefix.size()
			);
		}
	}

	struct string_map_policy : yesod::string_map_default_policy<char> {
		typedef Alloc allocator_type;
	};

	yesod::string_map<char, fixed_string, string_map_policy> items;

	struct level {
		typename decltype(items)::locus loc;
		fixed_string name;
		uint32_t child_name_cnt;

		level()
		: name(fixed_string::make()), child_name_cnt(0)
		{}

		level(decltype(loc) loc_, fixed_string name_)
		: loc(loc_), name(name_), child_name_cnt(0)
		{}
	};

	yesod::collector<level, 8, true, Alloc> levels;
	StoreType &store_ref;
	char const *key_separator;
	bool sync_not_present;
	bool in_save;
};

}}
#endif

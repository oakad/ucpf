/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman
    Copyright (c) 2013 Carl Barron

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

/*
 * Adaptations as part of present software bundle:
 * 
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */


#if !defined(UBB_EXT_STRING_MAP_DEC_05_2013_1820)
#define UBB_EXT_STRING_MAP_DEC_05_2013_1820

#include <yesod/is_sequence.hpp>
#include <ext/tst.hpp>

#include <initializer_list>
#include <memory>

namespace ubb { namespace ext {
namespace traits {

template <typename T>
inline T const *get_string_begin(T const *str)
{
	return str;
}

template <typename T>
inline T *get_string_begin(T *str)
{
	return str;
}

template <typename T>
inline T const *get_string_end(T const *str)
{
	T const *last = str;
	while (*last)
		last++;
	return last;
}

template <typename T>
inline T *get_string_end(T *str)
{
	T *last = str;
	while (*last)
		last++;
	return last;
}

template <typename T, typename Str>
inline typename Str::const_iterator get_string_begin(Str const &str)
{
	return str.cbegin();
}

template <typename T, typename Str>
inline typename Str::iterator get_string_begin(Str &str)
{
	return str.begin();
}

template <typename T, typename Str>
inline typename Str::const_iterator get_string_end(Str const &str)
{
	return str.cend();
}

template <typename T, typename Str>
inline typename Str::iterator get_string_end(Str &str)
{
	return str.end();
}

template <typename T, typename Str>
inline typename std::enable_if<
	!ucpf::yesod::is_sequence<Str>::value, T const *
>::type get_string_begin(Str const &str)
{
	return str;
}

template <typename T, typename Str>
inline typename std::enable_if<
	!ucpf::yesod::is_sequence<Str>::value, T const *
>::type get_string_end(Str const &str)
{
	return get_string_end(get_string_begin<T>(str));
}

}

struct unused_type {
};

template <
	typename Char = char, typename T = unused_type,
	typename Lookup = tst<Char, T>, typename Filter = tst_pass_through
> struct string_map {
	typedef Char char_type; // the character type
	typedef T value_type; // the value associated with each entry
	typedef string_map<Char, T, Lookup, Filter> this_type;
	typedef value_type attribute_type;

	string_map()
	: lookup(new Lookup())
	{}

	string_map(string_map const &other)
	: lookup(other.lookup)
	{}

	template <typename Filter_>
	string_map(string_map<Char, T, Lookup, Filter_> const &other)
	: lookup(other.lookup)
	{}

	template <typename StringMap>
	string_map(StringMap const &other)
	: lookup(new Lookup())
	{
		for (auto const &k: other)
			add(k);
	}

	template <typename StringMap, typename Data>
	string_map(StringMap const &other, Data const &data)
	: lookup(new Lookup())
	{
		auto d(std::begin(data));
		for (auto const &k: other)
			add(k, *d++);
	}

	string_map(std::initializer_list<std::pair<Char const *, T>> other)
	: lookup(new Lookup())
	{
		for (auto const &v: other)
			add(v->first, v->second);
	}

	string_map(std::initializer_list<Char const*> other)
	: lookup(new Lookup())
	{
		for (auto v: other)
			add(v);
	}

	string_map &operator=(string_map const &other)
	{
		lookup = other.lookup;
		return *this;
	}

	template <typename Filter_>
	string_map &operator=(string_map<Char, T, Lookup, Filter_> const &other)
	{
		lookup = other.lookup;
		return *this;
	}

	void clear()
	{
		lookup->clear();
	}

	template <typename F>
	void for_each(F f) const
	{
		lookup->for_each(f);
	}

	template <typename Str>
	value_type &at(Str const &str)
	{
		return *lookup->add(
			traits::get_string_begin<Char>(str),
			traits::get_string_end<Char>(str), T()
		);
	}

	template <typename Str>
	string_map &remove(Str const &str)
	{
		lookup->remove(
			traits::get_string_begin<Char>(str),
			traits::get_string_end<Char>(str)
		);
		return *this;
	}

	template <typename Iterator>
	value_type *prefix_find(Iterator &first, Iterator const &last)
	{
		return lookup->find(first, last, Filter());
	}

	template <typename Iterator>
	value_type const *prefix_find(
		Iterator &first, Iterator const &last
	) const
	{
		return lookup->find(first, last, Filter());
	}

	template <typename Str>
	value_type *find(Str const &str)
	{
		return find_impl(
			traits::get_string_begin<Char>(str),
			traits::get_string_end<Char>(str)
		);
	}

	template <typename Str>
	value_type const *find(Str const& str) const
	{
		return find_impl(
			traits::get_string_begin<Char>(str),
			traits::get_string_end<Char>(str)
		);
	}

private:
	template <typename Iterator>
	value_type *find_impl(Iterator begin, Iterator end)
	{
		auto r(lookup->find(begin, end, Filter()));
		return begin == end ? r : nullptr;
	}

	template <typename Iterator>
	value_type const *find_impl(Iterator begin, Iterator end) const
	{
		auto r(lookup->find(begin, end, Filter()));
		return begin == end ? r : nullptr;
	}
        
	std::shared_ptr<Lookup> lookup;
};

}}

#endif

/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_B039B2F220446C5D73F214936B21EDB2)
#define HPP_B039B2F220446C5D73F214936B21EDB2

#include <mina/detail/from_ascii_numeric_i.hpp>
#include <mina/detail/from_ascii_numeric_f.hpp>

namespace ucpf { namespace mina {
namespace detail {

template <typename T, bool IsFloat = false, bool IsSigned = false>
struct from_ascii_numeric_dispatch {
	template <
		typename FirstIterator, typename LastIterator, typename Alloc
	> static bool apply(
		T &v, FirstIterator &&first, LastIterator const &last,
		Alloc const &a
	)
	{
		return from_ascii_numeric_u<T>(
			v, std::forward<FirstIterator>(first), last
		);
	}
};

template <typename T>
struct from_ascii_numeric_dispatch<T, false, true> {
	template <
		typename FirstIterator, typename LastIterator, typename Alloc
	> static bool apply(
		T &v, FirstIterator &&first, LastIterator const &last,
		Alloc const &a
	)
	{
		return from_ascii_numeric_s<T>(
			v, std::forward<FirstIterator>(first), last
		);
	}
};

template <typename T>
struct from_ascii_numeric_dispatch<T, true, true> {
	template <
		typename FirstIterator, typename LastIterator, typename Alloc
	> static bool apply(
		T &v, FirstIterator &&first, LastIterator const &last,
		Alloc const &a
	)
	{
		return from_ascii_numeric_f<T>(
			v, std::forward<FirstIterator>(first), last, a
		);
	}
};

}

template <
	typename T, typename FirstIterator, typename LastIterator,
	typename Alloc = std::allocator<void>
>  bool from_ascii_numeric(
	T &v, FirstIterator &&first, LastIterator const &last, 
	Alloc const &a = Alloc()
)
{
	return detail::from_ascii_numeric_dispatch<
		T, std::is_floating_point<T>::value,
		std::is_signed<T>::value
	>::apply(v, std::forward<FirstIterator>(first), last, a);
}

}}

#endif


/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_FIXED_STRING_UTILS_20141111T2300)
#define UCPF_MINA_FIXED_STRING_UTILS_20141111T2300

#include <yesod/collector.hpp>
#include <mina/fixed_string.hpp>
#include <mina/to_ascii_decimal.hpp>

namespace ucpf { namespace mina {

template <typename T, typename Alloc>
auto to_fixed_string(
	fixed_string &s, T const &v, Alloc const &a
) -> typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
{
	yesod::collector<
		typename fixed_string::value_type, 32, true, Alloc
	> sink(a);

	to_ascii_decimal(std::back_inserter(sink), v, a);
	if (sink.empty()) {
		s = fixed_string::make();
		return false;
	} else {
		s = fixed_string::make(a, sink);
		return true;
	}
}

template <typename T, typename Alloc>
auto from_fixed_string(
	T &v, fixed_string const &s, Alloc const &a
) -> typename std::enable_if<std::is_arithmetic<T>::value, bool>::type;

}}

#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TO_FIXED_STRING_20141111T2300)
#define UCPF_MINA_TO_FIXED_STRING_20141111T2300

#include <yesod/collector.hpp>
#include <mina/fixed_string.hpp>
#include <mina/to_ascii_decimal.hpp>

namespace ucpf { namespace mina {

template <typename T, typename Alloc>
auto to_fixed_string(
	T const &v, Alloc const &a
) -> typename std::enable_if<std::is_arithmetic<T>::value, fixed_string>::type
{
	yesod::collector<
		typename fixed_string::value_type, 32, true, Alloc
	> sink(a);

	to_ascii_decimal(std::back_inserter(sink), v, a);
	if (sink.empty())
		return fixed_string::make();
	else
		return fixed_string::make(a, sink);
}

template <typename PrefixSeq, typename T, typename Alloc>
auto to_fixed_string(
	PrefixSeq const &prefix, T const &v, Alloc const &a
) -> typename std::enable_if<std::is_arithmetic<T>::value, fixed_string>::type
{
	yesod::collector<
		typename fixed_string::value_type, 32, true, Alloc
	> sink(a);

	to_ascii_decimal(std::back_inserter(sink), v, a);
	if (sink.empty())
		return fixed_string::make();
	else
		return fixed_string::make(a, prefix, sink);
}

}}

#endif

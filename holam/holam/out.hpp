/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_8AB2B6049EB6ABD37D6443C0A7527014)
#define HPP_8AB2B6049EB6ABD37D6443C0A7527014

#include <holam/detail/output_formatter.hpp>
#include <holam/output/stdio.hpp>

namespace ucpf { namespace holam {
namespace detail {


}

template <typename OutputIterator, typename... Args>
std::size_t out(OutputIterator &iter, char const *format, Args &&...args)
{
	detail::output_formatter<OutputIterator> formatter(iter, format);

	while (formatter.advance()) {

	}
	return formatter.count;
}

template <typename... Args>
std::size_t out(char const *format, Args &&...args)
{
	output::stdio dest(stdout);
	return out(dest, std::forward<Args>(args)...);
}

}}
#endif

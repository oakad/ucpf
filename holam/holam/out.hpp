/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_8AB2B6049EB6ABD37D6443C0A7527014)
#define HPP_8AB2B6049EB6ABD37D6443C0A7527014

#include <holam/output/stdio.hpp>

namespace ucpf { namespace holam {
namespace detail {

template <typename OutputIterator>
std::size_t out_fixed(OutputIterator &iter, char const *&format)
{
	std::size_t count(0);

	while (*format) {
		if (*format == '{')
			break;

		*iter++ = *format;
		++format;
		++count;
	}

	return count;
}

template <typename OutputIterator, typename... Args>
std::size_t out_arg(OutputIterator &iter, char const *format, Args &&...args)
{
	return 0;
}

}

template <typename OutputIterator, typename... Args>
std::size_t out(OutputIterator &iter, char const *format, Args &&...args)
{
	std::size_t count(0);
	while (true) {
		count += detail::out_fixed(iter, format);
		if (!*format)
			return count;
		count += detail::out_arg(
			iter, format, std::forward<Args>(args)...
		);
	}
}

template <typename... Args>
std::size_t out(char const *format, Args &&...args)
{
	output::stdio dest(stdout);
	return out(dest, std::forward<Args>(args)...);
}

}}
#endif

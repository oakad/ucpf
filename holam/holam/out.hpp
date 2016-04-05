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

template <typename ArgDef, typename Arg>
bool emit_arg_value(ArgDef &arg_def, Arg &&arg)
{
}

template <typename ArgDef, typename Arg0>
bool select_arg_value(ArgDef &arg_def, Arg0 &&arg0)
{
	if (arg_def.arg_pos)
		return false;
	else
		return emit_arg_value(arg_def, std::forward<Arg0>(arg0));
}

template <typename ArgDef, typename Arg0, typename... Args>
bool select_arg_value(ArgDef &arg_def, Arg0 &&arg0, Args &&...args)
{

	if (arg_def.arg_pos != (sizeof...(args)))
		return select_arg_value(
			arg_def, std::forward<Args>(args)...
		);
	else
		return emit_arg_value(arg_def, std::forward<Arg0>(arg0));
}

}

template <typename OutputIterator, typename... Args>
std::size_t out(OutputIterator &iter, char const *format, Args &&...args)
{
	detail::output_formatter<OutputIterator> formatter(iter, format);

	while (formatter.advance(true)) {
		auto arg(formatter.get_arg(sizeof...(args));

		if (!arg.valid()) {
			formatter.advance(false);
			break;
		}

		if (!detail::select_arg_value(
			arg, std::forward<Args>(args)...
		)) {
			formatter.advance(false);
			break;
		}
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

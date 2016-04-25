/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4E17D50D00941F44FCFD5E01561235B7)
#define HPP_4E17D50D00941F44FCFD5E01561235B7

namespace ucpf { namespace holam { namespace traits {

template <typename T>
struct string_value {
	typedef std::true_type disabled;

	template <typename OutputIterator>
	static void apply(OutputIterator &iter, T &&val)
	{
	}
};


}}}
#endif

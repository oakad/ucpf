/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_EA28EE737250A60C55F0217FF0BBE469)
#define HPP_EA28EE737250A60C55F0217FF0BBE469

#include <string>
#include <vector>

#include <mina/np_packager.hpp>

namespace ucpf { namespace zivug { namespace config {

template <typename Alloc>
using string = std::basic_string<char, std::char_traits<char>, Alloc>;

template <typename Alloc>
using string_vec = std::vector<string<Alloc>, Alloc>;

}}}
#endif

/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_7F4FC9FA604CA8AF40D2A5F0CDCA860A)
#define HPP_7F4FC9FA604CA8AF40D2A5F0CDCA860A

#include <holam/support/int128.hpp>

namespace ucpf { namespace holam { namespace support {

constexpr int clz(uint32_t v)
{
	return __builtin_clz(v);
}

constexpr int clz(uint64_t v)
{
	return __builtin_clzll(v);
}

constexpr int clz(uint128_t v)
{
	return (
		v >> 64u
	) ? __builtin_clzll(
		static_cast<uint64_t>(v >> 64u)
	) : (
		64 + __builtin_clzll(static_cast<uint64_t>(v))
	);
}

}}}
#endif

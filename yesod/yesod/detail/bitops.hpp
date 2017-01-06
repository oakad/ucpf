/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_323DD09E91DC10AE51FDBC6FFBE08110)
#define HPP_323DD09E91DC10AE51FDBC6FFBE08110

namespace ucpf::yesod::detail {

constexpr static inline int clz(uint32_t v)
{
	return __builtin_clz(v);
}

constexpr static inline int clz(uint64_t v)
{
	return __builtin_clzll(v);
}

constexpr static inline int ctz(uint32_t v)
{
	return __builtin_ctz(v);
}

constexpr static inline int ctz(uint64_t v)
{
	return __builtin_ctzll(v);
}

constexpr static inline int popcount(uint32_t v)
{
	return __builtin_popcount(v);
}

constexpr static inline int popcount(uint64_t v)
{
	return __builtin_popcountll(v);
}

}
#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_BITOPS_20131117T1945)
#define UCPF_YESOD_BITOPS_20131117T1945

#include <cstdint>
#include <type_traits>
#include <yesod/detail/int128.hpp>

namespace ucpf { namespace yesod {

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
	return (v >> 64u)
	       ? __builtin_clzll(static_cast<uint64_t>(v >> 64u))
	       : (64 + __builtin_clzll(static_cast<uint64_t>(v)));
}

constexpr int ctz(uint32_t v)
{
	return __builtin_ctz(v);
}

constexpr int ctz(uint64_t v)
{
	return __builtin_ctzll(v);
}

constexpr int ctz(uint128_t v)
{
	return static_cast<uint64_t>(v)
	       ? __builtin_ctzll(static_cast<uint64_t>(v))
	       : (64 + __builtin_ctzll(static_cast<uint64_t>(v >> 64)));
}

constexpr int fls(uint32_t v)
{
	return 31 - __builtin_clz(v);
}

constexpr int fls(uint64_t v)
{
	return 63 - __builtin_clzll(v);
}

constexpr int fls(uint128_t v)
{
	return (v >> 64) ? fls(uint64_t(v >> 64u)) + 64
			 : fls(uint64_t(v));
}

template <typename T>
constexpr int order_base_2(T v)
{
	return v > (T(1) << fls(v)) ? fls(v) + 1 : fls(v);
}

constexpr int ffs(uint32_t v)
{
	return __builtin_ffs(v);
}

constexpr int ffs(uint64_t v)
{
	return __builtin_ffsll(v);
}

constexpr int ffs(uint128_t v)
{
	return static_cast<uint64_t>(v)
	       ? ffs(static_cast<uint64_t>(v))
	       : (ffs(static_cast<uint64_t>(v >> 64u)) + 64);
}

constexpr int popcount(uint32_t v)
{
	return __builtin_popcount(v);
}

constexpr int popcount(uint64_t v)
{
	return __builtin_popcountll(v);
}

constexpr int popcount(uint128_t v)
{
	return __builtin_popcountll(static_cast<uint64_t>(v >> 64u))
	       + __builtin_popcountll(static_cast<uint64_t>(v));
}

constexpr uint32_t shiftr(uint32_t v, unsigned int c)
{
	return c < 32 ? v >> c : uint32_t(0);
}

constexpr uint64_t shiftr(uint64_t v, unsigned int c)
{
	return c < 64 ? v >> c : uint64_t(0);
}

constexpr uint128_t shiftr(uint128_t v, unsigned int c)
{
	return c < 128 ? v >> c : uint128_t(0);
}

constexpr uint32_t rotl(uint32_t v, unsigned int c)
{
	return (v << c) | (v >> (32 - c));
}

constexpr uint32_t bswap(uint32_t v)
{
	return __builtin_bswap32(v);
}

constexpr uint64_t bswap(uint64_t v)
{
	return __builtin_bswap64(v);
}

}}
#endif

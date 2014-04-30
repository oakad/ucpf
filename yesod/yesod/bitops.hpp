/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_BITOPS_NOV_17_2013_1945)
#define UCPF_YESOD_BITOPS_NOV_17_2013_1945

#include <type_traits>

namespace ucpf { namespace yesod {

constexpr int fls(uint32_t v)
{
	return 31 - __builtin_clz(v);
}

constexpr int fls(uint64_t v)
{
	return 63 - __builtin_clzll(v);
}

template <typename T>
constexpr int order_base_2(T v)
{
	return v > (T(1) << fls(v)) ? fls(v) + 1 : fls(v);
}

constexpr int ffs(uint32_t v)
{
	return __builtin_ffs(v) - 1;
}

constexpr int ffs(uint64_t v)
{
	return __builtin_ffsll(v) - 1;
}

constexpr int popcount(uint32_t v)
{
	return __builtin_popcount(v);
}

constexpr int popcount(uint64_t v)
{
	return __builtin_popcountll(v);
}

}}
#endif

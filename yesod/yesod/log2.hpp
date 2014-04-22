/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_LOG2_NOV_17_2013_1945)
#define UCPF_YESOD_LOG2_NOV_17_2013_1945

#include <type_traits>

namespace ucpf { namespace yesod {

constexpr int ilog2(uint32_t v)
{
	return 31 - __builtin_clz(v);
}

constexpr int ilog2(uint64_t v)
{
	return 63 - __builtin_clzll(v);
}

template <typename T>
constexpr int order_base_2(T v)
{
	return v > (T(1) << ilog2(v)) ? ilog2(v) + 1 : ilog2(v);
}

}}
#endif

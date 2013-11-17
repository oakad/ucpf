/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STATIC_LOG2_NOV_17_2013_1945)
#define UCPF_YESOD_STATIC_LOG2_NOV_17_2013_1945

#include <type_traits>

namespace ucpf { namespace yesod {

template <typename T, T v>
struct static_log2;

template <uint32_t v>
struct static_log2<uint32_t, v> {
	constexpr static int msb = 31 - __builtin_clz(v);
	constexpr static int value = v > (1U << msb) ? msb + 1 : msb;
	typedef std::integral_constant<int, value> type;
};

template <uint32_t v>
constexpr int static_log2<uint32_t, v>::msb;
template <uint32_t v>
constexpr int static_log2<uint32_t, v>::value;

template <uint64_t v>
struct static_log2<uint64_t, v> {
	constexpr static int msb = 63 - __builtin_clzl(v);
	constexpr static int value = v > (1ULL << msb) ? msb + 1 : msb;
	typedef std::integral_constant<int, value> type;
};

template <uint64_t v>
constexpr int static_log2<uint64_t, v>::msb;
template <uint64_t v>
constexpr int static_log2<uint64_t, v>::value;

}}
#endif

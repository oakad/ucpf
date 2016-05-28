/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4E1685A34B80BC46A578A11AE4DA6B80)
#define HPP_4E1685A34B80BC46A578A11AE4DA6B80

#include <cstdint>
#include <utility>

namespace ucpf {

#if defined(_GLIBCXX_USE_INT128)

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

struct [[gnu::packed]] soft_int128_t {
	uint64_t low;
	uint64_t high;
};

struct [[gnu::packed]] soft_uint128_t {
	soft_uint128_t() = default;

	soft_uint128_t(uint64_t v)
	: low(v), high(0)
	{}

#if defined(_GLIBCXX_USE_INT128)
	soft_uint128_t(uint128_t v)
	: low(v), high(v >> 64)
	{}
#endif

	uint64_t low;
	uint64_t high;
};

struct [[gnu::packed]] soft_uint256_t {
	soft_uint128_t low;
	soft_uint128_t high;
};

#else

struct [[gnu::packed]] soft_int128_t {
	uint64_t high;
	uint64_t low;
};

struct [[gnu::packed]] soft_uint128_t {
	soft_uint128_t() = default;

	soft_uint128_t(uint64_t v)
	: high(0), low(v)
	{}

#if defined(_GLIBCXX_USE_INT128)
	soft_uint128_t(uint128_t v)
	: high(v >> 64), low(v)
	{}
#endif

	uint64_t high;
	uint64_t low;
};

struct [[gnu::packed]] soft_uint256_t {
	soft_uint128_t high;
	soft_uint128_t low;
};

#endif

#if defined(_GLIBCXX_USE_INT128)

namespace holam { namespace support {

static inline soft_uint128_t multiply(uint64_t l, uint64_t r)
{
	uint128_t acc(l);
	acc *= r;
	soft_uint128_t rv;
	rv.low = uint64_t(acc);
	rv.high = uint64_t(acc >> 64);
	return rv;
}

static inline soft_uint256_t multiply(uint128_t l, uint128_t r)
{
	soft_uint256_t rv;
	uint128_t acc((uint64_t(r)));
	acc *= uint64_t(l);
	rv.low.low = uint64_t(acc);
	rv.low.high = uint64_t(acc >> 64);

	acc = r >> 64;
	acc *= uint64_t(l);
	rv.high.low = __builtin_uaddll_overflow(
		rv.low.high, uint64_t(acc),
		reinterpret_cast<unsigned long long *>(&rv.low.high)
	) ? uint64_t(acc >> 64) + 1 : uint64_t(acc >> 64);

	acc = uint64_t(r);
	acc *= l >> 64;
	if (__builtin_uaddll_overflow(
		rv.low.high, uint64_t(acc),
		reinterpret_cast<unsigned long long *>(&rv.low.high)
	)) {
		rv.high.high = __builtin_uaddll_overflow(
			rv.high.low, uint64_t(acc >> 64) + 1,
			reinterpret_cast<unsigned long long *>(&rv.high.low)
		) ? 1 : 0;
	} else {
		rv.high.high = __builtin_uaddll_overflow(
			rv.high.low, uint64_t(acc >> 64),
			reinterpret_cast<unsigned long long *>(&rv.high.low)
		) ? 1 : 0;
	}

	acc = r >> 64;
	acc *= l >> 64;
	rv.high.high += __builtin_uaddll_overflow(
		rv.high.low, uint64_t(acc),
		reinterpret_cast<unsigned long long *>(&rv.high.low)
	) ? uint64_t(acc >> 64) + 1 : uint64_t(acc >> 64);
	return rv;
}

}}
#else

typedef soft_int128_t int128_t;
typedef soft_uint128_t uint128_t;

static inline uint128_t &operator-=(uint128_t &l, uint64_t r)
{
	if (__builtin_usubll_overflow(l.low, r, &l.low))
		--l.high;

	return l;
}

static inline uint128_t &operator|=(uint128_t &l, uint64_t r)
{
	l.low |= r;
	return l;
}

static inline uint128_t &operator<<=(uint128_t &l, int s)
{
	if (s < 64) {
		l.high <<= s;
		l.high |= l.low >>= 64 - s;
		l.low <<= s;
	} else if (s < 128) {
		l.high = l.low << (s - 64);
		l.low = 0;
	} else {
		l.low = 0;
		l.high = 0;
	}
	return l;
}

namespace holam { namespace support {

static inline soft_uint128_t multiply(uint64_t l, uint64_t r)
{
	soft_uint128_t rv;
	rv.low = uint32_t(r);
	rv.low *= uint32_t(l);
	uint64_t acc(r >> 32);
	acc *= uint32_t(l);
	rv.high = (acc >> 32) + (__builtin_uaddll_overflow(
		rv.low, acc << 32, &rv.low
	) ? 1 : 0);
	acc = uint32_t(r);
	acc *= l >> 32;
	rv.high += (acc >> 32) + (__builtin_uaddll_overflow(
		rv.low, acc << 32, &rv.low
	) ? 1 : 0);
	acc = r >> 32;
	acc *= l >> 32;
	rv.high += acc;
	return rv;
}

static inline soft_uint256_t multiply(uint128_t l, uint128_t r)
{
	soft_uint256_t rv;
	rv.low = multiply(l.low, r.low);
	soft_uint128_t acc(multiply(l.low, r.high));
	rv.high.low = acc.high + (__builtin_uaddll_overflow(
		rv.low.high, acc.low, &rv.low.high
	) ? 1 : 0);

	acc = multiply(l.high, r.low);
	if (__builtin_uaddll_overflow(
		rv.low.high, acc.low, &rv.low.high
	)) {
		rv.high.high = __builtin_uaddll_overflow(
			rv.high.low, acc.high + 1, &rv.high.low
		) ? 1 : 0;
	} else {
		rv.high.high = __builtin_uaddll_overflow(
			rv.high.low, acc.high, &rv.high.low
		) ? 1 : 0;
	}

	acc = multiply(l.high, r.high);
	rv.high.high += acc.high + (__builtin_uaddll_overflow(
		rv.high.low, acc.low, &rv.high.low
	) ? 1 : 0);
	return rv;
}
}}
#endif
}
#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_INT128_20140709T2300)
#define UCPF_YESOD_DETAIL_INT128_20140709T2300

#if defined(_GLIBCXX_USE_INT128)

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

namespace ucpf { namespace yesod {

inline std::pair<uint64_t, uint64_t> multiply(uint64_t x, uint64_t y)
{
	int128_t acc(x);
	x *= y;
	return std::make_pair(uint64_t(acc), uint64_t(acc >> 64));
}

inline std::pair<uint128_t, uint128_t> multiply(uint128_t x, uint128_t y)
{
	uint64_t xh(x >> 64), xl(x);
	uint64_t yh(y >> 64), yl(y);
	uint128_t acc_h(xh);
	acc_h *= yh;
	uint128_t acc_l(xl);
	acc_l *= yl;
	uint128_t acc_m(xh >= xl ? xh - xl : xl - xh);
	acc_m *= (yh >= yl ? yh - yl : yl - yh);

	if ((xh >= xl) != (yh >= yl))
		acc_m += acc_h + acc_l;
	else
		acc_m = acc_h + acc_l - acc_m;

	acc_m += acc_l >> 64;
	acc_h += acc_m >> 64;

	return std::make_pair(
		(acc_m << 64) | (acc_l & ~uint64_t(0)), acc_h
	);
}

}}

#else

struct [[gnu::packed]] int128_t {
	uint64_t low;
	uint64_t high;

	int128_t(uint64_t other)
	: low(other), high(other >> 63 ? ~uint64_t(0) : uint64_t(0))
	{}
};

struct [[gnu::packed]] uint128_t {
	uint64_t low;
	uint64_t high;

	uint128_t(uint64_t other)
	: low(other), high(uint64_t(0))
	{}
};

#endif

#endif

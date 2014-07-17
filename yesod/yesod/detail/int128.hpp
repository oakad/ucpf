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
	acc *= y;
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

namespace ucpf { namespace yesod {

inline std::pair<uint64_t, uint64_t> multiply(uint64_t x, uint64_t y)
{
	uint32_t xh(x >> 32), xl(x);
	uint32_t yh(y >> 32), yl(y);
	uint64_t acc_h(xh);
	acc_h *= yh;
	uint64_t acc_l(xl);
	acc_l *= yl;
	uint64_t acc_m(xh >= xl ? xh - xl : xl - xh);
	acc_m *= (yh >= yl ? yh - yl : yl - yh);

	if ((xh >= xl) != (yh >= yl))
		acc_m += acc_h + acc_l;
	else
		acc_m = acc_h + acc_l - acc_m;

	acc_m += acc_l >> 32;
	acc_h += acc_m >> 32;

	return std::make_pair(
		(acc_m << 32) | (acc_l & ~uint32_t(0)), acc_h
	);
}

}}

struct [[gnu::packed]] int128_t {
	uint64_t low;
	uint64_t high;

	constexpr int128_t()
	: low(0), high(0)
	{}

	constexpr int128_t(int64_t other)
	: low(other), high(other >= 0 ? uint64_t(0) : ~uint64_t(0))
	{}
};

struct [[gnu::packed]] uint128_t {
	uint64_t low;
	uint64_t high;

	constexpr uint128_t()
	: low(0), high(0)
	{}

	constexpr uint128_t(uint64_t other)
	: low(other), high(0)
	{}

	constexpr uint128_t(uint64_t low_, uint64_t high_)
	: low(low_), high(high_)
	{}

	constexpr uint128_t(std::pair<uint64_t, uint64_t> other)
	: low(other.first), high(other.second)
	{}

	constexpr uint128_t operator>>(int shift) const
	{
		return (shift & 0x40)
		       ? uint128_t{high >> (shift & 0x3f), 0}
		       : uint128_t{
				(low >> (shift & 0x3f))
				| high << (0x80 - (shift & 0x3f)),
				high >> (shift & 0x3f)
			};
	}

	uint128_t operator+(uint64_t other) const
	{
		auto xl(low + other);
		auto c(((low & other) | ((low | other) & ~xl)) >> 63);
		return uint128_t(xl, high + c);
	}

	uint128_t operator+(uint128_t other) const
	{
		auto xl(low + other.low);
		auto c(((low & other.low) | ((low | other.low) & ~xl)) >> 63);
		return uint128_t(xl, high + other.high + c);
	}

	uint128_t operator-(uint64_t other) const
	{
		auto xl(low - other);
		auto c(((~low & other) | ((~low | other) & xl)) >> 63);
		return uint128_t(xl, high - c);
	}

	uint128_t operator-(uint128_t other) const
	{
		auto xl(low - other.low);
		auto c(((~low & other.low) | ((~low | other.low) & xl)) >> 63);
		return uint128_t(xl, high - other.high - c);
	}

	constexpr operator uint64_t() const
	{
		return low;
	}

	uint128_t &operator<<=(int shift)
	{
		if (shift & 0x40) {
			high = low << (shift & 0x3f);
			low = 0;
		} else {
			high <<= shift & 0x3f;
			high |= low >> (0x80 - (shift & 0x3f));
			low <<= shift & 0x3f;
		}
		return *this;
	}

	uint128_t &operator|=(uint64_t other)
	{
		low |= other;
		return *this;
	}

	uint128_t &operator+=(uint64_t other)
	{
		auto xl(low + other);
		auto c(((low & other) | ((low | other) & ~xl)) >> 63);
		low = xl;
		high += c;
		return *this;
	}

	uint128_t &operator*=(uint64_t other)
	{
		auto acc_l(ucpf::yesod::multiply(low, other));
		low = acc_l.first;
		auto acc_h(ucpf::yesod::multiply(high, other));
		high = acc_h.first + acc_l.second;
		return *this;
	}
};

namespace ucpf { namespace yesod {

inline std::pair<uint128_t, uint128_t> multiply(uint128_t x, uint128_t y)
{
	auto xh(x.high), xl(x.low);
	auto yh(y.high), yl(y.low);
	uint128_t acc_h(multiply(xh, yh));
	uint128_t acc_l(multiply(xl, yl));
	uint128_t acc_m(multiply(
		xh >= xl ? xh - xl : xl - xh,
		yh >= yl ? yh - yl : yl - yh
	));

	if ((xh >= xl) != (yh >= yl))
		acc_m += acc_h + acc_l;
	else
		acc_m = acc_h + acc_l - acc_m;

	acc_m += acc_l.high;
	acc_h += acc_m.high;

	return std::make_pair(
		uint128_t(acc_l.low, acc_m.low), acc_h
	);
}

}}

#endif

#endif

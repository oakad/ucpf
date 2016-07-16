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

	constexpr soft_uint128_t(uint64_t low_)
	: low(low_), high(0)
	{}

	constexpr soft_uint128_t(uint64_t high_, uint64_t low_)
	: low(low_), high(high_)
	{}

#if defined(_GLIBCXX_USE_INT128)
	constexpr soft_uint128_t(uint128_t v)
	: low(v), high(v >> 64)
	{}

	constexpr operator uint128_t() const
	{
		return *reinterpret_cast<uint128_t const *>(this);
	}
#endif

	uint64_t round_half() const;

	explicit operator bool() const;
	explicit operator int32_t() const;
	explicit operator uint32_t() const;
	soft_uint128_t &operator++();
	soft_uint128_t &operator--();
	soft_uint128_t operator+(soft_uint128_t v) const;
	soft_uint128_t &operator+=(soft_uint128_t v);
	soft_uint128_t operator-(uint64_t v) const;
	soft_uint128_t operator-(soft_uint128_t v) const;
	soft_uint128_t operator*(soft_uint128_t v) const;
	soft_uint128_t &operator*=(uint64_t v);
	soft_uint128_t operator&(uint64_t v) const;
	soft_uint128_t operator&(soft_uint128_t v) const;
	soft_uint128_t &operator&=(soft_uint128_t v);
	soft_uint128_t &operator|=(uint64_t v);
	soft_uint128_t operator>>(std::size_t pos) const;
	soft_uint128_t &operator>>=(std::size_t pos);
	soft_uint128_t operator<<(std::size_t pos) const;
	soft_uint128_t &operator<<=(std::size_t pos);
	bool operator<(soft_uint128_t v) const;
	bool operator<=(soft_uint128_t v) const;
	bool operator>(soft_uint128_t v) const;
	bool operator>=(soft_uint128_t v) const;

	uint64_t low;
	uint64_t high;
};

struct [[gnu::packed]] soft_uint256_t {
	soft_uint128_t round_half() const;

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

	constexpr soft_uint128_t(uint64_t low_)
	: high(0), low(low_)
	{}

	constexpr soft_uint128_t(uint64_t high_, uint64_t low_)
	: high(high_), low(low_)
	{}

#if defined(_GLIBCXX_USE_INT128)
	constexpr soft_uint128_t(uint128_t v)
	: high(v >> 64), low(v)
	{}

	constexpr operator uint128_t() const
	{
		return *reinterpret_cast<uint128_t const *>(&high);
	}
#endif

	uint64_t round_half() const;

	explicit operator bool() const;
	explicit operator int32_t() const;
	explicit operator uint32_t() const;
	soft_uint128_t &operator++();
	soft_uint128_t &operator--();
	soft_uint128_t operator+(soft_uint128_t v) const;
	soft_uint128_t &operator+=(soft_uint128_t v);
	soft_uint128_t operator-(uint64_t v) const;
	soft_uint128_t operator-(soft_uint128_t v) const;
	soft_uint128_t operator*(soft_uint128_t v) const;
	soft_uint128_t &operator*=(uint64_t v);
	soft_uint128_t operator&(uint64_t v) const;
	soft_uint128_t operator&(soft_uint128_t v) const;
	soft_uint128_t &operator&=(soft_uint128_t v);
	soft_uint128_t &operator|=(uint64_t v);
	soft_uint128_t operator>>(std::size_t pos) const;
	soft_uint128_t &operator>>=(std::size_t pos);
	soft_uint128_t operator<<(std::size_t pos) const;
	soft_uint128_t &operator<<=(std::size_t pos);
	bool operator<(soft_uint128_t v) const;
	bool operator<=(soft_uint128_t v) const;
	bool operator>(soft_uint128_t v) const;
	bool operator>=(soft_uint128_t v) const;

	uint64_t high;
	uint64_t low;
};

struct [[gnu::packed]] soft_uint256_t {
	soft_uint128_t round_half() const;

	soft_uint128_t high;
	soft_uint128_t low;
};

#endif

uint64_t soft_uint128_t::round_half() const
{
	return high + (low >> 63);
}

soft_uint128_t::operator bool() const
{
	return high || low;
}

soft_uint128_t::operator int32_t() const
{
	return static_cast<int32_t>(low);
}

soft_uint128_t::operator uint32_t() const
{
	return static_cast<uint32_t>(low);
}

soft_uint128_t &soft_uint128_t::operator++()
{
	if (__builtin_add_overflow(low, 1, &low))
		++high;
}

soft_uint128_t &soft_uint128_t::operator--()
{
	if (__builtin_sub_overflow(low, 1, &low))
		--high;
}

soft_uint128_t soft_uint128_t::operator+(soft_uint128_t v) const
{
	soft_uint128_t rv;
	if (__builtin_add_overflow(low, v.low, &rv.low))
		rv.high = high + v.high + 1;
	else
		rv.high = high + v.high;

	return rv;
}

soft_uint128_t soft_uint128_t::operator-(uint64_t v) const
{
	soft_uint128_t rv;
	if (__builtin_sub_overflow(low, v, &rv.low))
		rv.high = high - 1;
	else
		rv.high = high;

	return rv;
}

soft_uint128_t soft_uint128_t::operator-(soft_uint128_t v) const
{
	soft_uint128_t rv;
	if (__builtin_sub_overflow(low, v.low, &rv.low))
		rv.high = high - v.high - 1;
	else
		rv.high = high - v.high;

	return rv;
}

soft_uint128_t &soft_uint128_t::operator+=(soft_uint128_t v)
{
	if (__builtin_add_overflow(low, v.low, &low))
		++high;

	high += v.high;
	return *this;
}

soft_uint128_t soft_uint128_t::operator&(uint64_t v) const
{
	return soft_uint128_t(0, low & v);
}

soft_uint128_t soft_uint128_t::operator&(soft_uint128_t v) const
{
	return soft_uint128_t(high & v.high, low & v.low);
}

soft_uint128_t &soft_uint128_t::operator&=(soft_uint128_t v)
{
	high &= v.high;
	low &= v.low;
	return *this;
}

soft_uint128_t &soft_uint128_t::operator|=(uint64_t v)
{
	low |= v;
	return *this;
}

soft_uint128_t soft_uint128_t::operator>>(std::size_t pos) const
 {
	if (pos >= 64)
		return soft_uint128_t(high >> (pos - 64));
	else
		return soft_uint128_t(
			high >> pos,
			(high << (64 - pos)) | (low >> pos)
		);
}

soft_uint128_t &soft_uint128_t::operator>>=(std::size_t pos)
{
	if (pos >= 64) {
		low = high >> (pos - 64);
		high = 0;
	} else {
		low >>= pos;
		low |= high << (64 - pos);
		high >>= pos;
	}

	return *this;
}

soft_uint128_t soft_uint128_t::operator<<(std::size_t pos) const
 {
	if (pos >= 64)
		return soft_uint128_t(low << (pos - 64), 0);
	else
		return soft_uint128_t(
			high << pos | (low >> (64 - pos)),
			low << pos
		);
}

soft_uint128_t &soft_uint128_t::operator<<=(std::size_t pos)
{
	if (pos >= 64) {
		high = low << (pos - 64);
		low = 0;
	} else {
		high <<= pos;
		high |= low >> (64 - pos);
		low <<= pos;
	}

	return *this;
}

bool soft_uint128_t::operator<(soft_uint128_t v) const
{
	if (high > v.high)
		return false;

	if (high < v.high)
		return true;

	return low < v.low;
}

bool soft_uint128_t::operator<=(soft_uint128_t v) const
{
	if (high > v.high)
		return false;

	if (high < v.high)
		return true;

	return low <= v.low;
}

bool soft_uint128_t::operator>(soft_uint128_t v) const
{
	if (high < v.high)
		return false;

	if (high > v.high)
		return true;

	return low > v.low;
}

bool soft_uint128_t::operator>=(soft_uint128_t v) const
{
	if (high < v.high)
		return false;

	if (high > v.high)
		return true;

	return low >= v.low;
}

soft_uint128_t soft_uint256_t::round_half() const
{
	soft_uint128_t rv{};

	if (__builtin_add_overflow(high.low, low.high >> 63, &rv.low))
		rv.high = high.high + 1;
	else
		rv.high = high.high;

	return rv;
}

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
	rv.high.low = __builtin_add_overflow(
		rv.low.high, uint64_t(acc), &rv.low.high
	) ? uint64_t(acc >> 64) + 1 : uint64_t(acc >> 64);

	acc = uint64_t(r);
	acc *= l >> 64;
	if (__builtin_add_overflow(
		rv.low.high, uint64_t(acc), &rv.low.high
	)) {
		rv.high.high = __builtin_add_overflow(
			rv.high.low, uint64_t(acc >> 64) + 1, &rv.high.low
		) ? 1 : 0;
	} else {
		rv.high.high = __builtin_add_overflow(
			rv.high.low, uint64_t(acc >> 64), &rv.high.low
		) ? 1 : 0;
	}

	acc = r >> 64;
	acc *= l >> 64;
	rv.high.high += __builtin_add_overflow(
		rv.high.low, uint64_t(acc), &rv.high.low
	) ? uint64_t(acc >> 64) + 1 : uint64_t(acc >> 64);
	return rv;
}

}}

soft_uint128_t &soft_uint128_t::operator*=(uint64_t v)
{
	auto v0(holam::support::multiply(low, v));
	auto v1(holam::support::multiply(high, v));
	low = v0;
	high = v0 >> 64;
	high += v1;
	return *this;
}

#else

typedef soft_int128_t int128_t;
typedef soft_uint128_t uint128_t;

namespace holam { namespace support {

static inline soft_uint128_t multiply(uint64_t l, uint64_t r)
{
	soft_uint128_t rv;
	rv.low = uint32_t(r);
	rv.low *= uint32_t(l);
	uint64_t acc(r >> 32);
	acc *= uint32_t(l);
	rv.high = (acc >> 32) + (__builtin_add_overflow(
		rv.low, acc << 32, &rv.low
	) ? 1 : 0);
	acc = uint32_t(r);
	acc *= l >> 32;
	rv.high += (acc >> 32) + (__builtin_add_overflow(
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
	rv.high.low = acc.high + (__builtin_add_overflow(
		rv.low.high, acc.low, &rv.low.high
	) ? 1 : 0);

	acc = multiply(l.high, r.low);
	if (__builtin_add_overflow(
		rv.low.high, acc.low, &rv.low.high
	)) {
		rv.high.high = __builtin_add_overflow(
			rv.high.low, acc.high + 1, &rv.high.low
		) ? 1 : 0;
	} else {
		rv.high.high = __builtin_add_overflow(
			rv.high.low, acc.high, &rv.high.low
		) ? 1 : 0;
	}

	acc = multiply(l.high, r.high);
	rv.high.high += acc.high + (__builtin_add_overflow(
		rv.high.low, acc.low, &rv.high.low
	) ? 1 : 0);
	return rv;
}

}}

soft_uint128_t &soft_uint128_t::operator*=(uint64_t v)
{
	auto v0(holam::support::multiply(low, v));
	auto v1(holam::support::multiply(high, v));
	low = v0.low;
	high = v0.high + v1.low;
	return *this;
}
#endif

soft_uint128_t soft_uint128_t::operator*(soft_uint128_t v) const
{
	return holam::support::multiply(*this, v).low;
}

}
#endif

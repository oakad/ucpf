/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_INT128_20140709T2300)
#define UCPF_YESOD_DETAIL_INT128_20140709T2300

#include <cstdint>
#include <cstdio>
#if defined(_GLIBCXX_USE_INT128)

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

namespace ucpf { namespace yesod { namespace detail {

inline std::pair<uint64_t, uint64_t> multiply(uint64_t l, uint64_t r)
{
	int128_t acc(l);
	acc *= r;
	return std::make_pair(uint64_t(acc), uint64_t(acc >> 64));
}

inline std::pair<uint128_t, uint128_t> multiply(uint128_t l, uint128_t r)
{
	uint64_t m[4] = {0};
	std::size_t m_pos(0);
	for (int rc(0); rc < 128; rc += 64) {
		auto n_pos(m_pos);
		uint64_t c(0);
		for (int lc(0); lc < 128; lc += 64) {
			uint128_t acc(uint64_t(r >> rc));
			acc *= uint64_t(l >> lc);
			acc += m[n_pos];
			acc += c;
			c = acc >> 64;
			m[n_pos++] = acc;
		}
		m[n_pos] = c;
		++m_pos;
	}
	std::pair<uint128_t, uint128_t> rv(m[1], m[3]);
	rv.first = (rv.first << 64) | m[0];
	rv.second = (rv.second << 64) | m[2];
	return rv;
}

inline uint32_t divide_near(uint128_t &num, uint64_t denom)
{
	auto rv(num / denom);
	num %= denom;
	return rv;
}

}}}

#else

namespace ucpf { namespace yesod { namespace detail {

inline std::pair<uint64_t, uint64_t> multiply(uint64_t l, uint64_t r)
{
	uint32_t m[4] = {0};
	std::size_t m_pos(0);
	for (int rc(0); rc < 64; rc += 32) {
		auto n_pos(m_pos);
		uint32_t c(0);
		for (int lc(0); lc < 64; lc += 32) {
			uint64_t acc(uint32_t(r >> rc));
			acc *= uint32_t(l >> lc);
			acc += m[n_pos];
			acc += c;
			c = acc >> 32;
			m[n_pos++] = acc;
		}
		m[n_pos] = c;
		++m_pos;
	}
	std::pair<uint64_t, uint64_t> rv(m[1], m[3]);
	rv.first = (rv.first << 32) | m[0];
	rv.second = (rv.second << 32) | m[2];
	return rv;
}

}}}

struct [[gnu::packed]] int128_t {
	uint64_t v[2];

	constexpr int128_t()
	: v{0, 0}
	{}

	constexpr int128_t(int64_t other)
	: v{uint64_t(other), other >= 0 ? uint64_t(0) : ~uint64_t(0)}
	{}
};

struct [[gnu::packed]] uint128_t {
	uint64_t v[2];

	constexpr uint128_t()
	: v{0, 0}
	{}

	constexpr uint128_t(uint64_t other)
	: v{other, 0}
	{}

	constexpr uint128_t(uint64_t low, uint64_t high)
	: v{low, high}
	{}

	constexpr uint128_t(std::pair<uint64_t, uint64_t> other)
	: v{other.first, other.second}
	{}

	constexpr uint128_t operator>>(unsigned int shift) const
	{
		return (shift & 0x40)
		       ? uint128_t{v[1] >> (shift & 0x3f), 0}
		       : uint128_t{
				(v[0] >> (shift & 0x3f))
				| (v[1] << (0x80 - (shift & 0x3f))),
				v[1] >> (shift & 0x3f)
			};
	}

	constexpr uint128_t operator<<(unsigned int shift) const
	{
		return (shift & 0x40)
		       ? uint128_t{0, v[0] << (shift & 0x3f)}
		       : uint128_t{
			       v[0] << (shift & 0x3f),
			       (v[0] >> (0x80 - (shift & 0x3f)))
			       | (v[1] << (shift & 0x3f))
			};
	}

	constexpr uint128_t operator>>(int shift) const
	{
		return (shift >= 0)
		       ? *this >> (unsigned int)shift
		       : *this << (unsigned int)-shift;
	}

	constexpr uint128_t operator<<(int shift) const
	{
		return (shift >= 0)
		       ? *this << (unsigned int)shift
		       : *this >> (unsigned int)-shift;
	}

	constexpr uint128_t operator+(int other) const
	{
		return (other >= 0)
		       ? *this + uint64_t(other)
		       : *this - uint64_t(-other);
	}

	constexpr uint128_t operator+(uint64_t other) const
	{
		return (
			other <= ((~uint64_t(0)) - v[0])
		) ? uint128_t(v[0] + other, v[1]) : uint128_t(
			other - ((~uint64_t(0)) - v[0]) - 1, v[1] + 1
		);
	}

	uint128_t operator+(uint128_t other) const
	{
		auto xl(v[0] + other.v[0]);
		auto c((
			(v[0] & other.v[0]) | ((v[0] | other.v[0]) & ~xl)
		) >> 63);
		return uint128_t(xl, v[1] + other.v[1] + c);
	}

	constexpr uint128_t operator-(int other) const
	{
		return (other >= 0)
		       ? *this - uint64_t(other)
		       : *this + uint64_t(-other);
	}

	constexpr uint128_t operator-(uint64_t other) const
	{
		return (
			other <= v[0]
		) ? uint128_t(v[0] - other, v[1]) : uint128_t(
			(~uint64_t(0)) - (other - v[0]) + 1, v[1] - 1
		);
	}

	uint128_t operator-(uint128_t other) const
	{
		auto xl(v[0] - other.v[0]);
		auto c((
			(~v[0] & other.v[0]) | ((~v[0] | other.v[0]) & xl)
		) >> 63);
		return uint128_t(xl, v[1] - other.v[1] - c);
	}

	uint128_t operator&(uint32_t other) const
	{
		return uint128_t(v[0] & other, 0);
	}

	uint128_t operator&(uint128_t other) const
	{
		return uint128_t(v[0] & other.v[0], v[1] & other.v[1]);
	}

	constexpr bool operator<(uint64_t other) const
	{
		return v[1] ? true : (v[0] > other);
	}

	constexpr bool operator<(uint128_t other) const
	{
		return v[1] == other.v[1]
		       ? (v[0] > other.v[0]) : (v[1] > other.v[1]);
	}

	constexpr operator uint64_t() const
	{
		return v[0];
	}

	uint128_t &operator++()
	{
		if (~v[0])
			++v[0];
		else {
			v[0] = ~v[0];
			++v[1];
		}
		return *this;
	}

	uint128_t &operator--()
	{
		if (v[0])
			--v[0];
		else {
			v[0] = ~v[0];
			--v[1];
		}
		return *this;
	}

	uint128_t &operator<<=(unsigned int shift)
	{
		if (shift & 0x40) {
			v[1] = v[0] << (shift & 0x3f);
			v[0] = 0;
		} else {
			v[1] <<= shift & 0x3f;
			v[1] |= v[0] >> (0x80 - (shift & 0x3f));
			v[0] <<= shift & 0x3f;
		}
		return *this;
	}

	uint128_t &operator>>=(unsigned int shift)
	{
		if (shift & 0x40) {
			v[0] = v[1] >> (shift & 0x3f);
			v[1] = 0;
		} else {
			v[0] >>= shift & 0x3f;
			v[0] |= v[1] << (0x80 - (shift & 0x3f));
			v[1] >>= shift & 0x3f;
		}
		return *this;
	}

	uint128_t &operator|=(uint64_t other)
	{
		v[0] |= other;
		return *this;
	}

	uint128_t &operator+=(uint64_t other)
	{
		auto xl(v[0] + other);
		auto c(((v[0] & other) | ((v[0] | other) & ~xl)) >> 63);
		v[0] = xl;
		v[1] += c;
		return *this;
	}

	uint128_t &operator+=(uint128_t other)
	{
		auto xl(v[0] + other.v[0]);
		auto c((
			(v[0] & other.v[0]) | ((v[0] | other.v[0]) & ~xl)
		) >> 63);
		v[0] = xl;
		v[1] += c;
		v[1] += other.v[1];
		return *this;
	}

	uint128_t &operator-=(uint64_t other)
	{
		auto xl(v[0] - other);
		auto c(((~v[0] & other) | ((~v[0] | other) & xl)) >> 63);
		v[0] = xl;
		v[1] -= c;
		return *this;
	}

	uint128_t &operator*=(uint64_t other)
	{
		auto acc_l(ucpf::yesod::detail::multiply(v[0], other));
		v[0] = acc_l.first;
		auto acc_h(ucpf::yesod::detail::multiply(v[1], other));
		v[1] = acc_h.first + acc_l.second;
		return *this;
	}

	uint128_t &operator&=(uint128_t other)
	{
		v[0] &= other.v[0];
		v[1] &= other.v[1];
		return *this;
	}
};

namespace ucpf { namespace yesod { namespace detail {

inline std::pair<uint128_t, uint128_t> multiply(uint128_t l, uint128_t r)
{
	uint64_t m[4] = {0};
	std::size_t m_pos(0);
	for (int rc(0); rc < 2; ++rc) {
		auto n_pos(m_pos);
		uint64_t c(0);
		for (int lc(0); lc < 2; ++lc) {
			uint128_t acc(r.v[rc]);
			acc *= l.v[lc];
			acc += m[n_pos];
			acc += c;
			c = acc.v[1];
			m[n_pos++] = acc.v[0];
		}
		m[n_pos] = c;
		++m_pos;
	}

	return std::make_pair(uint128_t(m[0], m[1]), uint128_t(m[2], m[3]));
}

inline uint32_t divide_near(uint128_t &num, uint64_t denom)
{
	auto denom_sz(64 - __builtin_clzll(denom));
	uint32_t rv(0);

	while (true) {
		if (!num.v[1]) {
			rv += num.v[0] / denom;
			num.v[0] %= denom;
			return rv;
		}

		auto num_sz(128 - __builtin_clzll(num.v[1]));
		if (num_sz <= (denom_sz + 1)) {
			while (num.v[1] || (num.v[0] >= denom)) {
				num -= denom;
				++rv;
			}
			return rv;
		}

		auto order(num_sz - denom_sz - 1);
		auto ol(denom << order);
		auto oh(denom >> (64 - order));
		if (ol <= num.v[0]) {
			num.v[0] -= ol;
			num.v[1] -= oh;
		} else {
			num.v[0] = (~uint64_t(0)) - (ol - num.v[0]) + 1;
			num.v[1] -= oh + 1;
		}
		rv += uint32_t(1) << order;
	}
}

}}}

#endif

#endif

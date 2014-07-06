/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_BIGINT_20140706T1800)
#define UCPF_MINA_DETAIL_BIGINT_20140706T1800

#include <gmp.h>
#include <array>
#include <vector>

namespace ucpf { namespace mina { namespace detail {

constexpr std::array<uint64_t, 20> small_power_10 = {{
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
	1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000,
	100000000000000, 1000000000000000, 10000000000000000,
	100000000000000000, 1000000000000000000, 10000000000000000000ull
}};

constexpr int small_power_10_estimate(int order_2)
{
	return (order_2 * 1233) >> 12;
}

typedef mp_limb_t bigint_limb_type;

template <typename Vector>
auto bigint_to_ascii_decimal(Vector const &v)
{
	typedef std::vector<
		uint8_t,
		typename std::allocator_traits<
			typename Vector::allocator_type
		>::template rebind_alloc<uint8_t>
	> string_type;

	constexpr static auto max_order = small_power_10_estimate(
		GMP_NUMB_BITS
	);

	auto xv(v);
	auto str_sz(v.size() * max_order + v.size() + 1);
	string_type str(str_sz, v.get_allocator());
	auto sz(mpn_get_str(&str.front(), 10, &xv.front(), xv.size()));
	for (auto c(0); c < sz; c++)
		str[c] += '0';

	str.resize(sz + 1);
	return str;
}

template <typename Vector, typename T>
void bigint_assign_scalar(Vector &v, T value, size_t order = 0)
{
	typedef typename Vector::value_type limb_type;
	constexpr static int limb_bits = GMP_NUMB_BITS;
	constexpr static int value_bits = std::numeric_limits<T>::digits;

	auto bit_cnt(value_bits + order);
	auto limb_cnt(bit_cnt / limb_bits);
	if ((limb_cnt * limb_bits) < bit_cnt)
		++limb_cnt;

	v.clear();
	v.resize(limb_cnt, 0);
	auto iter(v.begin());
	decltype(bit_cnt) bit_pos(0);
	while ((bit_pos + limb_bits) <= order) {
		*iter++ = 0;
		bit_pos += limb_bits;
	}

	auto bit_off(order - bit_pos);
	limb_type x(value);
	x <<= bit_off;
	x &= GMP_NUMB_MASK;
	*iter++ = x;
	bit_pos += limb_bits;
	bit_off = limb_bits - bit_off;
	if (bit_off < value_bits)
		value >>= bit_off;
	else
		return;

	while (value) {
		x = value;
		x &= GMP_NUMB_MASK;
		*iter++ = x;
		value >>= std::min(value_bits, limb_bits);
	}
}

template <typename Vector>
void bigint_shift_left(Vector &v, size_t order)
{
	constexpr static int max_order = GMP_NUMB_BITS - 1;

	v.reserve(v.size() + (order / max_order + 1));
	auto c(mpn_lshift(&v.front(), &v.front(), v.size(), order % max_order));
	if (c)
		v.push_back(c);

	order -= order % max_order;
	for (; order; order -= max_order) {
		c = mpn_lshift(
			&v.front(), &v.front(), v.size(), max_order
		);
		if (c)
			v.push_back(c);
	}
}

template <typename Vector>
void bigint_assign_pow10(Vector &v, size_t order)
{
	constexpr static auto max_order = small_power_10_estimate(
		GMP_NUMB_BITS
	);

	v.clear();
	v.reserve(order / max_order + 1);
	v.push_back(small_power_10[order % max_order]);
	order -= order % max_order;
	for(; order; order -= max_order) {
		auto c(mpn_mul_1(
			&v.front(), &v.front(), v.size(),
			small_power_10[max_order]
		));
		if (c)
			v.push_back(c);
	}
}

template <typename Vector>
void bigint_mul(Vector &l, Vector const &r)
{
	Vector acc(l.size() + r.size(), 0, l.get_allocator());
	mpn_mul(&acc.front(), &l.front(), l.size(), &r.front(), r.size());
	l.swap(acc);
}

}}}

#endif

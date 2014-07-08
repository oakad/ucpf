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
	constexpr static int limb_bits(GMP_NUMB_BITS);
	constexpr static int value_bits(std::numeric_limits<T>::digits);

	auto bit_cnt(value_bits + order);
	auto limb_cnt(bit_cnt / limb_bits);
	if ((limb_cnt * limb_bits) < bit_cnt)
		++limb_cnt;

	v.clear();
	v.reserve(limb_cnt);
	decltype(bit_cnt) bit_pos(0);
	while ((bit_pos + limb_bits) <= order) {
		v.push_back(0);
		bit_pos += limb_bits;
	}

	auto bit_off(order - bit_pos);
	bigint_limb_type x(value);
	x <<= bit_off;
	x &= GMP_NUMB_MASK;
	v.push_back(x);
	bit_pos += limb_bits;
	bit_off = limb_bits - bit_off;
	if (bit_off < value_bits)
		value >>= bit_off;
	else
		return;

	while (value) {
		x = value;
		x &= GMP_NUMB_MASK;
		v.push_back(x);
		value >>= std::min(value_bits, limb_bits);
	}
}

template <typename Vector>
void bigint_shift_left(Vector &v, size_t order)
{
	constexpr static int max_order(GMP_NUMB_BITS - 1);

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
	constexpr static auto max_order(small_power_10_estimate(GMP_NUMB_BITS));

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
	if (l.size() >= r.size())
		mpn_mul(
			&acc.front(), &l.front(), l.size(), &r.front(), r.size()
		);
	else
		mpn_mul(
			&acc.front(), &r.front(), r.size(), &l.front(), l.size()
		);

	l.swap(acc);
}

template <typename Vector>
int bigint_compare(Vector const &l, Vector const &r)
{
	if (l.size() > r.size())
		return 1;
	else if (r.size() > l.size())
		return -1;
	else
		return mpn_cmp(&l.front(), &r.front(), l.size());
}

template <typename Vector>
int bigint_compare_sum(Vector const &l0, Vector const &l1, Vector const &r)
{
	auto l_sz(std::max(l0.size(), l1.size()) + 1);

	if (r.size() > l_sz)
		return -1;
	else if ((l_sz - r.size()) > 1)
		return 1;

	bigint_limb_type sum[l_sz];

	if (l0.size() >= l1.size())
		sum[l_sz - 1] = mpn_add(
			sum, &l0.front(), l0.size(), &l1.front(), l1.size()
		);
	else
		sum[l_sz - 1] = mpn_add(
			sum, &l1.front(), l1.size(), &l0.front(), l0.size()
		);

	if (sum[l_sz - 1]) {
		if (r.size() == l_sz)
			return mpn_cmp(sum, &r.front(), l_sz);
		else
			return 1;
	} else {
		if (r.size() == (l_sz - 1))
			return mpn_cmp(sum, &r.front(), l_sz - 1);
		else
			return -1;
	}
}

template <typename Vector, typename T>
void bigint_mul_scalar(Vector &l, T r)
{
	constexpr static int limb_bits(GMP_NUMB_BITS);
	constexpr static int value_bits(std::numeric_limits<T>::digits);
	constexpr static int max_shift(
		limb_bits > value_bits ? value_bits : limb_bits
	);

	if (value_bits > limb_bits) {
		Vector xr(value_bits / limb_bits + 1, 0, l.get_allocator());
		xr.clear();
		while (r) {
			xr.push_back(r & GMP_NUMB_MASK);
			r >>= max_shift;
		}
		Vector acc(xr.size() + l.size(), 0, l.get_allocator());
		mpn_mul(
			&acc.front(), &l.front(), l.size(), &xr.front(),
			xr.size()
		);
		l.swap(acc);
	} else {
		auto c(mpn_mul_1(&l.front(), &l.front(), l.size(), r));
		if (c)
			l.push_back(c);
	}
}

template <typename Vector>
void bigint_div(
	Vector &quot, Vector &rem, Vector const &num, Vector const &denom
){
	quot.clear();
	rem.clear();

	if (denom.size() > num.size()) {
		quot.push_back(0);
		rem = denom;
		return;
	}
	quot.resize(num.size() - denom.size() + 1, 0);
	rem.resize(denom.size(), 0);
	mpn_tdiv_qr(
		&quot.front(), &rem.front(), 0, &num.front(),num.size(),
		&denom.front(), denom.size()
	);
}

}}}

#endif

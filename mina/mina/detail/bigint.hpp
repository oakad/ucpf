/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_BIGINT_20140706T1800)
#define UCPF_MINA_DETAIL_BIGINT_20140706T1800

#include <vector>
#include <algorithm>
#include <yesod/bitops.hpp>

namespace ucpf { namespace mina { namespace detail {

constexpr uint64_t small_power_10[20] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
	1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000,
	100000000000000, 1000000000000000, 10000000000000000,
	100000000000000000, 1000000000000000000ull
};

constexpr int small_power_10_estimate(int order_2)
{
	return (order_2 * 1233) >> 12;
}

template <unsigned int N>
struct bigint_calc_traits;

template <>
struct bigint_calc_traits<32> {
	typedef uint32_t limb_type;
	typedef uint64_t acc_type;
	constexpr static unsigned int limb_bits = 32;
	constexpr static unsigned int limb_digits_10 = 9;
};

template <>
struct bigint_calc_traits<64> {
	typedef uint64_t limb_type;
	typedef uint128_t acc_type;
	constexpr static unsigned int limb_bits = 64;
	constexpr static unsigned int limb_digits_10 = 18;
};

struct bigint {
	typedef typename std::conditional<
		std::is_fundamental<uint128_t>::value,
		bigint_calc_traits<64>, bigint_calc_traits<32>
	>::type traits_type;

	typedef typename traits_type::limb_type limb_type;
	typedef typename traits_type::acc_type acc_type;
	constexpr static unsigned int limb_bits = traits_type::limb_bits;
	constexpr static unsigned int limb_digits_10
	= traits_type::limb_digits_10;

	template <typename Vector>
	static auto to_ascii_hex(Vector const &v)
	{
		typedef std::vector<
			uint8_t,
			typename std::allocator_traits<
				typename Vector::allocator_type
			>::template rebind_alloc<uint8_t>
		> string_type;

		string_type str(v.get_allocator());
		std::for_each(
			v.rbegin(), v.rend(), [&str](limb_type d) -> void {
				for (auto c(limb_bits); c > 0; c -= 4) {
					auto h((d >> (c - 4)) & 0xf);
					str.push_back(
						h < 10 ? h + '0' : h + '7'
					);
				}
			}
		);
		return str;
	}

	template <typename Vector, typename T>
	static void assign_scalar(Vector &v, T value, size_t order = 0)
	{
		size_t bit_pos(0);
		v.reserve(
			(yesod::fls(value) + order) / limb_bits  + 1
		);
		while ((bit_pos + limb_bits) <= order) {
			v.push_back(0);
			bit_pos += limb_bits;
		}

		auto bit_off(order - bit_pos);
		limb_type x(value);
		x <<= bit_off;
		v.push_back(x);

		value -= x >> bit_off;
		while (value) {
			value >>= limb_bits - bit_off;
			limb_type x(value);
			v.push_back(x);
			value -= x;
			bit_off = limb_bits;
		}
	}

	template <typename Vector>
	static void shift_left(Vector &v, size_t order)
	{
		v.reserve(v.size() + (order / limb_bits + 1));
		auto s_off(order / limb_bits);
		if (s_off)
			v.insert(v.begin(), s_off, 0);

		auto shift(order % limb_bits);
		if (!shift)
			return;

		limb_type c(0);
		for (auto iter(v.begin() + s_off); iter != v.end(); ++iter) {
			limb_type x(*iter << shift);
			x |= c;
			c = *iter >> (limb_bits - shift);
			*iter = x;
		}
		if (c)
			v.push_back(c);
	}

	template <typename Vector>
	static void assign_pow10(Vector &v, size_t order)
	{
		v.clear();
		v.reserve(order / limb_digits_10 + 1);

		v.push_back(small_power_10[order % limb_digits_10]);
		order -= order % limb_digits_10;
		for(; order; order -= limb_digits_10)
			multiply(v, small_power_10[limb_digits_10]);
	}

	template <typename Vector>
	static int compare(Vector const &l, Vector const &r)
	{
		auto l_sz(l.size() * limb_bits - yesod::clz(l.back()));
		auto r_sz(r.size() * limb_bits - yesod::clz(r.back()));

		if (l_sz > r_sz)
			return 1;
		else if (r_sz > l_sz)
			return -1;

		auto rv(std::mismatch(l.rbegin(), l.rend(), r.rbegin()));
		if (rv.first != l.rend()) {
			if (*rv.first > *rv.second)
				return 1;
			else
				return -1;
		} else
			return 0;
	}

	template <typename Vector>
	static int compare_sum(
		Vector const &l0, Vector const &l1, Vector const &r
	)
	{
		constexpr static limb_type max_value = ~limb_type(0);

		auto l0_sz(l0.size() * limb_bits - yesod::clz(l0.back()));
		auto l1_sz(l1.size() * limb_bits - yesod::clz(l1.back()));
		auto r_sz(r.size() * limb_bits - yesod::clz(r.back()));

		auto l_sz(std::max(l0_sz, l1_sz));

		if (l_sz > r_sz)
			return 1;
		else if (r_sz > (l_sz + 1))
			return -1;

		auto const &l_min(l0_sz <= l1_sz ? l0 : l1);
		auto const &l_max(l0_sz <= l1_sz ? l1 : l0);

		limb_type c(0);
		size_t pos(0);
		int rv(0);
		for (; pos < l_min.size(); ++pos) {
			acc_type acc(l_min[pos]);
			acc += l_max[pos];
			acc += c;
			c = acc >> limb_bits;
			if (limb_type(acc) > r[pos])
				rv = 1;
			else if (limb_type(acc) < r[pos])
				rv = -1;
		}

		for (; pos < l_max.size(); ++pos) {
			auto xl(l_max[pos] + c);
			c = (l_max[pos] == max_value) && c ? 1 : 0;
			if (xl > r[pos])
				rv = 1;
			else if (xl < r[pos])
				rv = -1;
		}

		if (pos < r.size()) {
			auto xr(r[pos]);
			if (xr > c)
				rv = -1;
		} else if (c)
			rv = 1;

		return rv;
	}

	template <typename Vector>
	static void multiply(Vector &l, limb_type r)
	{
		limb_type c(0);
		for (auto &d: l) {
			acc_type acc(r);
			acc *= d;
			acc += c;
			c = acc >> limb_bits;
			d = acc;
		}
		if (c)
			l.push_back(c);
	}

	template <typename Vector>
	static void multiply(Vector &l, Vector const &r)
	{
		Vector m(l.size() + r.size(), 0, l.get_allocator());
		size_t m_pos(0);
		for (auto rd: r) {
			auto n_pos(m_pos);
			limb_type c(0);
			for (auto ld: l) {
				acc_type acc(rd);
				acc *= ld;
				acc += m[n_pos];
				acc += c;
				c = acc >> limb_bits;
				m[n_pos++] = acc;
			}
			m[n_pos] = c;
			++m_pos;
		}

		while (!m.back())
			m.pop_back();

		if (m.empty()) {
			l.clear();
			l.push_back(0);
		} else
			l.swap(m);
	}

	template <typename Vector>
	static void subtract_scaled(
		Vector &l, Vector const &r, size_t order
	)
	{
		auto l_iter(l.begin());
		std::advance(l_iter, order / limb_bits);
		auto shift(order % limb_bits);

		limb_type b(0);
		limb_type sc(0);

		if (shift) {
			for (auto rd: r) {
				limb_type xr(rd << shift);
				xr |= sc;
				sc = rd >> (limb_bits - shift);

				acc_type acc(1);
				acc <<= limb_bits;
				acc += *l_iter;
				acc -= xr;
				acc -= b;

				*l_iter++ = acc;
				b = 1 - (acc >> limb_bits);
			}
		} else {
			for (auto rd: r) {
				acc_type acc(1);
				acc <<= limb_bits;
				acc += *l_iter;
				acc -= rd;
				acc -= b;

				*l_iter++ = acc;
				b = 1 - (acc >> limb_bits);
			}
		}

		if ((sc || b) && (l_iter != l.end())) {
			acc_type acc(1);
			acc <<= limb_bits;
			acc += *l_iter;
			acc -= sc;
			acc -= b;
			*l_iter++ = acc;
			b = 1 - (acc >> limb_bits);
		}

		for (; b && (l_iter != l.end()); ++l_iter) {
			if (*l_iter) {
				*l_iter -= b;
				b = 0;
			} else
				*l_iter = ~*l_iter;
		}

		while (!l.back())
			l.pop_back();

		if (l.empty())
			l.push_back(0);
	}

	template <typename Vector>
	static limb_type divide_near(Vector &num, Vector const &denom)
	{
		auto denom_sz(
			denom.size() * limb_bits - yesod::clz(denom.back())
		);
		auto num_sz(num.size() * limb_bits - yesod::clz(num.back()));

		if (num_sz < denom_sz)
			return 0;

		limb_type rv(0);

		while (num_sz > (denom_sz + 1)) {
			auto order(num_sz - denom_sz);
			subtract_scaled(num, denom, order - 1);
			rv += limb_type(1) << (order - 1);
			num_sz = num.size() * limb_bits
				 - yesod::clz(num.back());
		}

		while (compare(num, denom) >= 0) {
			subtract_scaled(num, denom, 0);
			++rv;
		}

		return rv;
	}
};

}}}
#endif

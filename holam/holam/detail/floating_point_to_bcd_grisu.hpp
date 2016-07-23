/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * Based on algorithm from:
 *
 *     Printing Floating-Point Numbers Quickly and Accurately with Integers
 *     (Florian Loitsch) in Proceedings of the ACM SIGPLAN 2010 Conference
 *     on Programming Language Design and Implementation, PLDI 2010
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F17495057C4D6E59FD92F2FA392D573D)
#define HPP_F17495057C4D6E59FD92F2FA392D573D

#include <holam/detail/floating_point_traits.hpp>
#include <holam/detail/binary_exp10.hpp>

namespace ucpf { namespace holam { namespace detail {

template <typename T>
struct floating_point_to_bcd_grisu {
	typedef fp_value_traits<T> value_traits;
	typedef fp_value_t<T> value_type;
	typedef typename value_traits::mantissa_type mantissa_type;

	static bool apply(
		uint8_t *m_out, std::size_t &m_len, int32_t &exp_out,
		value_type const &val
	) {
		value_type bound_low, bound_high;
		val.boundaries(bound_low, bound_high);

		auto scaled_val(val.normal_form());
		auto exp2(int32_t(value_traits::minimal_target_exp) - int32_t(
			scaled_val.exp + sizeof(mantissa_type) * 8
		));

		auto exp_bound(
			binary_exp10<>::lookup_exp10<mantissa_type>(exp2)
		);

		{
			value_type scale(exp_bound.m, exp_bound.exp2);
			scaled_val = multiply_near(scaled_val, scale);
			bound_low = multiply_near(bound_low, scale);
			bound_high = multiply_near(bound_high, scale);
		}

		--bound_low.m;
		++bound_high.m;

		value_type unsafe(bound_high.m - bound_low.m, bound_high.exp);
		auto unity(value_type(
			mantissa_type(1) << uint32_t(-scaled_val.exp),
			scaled_val.exp
		));

		auto integral(bound_high.m >> -scaled_val.exp);
		auto fractional(bound_high.m & (unity.m - 1)); 

		int32_t exp10(
			((unity.exp + sizeof(mantissa_type) * 8 + 1) * 1233)
			>> 12
		);

		if (integral < small_power_10(exp10))
			--exp10;

		auto exp10_bound_high(exp10 + 1);

		mantissa_type scale(1);
		m_len = 0;
		while (exp10_bound_high > 0) {
			uint32_t digit(divide_near_pow10(integral, exp10));
			--exp10_bound_high;
			auto remainder(
				(integral << (-unity.exp)) + fractional
			);

			if (remainder < unsafe.m) {
				if (!round_weed(
					digit,
					bound_high.m - scaled_val.m,
					unsafe.m, remainder,
					small_power_10(exp10) << (-unity.exp),
					scale
				))
					return false;
				else {
					m_out[
						m_len / 2
					] = m_len & 1
					    ? m_out[m_len / 2] | (digit << 4)
					    : digit;
					++m_len;
					exp_out = exp10_bound_high
						  - exp_bound.exp10;

					return true;
				}
			} else {
				m_out[
					m_len / 2
				] = m_len & 1
				    ? m_out[m_len / 2] | (digit << 4)
				    : digit;
				++m_len;
			}
			--exp10;
		}

		while (true) {
			fractional *= 10;
			scale *= 10;
			unsafe.m *= 10;
			uint32_t digit(fractional >> (-unity.exp));

			fractional &= unity.m - 1;
			--exp10_bound_high;

			if (fractional < unsafe.m) {
				if (!round_weed(
					digit, (
						bound_high.m
						- scaled_val.m
					) * scale,
					unsafe.m, fractional, unity.m, scale
				))
					return false;
				else {
					m_out[
						m_len / 2
					] = m_len & 1
					    ? m_out[m_len / 2] | (digit << 4)
					    : digit;
					++m_len;
					exp_out = exp10_bound_high
						  - exp_bound.exp10;

					return true;
				}
			} else {
				m_out[
					m_len / 2
				] = m_len & 1
				    ? m_out[m_len / 2] | (digit << 4)
				    : digit;
				++m_len;
			}
		}
	}

private:
	static bool round_weed(
		uint32_t &last_digit, mantissa_type upper_range,
		mantissa_type unsafe_range, mantissa_type remainder,
		mantissa_type mult, mantissa_type scale
	) {
		auto range_low(upper_range - scale);
		auto range_high(upper_range + scale);

		while (
			(remainder < range_low)
			&& ((unsafe_range - remainder) >= mult)
			&& (
				((remainder + mult) < range_low)
				|| (
					(range_low - remainder)
					>= (remainder + mult - range_high)
				)
			)
		) {
			last_digit--;
			remainder += mult;
		}

		if (
			(remainder < range_high)
			&& ((unsafe_range - remainder) >= mult)
			&& (
				((remainder + mult) < range_high)
				|| (
					(range_high - remainder)
					> (remainder + mult - range_high)
				)
			)
		)
			return false;

		return (
			(scale << 1) <= remainder)
			&& (remainder <= (unsafe_range - (scale << 2))
		);
	}

	static mantissa_type small_power_10(int32_t exp10);

	static value_type multiply_near(
		value_type const &l, value_type const &r
	);

	static uint32_t divide_near_pow10(
		mantissa_type &num, int32_t exp10
	);
};

template <>
auto floating_point_to_bcd_grisu<float>::small_power_10(
	int32_t exp10
) -> mantissa_type
{
	mantissa_type rv(small_power_5[exp10]);
	return rv << exp10;
}

template <>
auto floating_point_to_bcd_grisu<float>::multiply_near(
	value_type const &l, value_type const &r
) -> value_type
{
	uint64_t acc(l.m);
	acc *= r.m;
	acc += uint64_t(1) << 31u;
	return value_type(
		acc >> 32, l.exp + r.exp + 32
	);
}

template <>
uint32_t floating_point_to_bcd_grisu<float>::divide_near_pow10(
	mantissa_type &num, int32_t exp10
)
{
	auto rv = num / mantissa_type(small_power_5[exp10]);
	rv >>= exp10;
	num -= (small_power_5[exp10] * rv) << exp10;
	return uint32_t(rv);
}

template <>
auto floating_point_to_bcd_grisu<double>::small_power_10(
	int32_t exp10
) -> mantissa_type
{
	mantissa_type rv(small_power_5[exp10]);
	return rv << exp10;
}

template <>
auto floating_point_to_bcd_grisu<double>::multiply_near(
	value_type const &l, value_type const &r
) -> value_type
{
	auto rv(support::multiply(l.m, r.m));
	return value_type(
		rv.round_half(), l.exp + r.exp + 64
	);
}

template <>
uint32_t floating_point_to_bcd_grisu<double>::divide_near_pow10(
	mantissa_type &num, int32_t exp10
)
{
	auto rv(num / small_power_5[exp10]);
	rv >>= exp10;
	num -= (rv * small_power_5[exp10]) << exp10;
	return uint32_t(rv);
}

template <>
auto floating_point_to_bcd_grisu<float128>::small_power_10(
	int32_t exp10
) -> mantissa_type
{
	mantissa_type rv(small_power_5[exp10]);
#if defined(_GLIBCXX_USE_INT128)
	return rv << exp10;
#else
	rv.high = rv.low >> (64 - exp10);
	rv.low <<= exp10;
	return rv;
#endif
}

template <>
auto floating_point_to_bcd_grisu<float128>::multiply_near(
	value_type const &l, value_type const &r
) -> value_type
{
	auto rv(support::multiply(l.m, r.m));
	auto x(rv.round_half());
	return value_type(
		rv.round_half(), l.exp + r.exp + 128
	);
}

template <>
uint32_t floating_point_to_bcd_grisu<float128>::divide_near_pow10(
	mantissa_type &num, int32_t exp10
)
{
	uint32_t rv(uint64_t(num >> exp10) / small_power_5[exp10]);
	if (rv) {
		uint128_t denom(small_power_5[exp10]);
		denom <<= exp10;
		num -= denom * rv;
	}

	return uint32_t(rv);
}

}}}
#endif

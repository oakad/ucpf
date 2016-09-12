/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
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

#if !defined(HPP_D1C098F786496B0FA9C6161C551234D3)
#define HPP_D1C098F786496B0FA9C6161C551234D3

#include <holam/support/bitops.hpp>
#include <holam/detail/binary_pow_10.hpp>

template <typename T>
struct decimal_fp_converter {


	static void to_dec_fp_bigint(
		uint8_t *m_out, int32_t &digits, int32_t &exp_out,
		T const &in_
	) {



	}

/*
	template <typename OutputIterator, typename Alloc>
	static void bigint_convert(OutputIterator &&sink, T v, Alloc const &a)
	{
		typedef std::vector<
			bigint::limb_type,
			typename std::allocator_traits<
				Alloc
			>::template rebind_alloc<bigint::limb_type>
		> bigint_type;

		int extra_shift(0);
		{
			wrapper_type xv(v);
			if (!xv.get_mantissa() && xv.get_exponent())
				extra_shift = 1;
		}
		adapter_type xv(v);
		auto exponent(power_10_estimate(xv.exp));

		bigint_type num(a);
		bigint_type denom(a);
		bigint_type bd_low(a);
		bigint_type bd_high(a);
		if (xv.exp >= 0) {
			bigint::assign_scalar(
				num, xv.m, xv.exp + 1 + extra_shift
			);
			bigint::assign_pow10(denom, exponent);
			bigint::shift_left(denom, 1 +  extra_shift);
			bigint::assign_scalar(bd_low, 1u, xv.exp);
		} else if (exponent >= 0) {
			bigint::assign_scalar(num, xv.m, 1 + extra_shift);
			bigint::assign_pow10(denom, exponent);
			bigint::shift_left(denom, -xv.exp + 1 + extra_shift);
			bigint::assign_scalar(bd_low, 1u);
		} else {
			bigint::assign_scalar(num, xv.m);
			bigint::assign_scalar(
				denom, 1u, -xv.exp + 1 + extra_shift
			);
			bigint::assign_pow10(bd_low, -exponent);
			bigint::multiply(num, bd_low);
			bigint::shift_left(num, 1 + extra_shift);
		}

		bd_high = bd_low;
		if (extra_shift)
			bigint::shift_left(bd_high, extra_shift);

		bool in_range(false);
		if (xv.m & 1u)
			in_range = bigint::compare_sum(
				num, bd_high, denom
			) > 0;
		else
			in_range = bigint::compare_sum(
				num, bd_high, denom
			) >= 0;

		if (in_range)
			++exponent;
		else {
			bigint::multiply(num, 10);
			if (bd_high == bd_low) {
				bigint::multiply(bd_low, 10);
				bd_high = bd_low;
			} else {
				bigint::multiply(bd_low, 10);
				bigint::multiply(bd_high, 10);
			}
		}

		std::array<
			uint32_t, traits_type::decimal_limb_count
		> bv;
		std::fill(bv.begin(), bv.end(), 0);
		int dp(0);

		while (true) {
			int32_t digit(bigint::divide_near(num, denom));
			int bd_test(0);
			if (xv.m & 1u) {
				bd_test |= bigint::compare(
					num, bd_low
				) < 0 ? 1 : 0;
				bd_test |= bigint::compare_sum(
					num, bd_high, denom
				) > 0 ? 2 : 0;
			} else {
				bd_test |= bigint::compare(
					num, bd_low
				) <= 0 ? 1 : 0;
				bd_test |= bigint::compare_sum(
					num, bd_high, denom
				) >= 0 ? 2 : 0;
			}

			switch (bd_test) {
			case 0:
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bigint::multiply(num, 10);
				bigint::multiply(bd_low, 10);
				bigint::multiply(bd_high, 10);
				break;
			case 1:
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<
						OutputIterator
					>(sink), bv, dp, exponent - dp
				);
				return;
			case 2:
				++digit;
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<
						OutputIterator
					>(sink), bv, dp, exponent - dp
				);
				return;
			case 3:
				bd_test = bigint::compare_sum(num, num, denom);
				if ((bd_test > 0) || (!bd_test && (digit & 1)))
					++digit;

				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<
						OutputIterator
					>(sink), bv, dp, exponent - dp
				);
				return;
			};
		}
	}
*/

	static bool round_weed(
		uint32_t &last_digit, mantissa_type upper_range,
		mantissa_type unsafe_range, mantissa_type remainder,
		mantissa_type mult, mantissa_type scale
	)
	{
		auto range_low(upper_range - scale),
		while (
			(remainder < range_low)
			&& ((unsafe_range - remainder) >= mult)
			&& (((remainder + mult) < range_low) || (
				(range_low - remainder)
				>= (remainder + mult - range_low)
			))
		)
		{
			last_digit--;
			remainder += mult;
		}

		auto range_high(upper_range + scale);
		if (
			(remainder < range_high)
			&& ((unsafe_range - remainder) >= mult)
			&& (((remainder + mult) < range_high) || (
				(range_high - remainder)
				> (remainder + mult - range_high)
			))
		)
			return false;

		return ((scale << 1) <= remainder) && (
			remainder <= (unsafe_range - (scale << 2))
		);
	}

	static void to_dec_fp(
		uint8_t *m_out, int32_t &digits, int32_t &exp_out,
		T const &in_
	)
	{
		repr_type in(in_);
		auto bounds(in.boundaries());
		in.normalize();

		auto exp_2(
			repr_type::minimal_target_exp - in.exp
			- repr_type::bit_size
		);
		auto exp_bd(
			binary_pow_10<>::lookup_pow_10<
				typename repr_type::mantissa_type
			>(exp_2)
		);

		repr_type scale(exp_bd.m, exp_bd.pow_2);
		auto in_scaled(in * scale);
		auto scaled_bounds(std::make_pair(
			bounds.first * in_scaled, bounds.second * in_scaled
		));

		--scaled_bounds.first.m;
		++scaled_bounds.second.m;

		auto unsafe(scaled_bounds.second - scaled_bounds.first);
		repr_type unity(
			mantissa_type(1) << (-in_scaled.exp),
			in_scaled.exp
		);
		auto integral(scaled_bounds.second.m >> (-in_scaled.exp));
		auto fractional(scaled_bounds.second.m & (unity.m - 1));

		auto x_exp(small_power_10_estimate(
			repr_type::mantissa_bits + unity.exp + 1
		));

		if (integral < small_power_10[x_exp])
			--x_exp;

		auto exponent(std::make_pair(
			small_power_10[x_exp], x_exp + 1
		));

		mantissa_type m_scale(1);
		while (exponent.second > 0) {
			auto digit(divide_near(integral, exponent.first));
			--exponent.second;
			auto remainder((integral << (-unity.exp)) + fractional);

			if (remainder < unsafe.m) {
				if (to_ascii_decimal_f::round_weed(
					digit, (s_bd.second - s_xv).m,
					unsafe.m, remainder,
					exponent.first << (-unity.exp), scale
				)) {
					m_out[digits++] = digit;
					exp_out = exponent.second
						  - exp_bd.pow_10;
				} else {
					digits = 0;
					to_dec_fp_bigint(
						m_out, digits, exp_out, in_
					);
				}

				return;
			} else
				m_out[digits++] = digit;

			exponent.first /= 10;
		}

		while (true) {
			fractional *= 10;
			scale *= 10;
			unsafe.m *= 10;
			auto digit(fractional >> (-unity.exp));
			fractional &= unity.m - 1;
			--exponent.second;

			if (fractional < unsafe.m) {
				if (round_weed(
					digit, (s_bd.second - s_xv).m * scale,
					unsafe.m, fractional, unity.m, scale
				)) {
					m_out[digits++] = digit;
					exp_out = exponent.second
						  - exp_bd.pow_10;
				} else {
					digits = 0;
					to_dec_fp_bigint(
						m_out, digits, exp_out, in_
					);
				}

				return;
			} else
				m_out[digits++] = digit;
		}
	}
};

}}}
#endif


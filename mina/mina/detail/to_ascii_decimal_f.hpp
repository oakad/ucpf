/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
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

#if !defined(UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_F_20140624T2300)
#define UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_F_20140624T2300

#include <vector>
#include <mina/detail/float.hpp>
#include <mina/detail/bigint.hpp>
#include <mina/detail/binary_pow_10.hpp>
#include <mina/detail/to_ascii_decimal_u.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename OutputIterator, size_t N>
void bcd_to_ascii_f(
	OutputIterator &&sink, std::array<uint32_t, N> const &v,
	int length, int exp
)
{
	constexpr static int negative_cutoff = -6;
	constexpr static int positive_cutoff = 21;
	int c(0);
	auto dot_pos(length + exp);

	if ((dot_pos >= negative_cutoff) && (dot_pos < positive_cutoff)) {
		if (dot_pos < 0) {
			*sink++ = '0';
			*sink++ = '.';
			for(; dot_pos < 0; ++dot_pos)
				*sink++ = '0';

			for (; c < length; ++c) {
				*sink++ = '0' + ((
					v[c >> 3] >> ((7 - (c & 7)) << 2)
				) & 0xf);
			}
		} else if (dot_pos < length) {
			for (; c < dot_pos; ++c) {
				*sink++ = '0' + ((
					v[c >> 3] >> ((7 - (c & 7)) << 2)
				) & 0xf);
			}

			*sink++ = '.';

			for (; c < length; ++c) {
				*sink++ = '0' + ((
					v[c >> 3] >> ((7 - (c & 7)) << 2)
				) & 0xf);
			}
		} else {
			for (; c < length; ++c) {
				*sink++ = '0' + ((
					v[c >> 3] >> ((7 - (c & 7)) << 2)
				) & 0xf);
			}

			for (; c < dot_pos; ++c)
				*sink++ = '0';

			*sink++ = '.';
			*sink++ = '0';
		}
	} else {
		*sink++ = '0' + (
			(v[c >> 3] >> ((7 - (c & 7)) << 2)) & 0xf
		);
		++c;
		if (c < length)
			*sink++ = '.';

		for (; c < length; ++c) {
			*sink++ = 0x30 + (
				(v[c >> 3] >> ((7 - (c & 7)) << 2)) & 0xf
			);
		}
		*sink++ = 'e';

		if (exp >= 0) {
			*sink++ = '+';
			exp += length - 1;
		} else if (dot_pos > 0) {
			*sink++ = '+';
			exp = length + exp - 1;
		} else {
			*sink++ = '-';
			exp = -exp;
			exp -= length - 1;
		}

		to_ascii_decimal_u<uint32_t>(
			std::forward<OutputIterator>(sink), uint32_t(exp)
		);
	}
}

template <typename T>
struct to_ascii_decimal_f {
	typedef typename yesod::fp_adapter_type<T>::type wrapper_type;
	typedef float_t<wrapper_type::bit_size> adapter_type;
	typedef typename wrapper_type::storage_type storage_type;
	typedef to_ascii_decimal_f_traits<T> traits_type;

	static long power_10_estimate(int32_t exp_2)
	{
		constexpr static double inv_log2_10 = 0.30102999566398114;
		return std::lround(std::ceil((
			exp_2
			+ int32_t(wrapper_type::traits_type::mantissa_bits)
			- 1
		) * inv_log2_10 - 1e-10));
	}

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

	static bool round_weed(
		uint32_t &last_digit, storage_type upper_range,
		storage_type unsafe_range, storage_type remainder,
		storage_type mult, storage_type scale
	)
	{
		auto range(std::make_pair(
			upper_range - scale, upper_range + scale
		));

		while (
			(remainder < range.first)
			&& ((unsafe_range - remainder) >= mult)
			&& (((remainder + mult) < range.first) || (
				(range.first - remainder)
				>= (remainder + mult - range.first)
			))
		)
		{
			last_digit--;
			remainder += mult;
		}

		if (
			(remainder < range.second)
			&& ((unsafe_range - remainder) >= mult)
			&& (((remainder + mult) < range.second) || (
				(range.second - remainder)
				> (remainder + mult - range.second)
			))
		)
			return false;

		return ((scale << 1) <= remainder) && (
			remainder <= (unsafe_range - (scale << 2))
		);
		
	}

	template <typename OutputIterator>
	static bool emit_special(OutputIterator &&sink, T v)
	{
		wrapper_type xv(v);
		if (!xv.is_special()) {
			if (!xv.get_storable()) {
				*sink++ = '0';
				*sink++ = '.';
				*sink++ = '0';
				return true;
			}
			return false;
		}

		*sink++ = '1';
		*sink++ = '.';
		*sink++ = '#';
		if (!xv.get_mantissa()) {
			*sink++ = 'i';
			*sink++ = 'n';
			*sink++ = 'f';
		} else {
			auto nv(xv.get_nan());
			if (nv.second)
				*sink++ = 'q';
			else
				*sink++ = 's';

			*sink++ = '(';
			to_ascii_decimal_u<storage_type>(
				std::forward<OutputIterator>(sink), nv.first
			);
			*sink = ')';
		}
		return true;
	}

	template <typename OutputIterator, typename Alloc>
	to_ascii_decimal_f(OutputIterator &&sink, T v, Alloc const &a)
	{
		if (emit_special(std::forward<OutputIterator>(sink), v))
			return;

		adapter_type xv(v);
		auto bd(xv.boundaries());
		xv.normalize();

		auto exp_2(
			int32_t(traits_type::minimal_target_exp)
			- (xv.exp + int32_t(wrapper_type::bit_size))
		);

		auto exp_bd(binary_pow_10::lookup_pow_10<storage_type>(exp_2));
		adapter_type x_scale(exp_bd.m, exp_bd.pow_2);
		auto s_xv(xv * x_scale);
		auto s_bd(std::make_pair(
			bd.first * x_scale, bd.second * x_scale
		));

		--s_bd.first.m;
		++s_bd.second.m;

		auto unsafe(s_bd.second - s_bd.first);
		adapter_type unity(
			storage_type(1) << uint32_t(-s_xv.exp), s_xv.exp
		);
		auto integral(s_bd.second.m >> uint32_t(-s_xv.exp));
		auto fractional(s_bd.second.m & (unity.m - 1));

		auto x_exp(small_power_10_estimate(
			int32_t(wrapper_type::bit_size) + unity.exp + 1
		));

		if (integral < small_power_10[x_exp])
			--x_exp;

		auto exponent(std::make_pair(
			small_power_10[x_exp], x_exp + 1
		));

		std::array<uint32_t, traits_type::decimal_limb_count> bv;
		std::fill(bv.begin(), bv.end(), 0);
		int dp(0);
		storage_type scale(1);
		while (exponent.second > 0) {
			uint32_t digit(divide_near(integral, exponent.first));
			--exponent.second;
			auto remainder((integral << (-unity.exp)) + fractional);

			if (remainder < unsafe.m) {
				if (!to_ascii_decimal_f::round_weed(
					digit, (s_bd.second - s_xv).m,
					unsafe.m, remainder,
					exponent.first << (-unity.exp), scale
				))
					bigint_convert(
						std::forward<
							OutputIterator
						>(sink), v, a
					);
				else {
					bv[dp >> 3] |= digit << (
						(7 - (dp & 7)) << 2
					);
					++dp;
					bcd_to_ascii_f(
						std::forward<
							OutputIterator
						>(sink), bv, dp,
						exponent.second - exp_bd.pow_10
					);
				}
				return;
			} else {
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
			}
			exponent.first /= 10;
		}

		while (true) {
			fractional *= 10;
			scale *= 10;
			unsafe.m *= 10;
			uint32_t digit(fractional >> (-unity.exp));
			fractional &= unity.m - 1;
			--exponent.second;

			if (fractional < unsafe.m) {
				if (!round_weed(
					digit, (s_bd.second - s_xv).m * scale,
					unsafe.m, fractional, unity.m, scale
				))
					bigint_convert(
						std::forward<
							OutputIterator
						>(sink), v, a
					);
				else {
					bv[dp >> 3] |= digit << (
						(7 - (dp & 7)) << 2
					);
					++dp;
					bcd_to_ascii_f(
						std::forward<
							OutputIterator
						>(sink),
						bv, dp,
						exponent.second - exp_bd.pow_10
					);
				}
				return;
			} else {
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
			}
		}
	}
};

template <>
struct to_ascii_decimal_f<yesod::float8> {
	template <typename OutputIterator, typename Alloc>
	to_ascii_decimal_f(
		OutputIterator &&sink, yesod::float8 v, Alloc const &a
	)
	{
		to_ascii_decimal_f<float>::to_ascii_decimal_f(
			std::forward<OutputIterator>(sink), float(v), a
		);
	}
};

template <>
struct to_ascii_decimal_f<yesod::float16> {
	template <typename OutputIterator, typename Alloc>
	to_ascii_decimal_f(
		OutputIterator &&sink, yesod::float16 v, Alloc const &a
	)
	{
		to_ascii_decimal_f<float>::to_ascii_decimal_f(
			std::forward<OutputIterator>(sink), float(v), a
		);
	}
};

}}}
#endif

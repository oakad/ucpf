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
#include <yesod/float.hpp>
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

template <unsigned int N>
struct float_t {
	typedef yesod::float_t<N> wrapper_type;
	typedef typename wrapper_type::storage_type mantissa_type;

	float_t(mantissa_type m_, int32_t exp_)
	: m(m_), exp(exp_)
	{}

	float_t(float_t const &other)
	: m(other.m), exp(other.exp)
	{}

	float_t(typename wrapper_type::machine_type v)
	{
		constexpr static auto exponent_bias(
			wrapper_type::traits_type::mantissa_bits - 1
		);

		wrapper_type xv(v);
		m = xv.get_mantissa();
		exp = xv.get_exponent_value() - exponent_bias;
		if (xv.get_exponent())
			m += mantissa_type(1) << exponent_bias;
	}

	float_t operator-(float_t other) const
	{
		return float_t(m - other.m, exp);
	}

	auto operator*(float_t other) const -> float_t;

	std::pair<float_t, float_t> boundaries() const
	{
		constexpr static auto exponent_bias
		= wrapper_type::traits_type::mantissa_bits
		  + wrapper_type::traits_type::exponent_bias;

		float_t high((m << 1) + 1, exp - 1);
		high.normalize();
		float_t low((m << 1) - 1, exp - 1);
		if (!m && (exp != exponent_bias)) {
			low.m = (m << 2) - 1;
			low.exp = exp - 2;
		}
		low.m <<= low.exp - high.exp;
		low.exp = high.exp;
		return std::make_pair(low, high);
	}

	void normalize();

	mantissa_type m;
	int32_t exp;
};

template <>
auto float_t<64>::operator*(float_t other) const -> float_t
{
	uint128_t acc(m);
	acc *= other.m;
	acc += 1ull << 63; /* rounding */
	return float_t(
		uint64_t(acc >> 64), exp + other.exp + 64
	);
}

template <>
void float_t<64>::normalize()
{
	auto l(__builtin_clzll(m));
	m <<= l;
	exp -= l;
}

template <typename T>
struct to_ascii_decimal_f_s;

template <typename T>
struct to_ascii_decimal_f_traits;;

template <>
struct to_ascii_decimal_f_traits<double> {
	constexpr static int minimal_target_exp = -60;
	constexpr static int decimal_limb_count = 3;
	constexpr static int mantissa_bits = 53;
	constexpr static double inv_log2_10 = 0.30102999566398114;

	static long pow_10_estimate(int32_t exp_2)
	{
		return std::lround(std::ceil(
			(exp_2 + mantissa_bits - 1) * inv_log2_10 - 1e-10
		));
	}
};

template <typename T>
struct to_ascii_decimal_f {
	typedef typename yesod::fp_adapter_type<T>::type wrapper_type;
	typedef float_t<wrapper_type::bit_size> adapter_type;
	typedef typename wrapper_type::storage_type storage_type;
	typedef to_ascii_decimal_f_traits<T> traits_type;

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
		auto exponent(traits_type::pow_10_estimate(xv.exp));

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
			bigint::assign_scalar(bd_low, 1, xv.exp);
		} else if (exponent >= 0) {
			bigint::assign_scalar(num, xv.m, 1 + extra_shift);
			bigint::assign_pow10(denom, exponent);
			bigint::shift_left(denom, -xv.exp + 1 + extra_shift);
			bigint::assign_scalar(bd_low, 1);
		} else {
			bigint::assign_scalar(num, xv.m);
			bigint::assign_scalar(
				denom, 1, -xv.exp + 1 + extra_shift
			);
			bigint::assign_pow10(bd_low, -exponent);
			bigint::multiply(num, bd_low);
			bigint::shift_left(num, 1 + extra_shift);
		}

		bd_high = bd_low;
		if (extra_shift)
			bigint::shift_left(bd_high, extra_shift);

		bool in_range(false);
		if (xv.m & 1)
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
			bigint::multiply_scalar(num, 10u);
			if (bd_high == bd_low) {
				bigint::multiply_scalar(bd_low, 10u);
				bd_high = bd_low;
			} else {
				bigint::multiply_scalar(bd_low, 10u);
				bigint::multiply_scalar(bd_high, 10u);
			}
		}

		bigint_type q(a);
		bigint_type r(a);
		std::array<
			uint32_t, traits_type::decimal_limb_count
		> bv;
		std::fill(bv.begin(), bv.end(), 0);
		int dp(0);
		while (true) {
			bigint::divide(q, r, num, denom);
			int32_t digit(q[0]);
			num.swap(r);

			int bd_test(0);
			if (xv.m & 1) {
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
				bigint::multiply_scalar(num, 10u);
				bigint::multiply_scalar(bd_low, 10u);
				bigint::multiply_scalar(bd_high, 10u);
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
			remainder <= (unsafe_range - (mult << 2))
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

		auto exp_bd(binary_pow_10<T>::lookup_exp_10(
			traits_type::minimal_target_exp
			- (xv.exp + wrapper_type::bit_size)
		));

		adapter_type x_scale(exp_bd.m, exp_bd.exp_2);
		auto s_xv(xv * x_scale);
		auto s_bd(std::make_pair(
			bd.first * x_scale, bd.second * x_scale
		));

		--s_bd.first.m;
		++s_bd.second.m;

		auto unsafe(s_bd.second - s_bd.first);
		adapter_type unity(storage_type(1) << -s_xv.exp, s_xv.exp);
		auto integral(s_bd.second.m >> -s_xv.exp);
		auto fractional(s_bd.second.m & (unity.m - 1));

		auto x_exp(small_power_10_estimate(
			wrapper_type::bit_size + unity.exp + 1
		));
		if (integral < small_power_10[x_exp])
			--x_exp;

		auto exponent(std::make_pair(small_power_10[x_exp], x_exp + 1));


		std::array<uint32_t, traits_type::decimal_limb_count> bv;
		std::fill(bv.begin(), bv.end(), 0);
		int dp(0);
		storage_type scale(1);
		while (exponent.second > 0) {
			uint32_t digit(integral / exponent.first);
			integral %= exponent.first;
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
						exponent.second - exp_bd.exp_10
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
						exponent.second - exp_bd.exp_10
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

}}}
#endif

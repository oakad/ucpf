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

#include <yesod/float.hpp>
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

	if ((exp >= negative_cutoff) && (exp < positive_cutoff)) {
		int dot_pos(length + exp);

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
		if (exp >= 0)
			*sink++ = '+';
		else {
			exp = -exp;
			*sink++ = '-';
		}
		exp += length - 1;
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
	unsigned __int128 acc(m);
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
struct to_ascii_decimal_f;

template <typename T>
struct to_ascii_decimal_f_s;

template <>
struct to_ascii_decimal_f<double> {
	static std::pair<uint32_t, int> pow_10_estimate(uint32_t v, int n_bits)
	{
		constexpr std::array<uint32_t, 11> pow_10 = {{
			0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
			100000000, 1000000000
		}};

		auto exp(((n_bits + 1) * 1233 >> 12) + 1);

		if (v < pow_10[exp])
			--exp;

		return std::make_pair(pow_10[exp], exp);
	}

	static bool round_weed(
		uint32_t &last_digit,
		uint64_t upper_range, uint64_t unsafe_range,
		uint64_t remainder, uint64_t mult, uint64_t scale
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
	to_ascii_decimal_f(OutputIterator &&sink, double v)
	{
		constexpr static int minimal_target_exp = -60;
		constexpr static int mantissa_size = 64;

		float_t<64> xv(v);
		auto bd(xv.boundaries());
		xv.normalize();

		auto exp_bd(binary_pow_10::lookup_exp_10<double>(
			minimal_target_exp - (xv.exp + mantissa_size)
		));
		float_t<64> x_scale(exp_bd.m, exp_bd.exp_2);
		auto s_xv(xv * x_scale);
		auto s_bd(std::make_pair(
			bd.first * x_scale, bd.second * x_scale
		));

		--s_bd.first.m;
		++s_bd.second.m;

		auto unsafe(s_bd.second - s_bd.first);
		float_t<64> unity(uint64_t(1) << -s_xv.exp, s_xv.exp);
		auto integral(s_bd.second.m >> -s_xv.exp);
		auto fractional(s_bd.second.m & (unity.m - 1));
		auto exponent(
			pow_10_estimate(integral, mantissa_size + unity.exp)
		);

		std::array<uint32_t, 3> bv{0, 0, 0};
		int dp(0);
		uint64_t scale(1);
		while (exponent.second > 0) {
			uint32_t digit(integral / exponent.first);
			integral %= exponent.first;
			--exponent.second;

			auto remainder((integral << (-unity.exp)) + fractional);

			if (remainder < unsafe.m) {
				if (!round_weed(
					digit, (s_bd.second - s_xv).m,
					unsafe.m, remainder,
					exponent.first << (-unity.exp), scale
				))
					to_ascii_decimal_f_s<double>(
						std::forward<
							OutputIterator
						>(sink), v
					);

				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<OutputIterator>(sink), bv,
					dp, exponent.second - exp_bd.exp_10
				);
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
					to_ascii_decimal_f_s<double>(
						std::forward<
							OutputIterator
						>(sink), v
					);

				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<OutputIterator>(sink), bv,
					dp, exponent.second - exp_bd.exp_10
				);
				return;
			} else {
				bv[dp >> 3] |= digit << ((7 - (dp & 7)) << 2);
				++dp;
			}
		}
	}
};

template <>
struct to_ascii_decimal_f_s<double> {
	template <typename OutputIterator>
	to_ascii_decimal_f_s(OutputIterator &&sink, double v)
	{
		
	}
};

}}}
#endif

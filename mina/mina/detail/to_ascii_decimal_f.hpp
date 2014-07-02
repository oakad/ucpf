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

	printf("xxx %d\n", exp);
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

template <typename T>
struct to_ascii_decimal_f;

template <>
struct to_ascii_decimal_f<double> {
	struct fp_type {
		uint64_t m;
		int exp;

		fp_type(uint64_t m_, int exp_)
		: m(m_), exp(exp_)
		{}

		fp_type(fp_type const &other)
		: m(other.m), exp(other.exp)
		{}

		fp_type(double v)
		{
			constexpr static int e_shift = 52;
			constexpr static int e_offset = 1075;
			union {
				double f;
				uint64_t i;
			} xv;
			xv.f = v;
			m = xv.i & ((1ull << (e_shift + 1)) - 1);
			exp = xv.i >> e_shift;
			exp -= e_offset;
			printf("xx %zx, %d\n", m, exp);
		}

		fp_type operator-(fp_type other) const
		{
			return fp_type(m - other.m, exp);
		}

		fp_type operator*(fp_type other) const
		{
			unsigned __int128 acc(m);
			acc *= other.m;
			acc += 0x8000000000000000ull; /* rounding */
			return fp_type(
				uint64_t(acc >> 64), exp + other.exp + 64
			);
		}

		std::pair<fp_type, fp_type> boundaries() const
		{
			constexpr static int exponent_bias = 1074;
			fp_type high((m << 1) + 1, exp - 1);
			high.normalize();
			fp_type low((m << 1) - 1, exp - 1);
			if (!m && (exp != exponent_bias)) {
				low.m = (m << 2) - 1;
				low.exp = exp - 2;
			}
			low.m <<= low.exp - high.exp;
			low.exp = high.exp;
			printf("bd (%zx, %d) - (%zx, %d)\n", low.m, low.exp, high.m, high.exp);
			return std::make_pair(low, high);
		}

		void normalize()
		{
			auto l(__builtin_clzll(m));
			m <<= l;
			exp -= l;
		}
	};

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
		uint64_t x_rem, uint64_t mult, uint64_t scale
	)
	{
		auto range(std::make_pair(
			upper_range - scale, upper_range + scale
		));

		while (
			(x_rem < range.first)
			&& ((unsafe_range - x_rem) >= mult)
			&& (((x_rem + mult) < range.first) || (
				(range.first - x_rem)
				>= (x_rem + mult - range.first)
			))
		)
		{
			last_digit--;
			x_rem += mult;
		}

		if (
			(x_rem < range.second)
			&& ((unsafe_range - x_rem) >= mult)
			&& (((x_rem + mult) < range.second) || (
				(range.second - x_rem)
				> (x_rem + mult - range.second)
			))
		)
			return false;

		return ((scale << 1) <= x_rem) && (
			x_rem <= (unsafe_range - (mult << 2))
		);
	}

	template <typename OutputIterator>
	to_ascii_decimal_f(OutputIterator &&sink, double v)
	{
		constexpr static int minimal_target_exp = -60;
		constexpr static int mantissa_size = 64;

		fp_type xv(v);
		auto bd(xv.boundaries());
		xv.normalize();
		printf("in %f, m: %zx, e: %d\n", v, xv.m, xv.exp);

		auto exp_bd(binary_pow_10::lookup_exp_10<double>(
			minimal_target_exp - (xv.exp + mantissa_size)
		));
		fp_type x_scale(exp_bd.m, exp_bd.exp_2);
		auto s_xv(xv * x_scale);
		auto s_bd(std::make_pair(bd.first * x_scale, bd.second * x_scale));

		printf("s_xv %zx, %d\n", s_xv.m, s_xv.exp);
		printf("s_bd min %zx, %d\n", s_bd.first.m, s_bd.first.exp);
		printf("s_bd max %zx, %d\n", s_bd.second.m, s_bd.second.exp);
		printf("scale %zx, %d, %d\n", exp_bd.m, exp_bd.exp_10, exp_bd.exp_2);

		auto s_bd_outer(s_bd);
		--s_bd_outer.first.m;
		++s_bd_outer.second.m;
		auto unsafe(s_bd_outer.second - s_bd_outer.first);
		fp_type x_one(uint64_t(1) << -s_xv.exp, s_xv.exp);
		auto x_int(s_bd_outer.second.m >> -s_xv.exp);
		auto x_frac(s_bd_outer.second.m & (x_one.m - 1));
		auto x_exp(pow_10_estimate(x_int, mantissa_size + x_one.exp));

		std::array<uint32_t, 3> bv{0, 0, 0};
		int dp(0);
		uint64_t scale(1);
		while (x_exp.second > 0) {
			uint32_t x_dig(x_int / x_exp.first);
			x_int %= x_exp.first;
			--x_exp.second;

			auto x_rem((x_int << (-x_one.exp)) + x_frac);

			if (x_rem < unsafe.m) {
				printf("e_int\n");
				if (!round_weed(
					x_dig, (s_bd_outer.second - s_xv).m,
					unsafe.m, x_rem,
					x_exp.first << (-x_one.exp), scale
				)) {
					printf("no match\n");
				}
				bv[dp >> 3] |= x_dig << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<OutputIterator>(sink), bv,
					dp, x_exp.second - exp_bd.exp_10
				);
				return;
			} else {
				bv[dp >> 3] |= x_dig << ((7 - (dp & 7)) << 2);
				++dp;
			}
			x_exp.first /= 10;
		}

		while (true) {
			x_frac *= 10;
			scale *= 10;
			unsafe.m *= 10;
			uint32_t x_dig(x_frac >> (-x_one.exp));
			x_frac &= x_one.m - 1;
			--x_exp.second;
			if (x_frac < unsafe.m) {
				printf("e_frac\n");
				if (!round_weed(
					x_dig,
					(s_bd_outer.second - s_xv).m * scale,
					unsafe.m, x_frac, x_one.m, scale
				)) {
					printf("no match\n");
				}
				bv[dp >> 3] |= x_dig << ((7 - (dp & 7)) << 2);
				++dp;
				bcd_to_ascii_f(
					std::forward<OutputIterator>(sink), bv,
					dp, x_exp.second - exp_bd.exp_10
				);
				return;
			} else {
				bv[dp >> 3] |= x_dig << ((7 - (dp & 7)) << 2);
				++dp;
			}
		}
	}
};

}}}
#endif

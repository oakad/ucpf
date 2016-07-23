/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_B25E3954ACC63CB019CE8FA6DE2DAAA6)
#define HPP_B25E3954ACC63CB019CE8FA6DE2DAAA6

#include <holam/support/float128.hpp>
#include <holam/support/bitops.hpp>
#include <cstdio>

#pragma push_macro("NAN")
#undef NAN

namespace ucpf { namespace holam { namespace detail {

template <typename T>
struct fp_value_traits;

template <>
struct fp_value_traits<float> {
	typedef uint32_t mantissa_type;
	constexpr static std::size_t mantissa_bits = 23;
	constexpr static std::size_t exponent_bits = 8;
	constexpr static std::size_t bcd_storage_size = 5;
	constexpr static int32_t exponent_bias = 127;
	constexpr static int minimal_target_exp = -28;
};

template <>
struct fp_value_traits<double> {
	typedef uint64_t mantissa_type;
	constexpr static std::size_t mantissa_bits = 52;
	constexpr static std::size_t exponent_bits = 11;
	constexpr static std::size_t bcd_storage_size = 9;
	constexpr static int32_t exponent_bias = 1023;
	constexpr static int minimal_target_exp = -60;
};

template <>
struct fp_value_traits<ucpf::float128> {
	typedef ucpf::uint128_t mantissa_type;
	constexpr static std::size_t mantissa_bits = 112;
	constexpr static std::size_t exponent_bits = 15;
	constexpr static std::size_t bcd_storage_size = 18;
	constexpr static int32_t exponent_bias = 16383;
	constexpr static int minimal_target_exp = -124;
};

constexpr uint64_t small_power_5[] = {
	1, 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125,
	9765625, 48828125, 244140625, 1220703125, 6103515625ull,
	30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull,
	19073486328125ull, 95367431640625ull, 476837158203125ull,
	2384185791015625ull, 11920928955078125ull, 59604644775390625ull,
	298023223876953125ull, 1490116119384765625ull, 7450580596923828125ull

};
constexpr std::size_t max_small_power_5 = 27;

template <std::size_t N>
struct bigint_calc_traits;

template <>
struct bigint_calc_traits<64> {
	typedef uint64_t limb_type;
	typedef uint128_t acc_type;
	constexpr static std::size_t limb_bits = 64;
	constexpr static std::size_t base5_digits = 27;

	static uint32_t assign_pow5(
		limb_type *val, uint32_t exp5
	)
	{
		if (exp5 <= max_small_power_5) {
			val[0] = small_power_5[exp5];
			return exp5;
		} else {
			val[0] = small_power_5[max_small_power_5];
			return max_small_power_5;
		}
	}
};

template <>
struct bigint_calc_traits<32> {
	typedef uint32_t limb_type;
	typedef uint64_t acc_type;
	constexpr static std::size_t limb_bits = 32;
	constexpr static std::size_t base5_digits = 13;

	static uint32_t assign_pow5(
		limb_type *val, uint32_t exp5
	)
	{
		if (exp5 <= base5_digits) {
			val[0] = static_cast<limb_type>(
				small_power_5[exp5]
			);
			return exp5;
		} else if (exp5 <= max_small_power_5) {
			val[0] = static_cast<limb_type>(
				small_power_5[exp5]
			);
			val[1] = small_power_5[exp5] >> 32;
			return exp5;
		} else {
			val[0] = static_cast<limb_type>(
				small_power_5[max_small_power_5]
			);
			val[1] = small_power_5[max_small_power_5] >> 32;
			return max_small_power_5;
		}

	}
};

template <typename T>
struct fp_value_t {
	typedef fp_value_traits<T> traits_type;
	typedef typename traits_type::mantissa_type mantissa_type;

	fp_value_t()
	: m(0), exp(0), flags(0)
	{}

	fp_value_t(T const &val)
	: flags(0)
	{
		__builtin_memcpy(&m, &val, sizeof(m));
		if (std::signbit(val))
			flags |= NEGATIVE;

		exp = static_cast<int32_t>((
			m >> traits_type::mantissa_bits
		) & (
			(mantissa_type(1) << traits_type::exponent_bits) - 1
		));
		m &= (mantissa_type(1) << traits_type::mantissa_bits) - 1;

		if ((exp + 1) >> traits_type::exponent_bits) {
			flags |= m ? NAN : INF;
			return;

		}

		if (exp) {
			if (!m)
				flags |= NORMAL_POW2;
			m += mantissa_type(1) << traits_type::mantissa_bits;
		}

		exp = (exp ? exp : 1)
		      - traits_type::exponent_bias
		      - traits_type::mantissa_bits;
	}

	fp_value_t(mantissa_type m_, int32_t exp_)
	: m(m_), exp(exp_), flags(0)
	{}

	bool is_normal_pow2() const
	{
		return flags & NORMAL_POW2 ? true : false;
	}

	bool is_special() const
	{
		return flags & (NAN | INF) ? true : false;
	}

	fp_value_t normal_form() const
	{
		fp_value_t rv(*this);
		auto shift(support::clz(rv.m));
		rv.m <<= shift;
		rv.exp -= shift;
		return rv;
	}

	void boundaries(fp_value_t &low, fp_value_t &high) const
	{
		if (flags & NORMAL_POW2) {
			low.m = (m << 2) - 1;
			low.exp = exp - 2;
		} else {
			low.m = (m << 1) - 1;
			low.exp = exp - 1;
		}

		high.m = (m << 1) + 1;
		high.exp = exp - 1;
		auto shift(support::clz(high.m));
		high.m <<= shift;
		high.exp -= shift;

		low.m <<= low.exp - high.exp;
		low.exp = high.exp;
	}

	enum FLAG : uint32_t {
		NEGATIVE = 1,
		NORMAL_POW2 = 2,
		NAN = 4,
		INF = 8
	};

	mantissa_type m;
	int32_t exp;
	uint32_t flags;
};
}}}

#pragma pop_macro("NAN")
#endif

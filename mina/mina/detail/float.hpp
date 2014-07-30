/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_FLOAT_20140723T2300)
#define UCPF_MINA_DETAIL_FLOAT_20140723T2300

#include <yesod/float.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct to_ascii_decimal_f_traits;;

template <>
struct to_ascii_decimal_f_traits<float> {
	constexpr static int minimal_target_exp = -28;
	constexpr static int decimal_limb_count = 2;
};

template <>
struct to_ascii_decimal_f_traits<double> {
	constexpr static int minimal_target_exp = -60;
	constexpr static int decimal_limb_count = 3;
};

template <>
struct to_ascii_decimal_f_traits<yesod::float128> {
	constexpr static int minimal_target_exp = -124;
	constexpr static int decimal_limb_count = 5;
};

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

	float_t operator*(float_t other) const;

	float_t &operator*=(float_t other)
	{
		auto x(*this * other);
		m = x.m;
		exp = x.exp;
	}

	std::pair<float_t, float_t> boundaries() const
	{
		constexpr static auto exponent_bias
		= wrapper_type::traits_type::mantissa_bits
		  + wrapper_type::traits_type::exponent_bias;

		float_t high((m << 1u) + 1, exp - 1);
		high.normalize();
		float_t low((m << 1u) - 1, exp - 1);
		if (!m && (exp != exponent_bias)) {
			low.m = (m << 2u) - 1;
			low.exp = exp - 2;
		}
		low.m <<= low.exp - high.exp;
		low.exp = high.exp;
		return std::make_pair(low, high);
	}

	void normalize()
	{
		auto l(yesod::clz(m));
		m <<= l;
		exp -= l;
	}

	mantissa_type m;
	int32_t exp;
};

template <>
auto float_t<32>::operator*(float_t other) const -> float_t
{
	uint64_t acc(m);
	acc *= other.m;
	acc += uint64_t(1) << 31u; /* rounding */
	return float_t(
		uint32_t(acc >> 32), exp + other.exp + 32
	);
}

template <>
auto float_t<64>::operator*(float_t other) const -> float_t
{
	auto rv(yesod::detail::multiply(m, other.m));
	rv.second += rv.first >> 63u; /* rounding */
	return float_t(
		rv.second, exp + other.exp + 64
	);
}

template <>
auto float_t<128>::operator*(float_t other) const -> float_t
{
	auto rv(yesod::detail::multiply(m, other.m));
	rv.second += rv.first >> 127u; /* rounding */
	return float_t(
		rv.second, exp + other.exp + 128
	);
}

}}}
#endif
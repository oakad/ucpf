/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_FLOAT128_20140713T2300)
#define UCPF_YESOD_DETAIL_FLOAT128_20140713T2300

namespace ucpf { namespace yesod { namespace detail {


}}}

#if defined(_GLIBCXX_USE_FLOAT128)
#include <quadmath.h>

namespace ucpf { namespace yesod {

typedef __float128 float128;

namespace detail {

union float128_const_t {
	uint8_t s[16];
	float128 v;
};

constexpr static float128_const_t float128_qnan = { .s = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x7f
}};

constexpr static float128_const_t float128_snan = { .s = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xff, 0x7f
}};

}
}}

namespace std {

inline constexpr ucpf::yesod::float128 abs(ucpf::yesod::float128 x)
{
	return __builtin_fabsq(x);
}

inline constexpr bool signbit(ucpf::yesod::float128 x)
{
	return __builtin_signbit(x);
}

template <>
struct numeric_limits<ucpf::yesod::float128> {
	constexpr static bool is_specialized = true;

	constexpr static ucpf::yesod::float128 min() noexcept
	{
		return FLT128_MIN;
	}

	constexpr static ucpf::yesod::float128 max() noexcept
	{
		return FLT128_MAX;
	}

	constexpr static ucpf::yesod::float128 lowest() noexcept
	{
		return -FLT128_MAX;
	}

	constexpr static int digits = FLT128_MANT_DIG;
	constexpr static int digits10 = FLT128_DIG;
	constexpr static int max_digits10 = 36;

	constexpr static bool is_signed = true;
	constexpr static bool is_integer = false;
	constexpr static bool is_exact = false;
	constexpr static int radix = 2;

	constexpr static ucpf::yesod::float128 epsilon() noexcept
	{
		return FLT128_EPSILON;
	}

	constexpr static ucpf::yesod::float128 round_error() noexcept
	{
		return 0.5Q;
	}

	constexpr static int min_exponent = FLT128_MIN_EXP;
	constexpr static int min_exponent10 = FLT128_MIN_10_EXP;
	constexpr static int max_exponent = FLT128_MAX_EXP;
	constexpr static int max_exponent10 = FLT128_MAX_10_EXP;

	constexpr static bool has_infinity = true;
	constexpr static bool has_quiet_NaN = true;
	constexpr static bool has_signaling_NaN = true;
	constexpr static float_denorm_style has_denorm = denorm_present;
	constexpr static bool has_denorm_loss = false;

	constexpr static ucpf::yesod::float128 infinity() noexcept
	{
		return __builtin_huge_valq();
	}

	constexpr static ucpf::yesod::float128 quiet_NaN() noexcept
	{
		return ucpf::yesod::detail::float128_qnan.v;
	}

	constexpr static ucpf::yesod::float128 signaling_NaN() noexcept
	{
		return ucpf::yesod::detail::float128_snan.v;
	}

	constexpr static ucpf::yesod::float128 denorm_min() noexcept
	{
		return FLT128_DENORM_MIN;
	}

	constexpr static bool is_iec559 = true;
	constexpr static bool is_bounded = true;
	constexpr static bool is_modulo = false;

	constexpr static bool traps = false;
	constexpr static bool tinyness_before = false;
	constexpr static float_round_style round_style = round_to_nearest;
};

}

#else

namespace ucpf { namespace yesod {

struct [[gnu::packed]] float128 {
	uint64_t low;
	uint64_t high;
};

}}

namespace std {

inline constexpr ucpf::yesod::float128 abs(ucpf::yesod::float128 x)
{
	return ucpf::yesod::float128{x.low, x.high & ~(uint64_t(1) << 63)};
}

inline constexpr bool signbit(ucpf::yesod::float128 x)
{
	return (x.high >> 63) ? true : false;
}

}
#endif

#endif

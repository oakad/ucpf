/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_1F38C1F9B81C12198C60CFCF07B7706D)
#define HPP_1F38C1F9B81C12198C60CFCF07B7706D

#include <limits>
#include <holam/support/int128.hpp>

#if defined(_GLIBCXX_USE_FLOAT128)
#include <quadmath.h>
#endif

namespace ucpf {

#if defined(_GLIBCXX_USE_FLOAT128)

typedef __float128 float128;

#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

struct [[gnu::packed]] soft_float128_t {
	soft_float128_t() = default;

#if defined(_GLIBCXX_USE_FLOAT128)
	constexpr soft_float128_t(float128 v)
	: low(*reinterpret_cast<uint64_t *>(&v)),
	  high(*(reinterpret_cast<uint64_t *>(&v) + 1))
	{}

	constexpr operator float128() const
	{
		return *reinterpret_cast<float128 const *>(&low);
	}
#endif

	constexpr soft_float128_t(uint64_t low_, uint64_t high_)
	: low(low_), high(high_)
	{}

	uint64_t low;
	uint64_t high;
};

#else

struct [[gnu::packed]] soft_float128_t {
	soft_float128_t() = default;

#if defined(_GLIBCXX_USE_FLOAT128)
	constexpr soft_float128_t(float128 v)
	: high(*reinterpret_cast<uint64_t *>(&v)),
	  low(*(reinterpret_cast<uint64_t *>(&v) + 1))
	{}

	constexpr operator float128() const
	{
		return *reinterpret_cast<float128 const *>(&high);
	}
#endif

	constexpr soft_float128_t(uint64_t low_, uint64_t high_)
	: high(high_), low(low_)
	{}

	uint64_t high;
	uint64_t low;
};

#endif

namespace holam { namespace detail {

constexpr static soft_float128_t float128_qnan = soft_float128_t(
	0, 0x7fff800000000000ull
);

constexpr static soft_float128_t float128_snan = soft_float128_t(
	0, 0x7fff400000000000ull
);

}}
}

#if defined(_GLIBCXX_USE_FLOAT128)

namespace std {

inline constexpr ucpf::float128 abs(ucpf::float128 x)
{
	return __builtin_fabsq(x);
}

inline constexpr bool signbit(ucpf::float128 x)
{
	return __builtin_signbit(x);
}

template <>
struct numeric_limits<ucpf::float128> {
	constexpr static bool is_specialized = true;

	constexpr static ucpf::float128 min() noexcept
	{
		return FLT128_MIN;
	}

	constexpr static ucpf::float128 max() noexcept
	{
		return FLT128_MAX;
	}

	constexpr static ucpf::float128 lowest() noexcept
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

	constexpr static ucpf::float128 epsilon() noexcept
	{
		return FLT128_EPSILON;
	}

	constexpr static ucpf::float128 round_error() noexcept
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

	constexpr static ucpf::float128 infinity() noexcept
	{
		return __builtin_infq();
	}

	constexpr static ucpf::float128 quiet_NaN() noexcept
	{
		return ucpf::holam::detail::float128_qnan;
	}

	constexpr static ucpf::float128 signaling_NaN() noexcept
	{
		return ucpf::holam::detail::float128_snan;
	}

	constexpr static ucpf::float128 denorm_min() noexcept
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

namespace ucpf {

typedef soft_float128 float128;

}

namespace std {

static inline constexpr ucpf::float128 abs(ucpf::float128 x)
{
	constexpr static uint64_t sign_mask = 0x7fffffffffffffffull;
	return ucpf::float128{
		.low = x.low,
		.high = x.high & sign_mask
	};
}

inline constexpr bool signbit(ucpf::float128 x)
{
	return (x.high >> 63) ? true : false;
}

}
#endif

#endif

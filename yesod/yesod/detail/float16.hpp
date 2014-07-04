/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_FLOAT16_20140703T2300)
#define UCPF_YESOD_DETAIL_FLOAT16_20140703T2300

#include <limits>
#include <cstdint>

namespace ucpf { namespace yesod {

struct float16 {
	constexpr static uint16_t sign_mask = 0x8000;
	constexpr static uint16_t exp_mask = 0x7c00;
	constexpr static uint16_t mantissa_mask = 0x3ff;

	uint16_t v;
};

}}

namespace std {

template <>
struct numeric_limits<ucpf::yesod::float16> {
	constexpr static bool is_specialized = true;

	constexpr static ucpf::yesod::float16 min() noexcept
	{
		return ucpf::yesod::float16{0x0400};
	}

	constexpr static ucpf::yesod::float16 max() noexcept
	{
		return ucpf::yesod::float16{0x7bff};
	}

	constexpr static ucpf::yesod::float16 lowest() noexcept
	{
		return ucpf::yesod::float16{0xfbff};
	}

	constexpr static int digits = 12;
	constexpr static int digits10 = 3;
	constexpr static int max_digits10 = 5;

	constexpr static bool is_signed = true;
	constexpr static bool is_integer = false;
	constexpr static bool is_exact = false;
	constexpr static int radix = 2;

	constexpr static ucpf::yesod::float16 epsilon() noexcept
	{
		return ucpf::yesod::float16{0x1000};
	}

	constexpr static ucpf::yesod::float16 round_error() noexcept
	{
		return ucpf::yesod::float16{0x3800};
	}

	constexpr static int min_exponent = -13;
	constexpr static int min_exponent10 = 3;
	constexpr static int max_exponent = 16;
	constexpr static int max_exponent10 = 4;

	constexpr static bool has_infinity = true;
	constexpr static bool has_quiet_NaN = true;
	constexpr static bool has_signaling_NaN = true;
	constexpr static float_denorm_style has_denorm = denorm_present;
	constexpr static bool has_denorm_loss = false;

	constexpr static ucpf::yesod::float16 infinity() noexcept
	{
		return ucpf::yesod::float16{0x7c00};
	}

	constexpr static ucpf::yesod::float16 quiet_NaN() noexcept
	{
		return ucpf::yesod::float16{0x7e00};
	}

	constexpr static ucpf::yesod::float16 signaling_NaN() noexcept
	{
		return ucpf::yesod::float16{0x7d00};
	}

	constexpr static ucpf::yesod::float16 denorm_min() noexcept
	{
		return ucpf::yesod::float16{0x0001};
	}

	constexpr static bool is_iec559 = true;
	constexpr static bool is_bounded = true;
	constexpr static bool is_modulo = false;

	constexpr static bool traps = false;
	constexpr static bool tinyness_before = false;
	constexpr static float_round_style round_style = round_to_nearest;
};

}

#endif

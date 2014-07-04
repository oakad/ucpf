/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_FLOAT8_20140703T2300)
#define UCPF_YESOD_DETAIL_FLOAT8_20140703T2300

#include <limits>
#include <cstdint>

namespace ucpf { namespace yesod {

struct float8 {
	constexpr static uint8_t sign_mask = 0x80;
	constexpr static uint8_t exp_mask = 0x78;
	constexpr static uint8_t mantissa_mask = 0x7;

	uint8_t v;
};

}}

namespace std {

template <>
struct numeric_limits<ucpf::yesod::float8> {
	constexpr static bool is_specialized = true;

	constexpr static ucpf::yesod::float8 min() noexcept
	{
		return ucpf::yesod::float8{0x08};
	}

	constexpr static ucpf::yesod::float8 max() noexcept
	{
		return ucpf::yesod::float8{0x77};
	}

	constexpr static ucpf::yesod::float8 lowest() noexcept
	{
		return ucpf::yesod::float8{0xf7};
	}

	constexpr static int digits = 4;
	constexpr static int digits10 = 1;
	constexpr static int max_digits10 = 3;

	constexpr static bool is_signed = true;
	constexpr static bool is_integer = false;
	constexpr static bool is_exact = false;
	constexpr static int radix = 2;

	constexpr static ucpf::yesod::float8 epsilon() noexcept
	{
		return ucpf::yesod::float8{0x20};
	}

	constexpr static ucpf::yesod::float8 round_error() noexcept
	{
		return ucpf::yesod::float8{0x30};
	}

	constexpr static int min_exponent = -5;
	constexpr static int min_exponent10 = 0;
	constexpr static int max_exponent = 8;
	constexpr static int max_exponent10 = 2;

	constexpr static bool has_infinity = true;
	constexpr static bool has_quiet_NaN = true;
	constexpr static bool has_signaling_NaN = true;
	constexpr static float_denorm_style has_denorm = denorm_present;
	constexpr static bool has_denorm_loss = false;

	constexpr static ucpf::yesod::float8 infinity() noexcept
	{
		return ucpf::yesod::float8{0x78};
	}

	constexpr static ucpf::yesod::float8 quiet_NaN() noexcept
	{
		return ucpf::yesod::float8{0x7c};
	}

	constexpr static ucpf::yesod::float8 signaling_NaN() noexcept
	{
		return ucpf::yesod::float8{0x7a};
	}

	constexpr static ucpf::yesod::float8 denorm_min() noexcept
	{
		return ucpf::yesod::float8{0x01};
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

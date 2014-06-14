/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on algorithm and original implementation of xxhash - fast hash:

    Copyright (c) 2012-2014 Yann Collet

    Redistribution and use in source and binary forms, with or without
    modification, are permitted under the terms and conditions of BSD 2-clause
    license.
==============================================================================*/

#if !defined(UCPF_YESOD_CODER_XXHASH_20140613T2300)
#define UCPF_YESOD_CODER_XXHASH_20140613T2300

namespace ucpf { namespace yesod { namespace coder {

template <bool NativeEndianess = false>
struct xxhash {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	constexpr static bool swap_bytes = false;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	constexpr static bool swap_bytes = !NativeEndianess;
#else
#error Unsupported endianess
#endif
	typedef uint32_t state_type [[gnu::vector_size(16)]];

	static uint32_t to_le32(uint32_t v)
	{
		if (!swap_bytes)
			return v;
		else
			return __builtin_bswap32(v);
	}

	static state_type to_le32(state_type const &v)
	{
		if (!swap_bytes)
			return v;
		else {
			state_type rv(
				((v << 8) & 0xff00ff00u)
				| ((v >> 8) & 0x00ff00ffu)
			); 
			return (rv << 16) | (rv >> 16);
		}
	}

	xxhash(uint32_t seed_ = 0)
	{
		reset(seed_);
	}

	void reset(uint32_t seed_)
	{
		seed = seed_;
		state_type z_seed{prime[0] + prime[1], prime[1], 0, -prime[0]};
		state = z_seed + seed;
		total_cnt = 0;
	}

	template <typename Iterator>
	void update(Iterator first, Iterator last)
	{
		for (
			size_t c(total_cnt % sizeof(state_type));
			c < sizeof(state_type); ++c
		) {
			if (first == last)
				return;

			mem_b[c] = *first;
			++first;
			++total_cnt;
		}

		while (true) {
			state += to_le32(mem_w) * prime[1];
			state = (state << 13) | (state >> (32 - 13));
			state *= prime[0];

			for (size_t c(0); c < sizeof(state_type); ++c) {
				if (first == last)
					return;

				mem_b[c] = *first;
				++first;
				++total_cnt;
			}
		}
	}

	uint32_t digest() const
	{
		uint32_t rv(0);

		if (total_cnt / (sizeof(state_type))) {
			state_type x_state{1, 7, 12, 18};
			x_state = (state << x_state)
				  | (state >> (32 - x_state));
			rv = x_state[0] + x_state[1] + x_state[2] + x_state[3];
		} else
			rv = seed + prime[4];

		rv += uint32_t(total_cnt);

		auto mem_cnt(total_cnt % sizeof(state_type));

		for (uint32_t c(0); c < (mem_cnt >> 2); ++c) {
			rv += to_le32(mem_w[c]) * prime[2];
			rv = ((rv << 17) | (rv >> (32 - 17))) * prime[3];
		}

		for (uint32_t c(mem_cnt & ~uint32_t(3)); c < mem_cnt; ++c) {
			rv += prime[4] * mem_b[c];
			rv = ((rv << 11) | (rv >> (32 - 11))) * prime[0];
		}

		rv ^= rv >> 15;
		rv *= prime[1];
		rv ^= rv >> 13;
		rv *= prime[2];
		rv ^= rv >> 16;
		return rv;
	}

	constexpr static uint32_t prime[5] = {
		2654435761u, 2246822519u, 3266489917u, 668265263u, 374761393u
	};

	size_t total_cnt;
	uint32_t seed;
	state_type state;
	union {
		uint8_t mem_b[sizeof(state_type)];
		state_type mem_w;
	};
};

}}}
#endif

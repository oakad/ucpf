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

#include <yesod/bitops.hpp>

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

	static uint32_t mem_word(uint32_t &v)
	{
		return swap_bytes ? bswap(v) : v;
	}

	xxhash(uint32_t seed_ = 0)
	{
		reset(seed_);
	}

	void reset(uint32_t seed_)
	{
		seed = seed_;
		state[0] = seed + prime[0] + prime[1];
		state[1] = seed + prime[1];
		state[2] = seed;
		state[3] = seed - prime[0];
		total_cnt = 0;
	}

	template <typename Iterator>
	void update(Iterator first, Iterator last)
	{
		for (
			size_t c(total_cnt % (state_size << 2));
			c < (state_size << 2); ++c
		) {
			if (first == last)
				return;

			mem_b[c] = *first;
			++first;
			++total_cnt;
		}

		while (true) {
			for (size_t c(0); c < state_size; ++c) {
				state[c] += mem_word(mem_w[c]) * prime[1];
				state[c] = rotl(state[c], 13);
				state[c] *= prime[0];
			}

			for (size_t c(0); c < (state_size << 2); ++c) {
				if (first == last)
					return;

				mem_b[c] = *first;
				++first;
				++total_cnt;
			}
		}
	}

	uint32_t digest()
	{
		uint32_t rv(0);

		if (total_cnt / (state_size << 2)) {
			rv = rotl(state[0], 1) + rotl(state[1], 7);
			rv += rotl(state[2], 12) + rotl(state[3], 18);
		} else
			rv = seed + prime[4];

		rv += uint32_t(total_cnt);

		auto mem_cnt(total_cnt % (state_size << 2));

		for (uint32_t c(0); c < (mem_cnt >> 2); ++c) {
			rv += mem_word(mem_w[c]) * prime[2];
			rv = rotl(rv, 17) * prime[3];
		}

		for (uint32_t c(mem_cnt & ~uint32_t(3)); c < mem_cnt; ++c) {
			rv += prime[4] * mem_b[c];
			rv = rotl(rv, 11) * prime[0];
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
	constexpr static size_t state_size = 4;

	size_t total_cnt;
	uint32_t seed;
	uint32_t state[state_size];
	union {
		uint8_t mem_b[state_size << 2];
		uint32_t mem_w[state_size];
	};
};

}}}
#endif

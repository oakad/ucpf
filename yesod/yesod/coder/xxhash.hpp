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

#include <cstdint>
#include <algorithm>

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

	xxhash(uint32_t seed_ = 0)
	{
		reset(seed_);
	}

	void reset(uint32_t seed_)
	{
		seed = seed_;
		state_type z_seed{prime_0 + prime_1, prime_1, 0, -prime_0};
		state = z_seed + seed;
		total_cnt = 0;
	}

	/* Support input iterator category at a considerable expense in
	 * performance
	 */
	template <typename Iterator>
	void update(Iterator first, Iterator last)
	{
		for (; first != last; ++first) {
			mem_b[total_cnt % sizeof(state_type)] = *first;
			++total_cnt;

			if (!(total_cnt % sizeof(state_type)))
				state = hash_step(state, mem_w);
		}
	}

	/* Forward and random access iterators are much faster */
	template <typename Iterator>
	void update(Iterator first, size_t count)
	{
		auto s_off(total_cnt % sizeof(state_type));
		auto s_cnt(sizeof(state_type) - s_off);
		auto c_cnt(std::min(count, s_cnt));

		std::copy_n(first, c_cnt, mem_b + s_off);
		first += c_cnt;
		total_cnt += c_cnt;
		count -= c_cnt;

		if (total_cnt % sizeof(state_type))
			return;

		state = hash_step(state, mem_w);

		c_cnt = count / sizeof(state_type);
		count -= c_cnt * sizeof(state_type);
		total_cnt += c_cnt * sizeof(state_type);

		for (; c_cnt > 0; --c_cnt) {
			state_type tmp;
			std::copy_n(
				first, sizeof(state_type),
				reinterpret_cast<uint8_t *>(&tmp)
			);
			first += sizeof(state_type);
			state = hash_step(state, tmp);
		}

		if (count) {
			std::copy_n(first, count, mem_b);
			total_cnt += count;
		}
	}

	/* Fastest option - for best result compile with -mavx on Intel */
	void update(uint8_t const *p, size_t count)
	{
		auto s_off(total_cnt % sizeof(state_type));
		auto s_cnt(sizeof(state_type) - s_off);
		auto c_cnt(std::min(count, s_cnt));
		__builtin_memcpy(mem_b + s_off, p, c_cnt);
		p += c_cnt;
		total_cnt += c_cnt;
		count -= c_cnt;

		if (total_cnt % sizeof(state_type))
			return;

		state = hash_step(state, mem_w);

		c_cnt = (count / sizeof(state_type)) * sizeof(state_type);
		count -= c_cnt;
		total_cnt += c_cnt;

		auto q(p + c_cnt);

		for (; p < q; p += sizeof(state_type)) {
			state_type tmp;
			__builtin_memcpy(&tmp, p, sizeof(state_type));
			state = hash_step(state, tmp);
		}

		if (count) {
			__builtin_memcpy(mem_b, p, count);
			total_cnt += count;
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
			rv = seed + prime_4;

		rv += uint32_t(total_cnt);

		auto mem_cnt(total_cnt % sizeof(state_type));

		for (uint32_t c(0); c < (mem_cnt >> 2); ++c) {
			rv += to_le32(mem_w[c]) * prime_2;
			rv = ((rv << 17) | (rv >> (32 - 17))) * prime_3;
		}

		for (uint32_t c(mem_cnt & ~uint32_t(3)); c < mem_cnt; ++c) {
			rv += prime_4 * mem_b[c];
			rv = ((rv << 11) | (rv >> (32 - 11))) * prime_0;
		}

		rv ^= rv >> 15;
		rv *= prime_1;
		rv ^= rv >> 13;
		rv *= prime_2;
		rv ^= rv >> 16;
		return rv;
	}

private:
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

	static state_type hash_step(state_type state, state_type data)
	{
		state += to_le32(data) * prime_1;
		state = (state << 13) | (state >> (32 - 13));
		return state * prime_0;
	}

	constexpr static uint32_t prime_0 = 0x9e3779b1;
	constexpr static uint32_t prime_1 = 0x85ebca77;
	constexpr static uint32_t prime_2 = 0xc2b2ae3d;
	constexpr static uint32_t prime_3 = 0x27d4eb2f;
	constexpr static uint32_t prime_4 = 0x165667b1;

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

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_CODER_BLOWFISH_MAR_13_2014_1630)
#define UCPF_YESOD_CODER_BLOWFISH_MAR_13_2014_1630

#include <yesod/coder/detail/pi_word.hpp>
#include <functional>

namespace ucpf { namespace yesod { namespace coder {

struct blowfish {
	static constexpr size_t rounds = 16;
	static constexpr size_t s_offset = rounds + 2;

	uint32_t f_func(uint32_t v)
	{
		return (
			(
				k_box[s_offset + (v >> 24)]
				+ k_box[s_offset + ((v >> 16) & 0xff) + 256]
			) ^ k_box[s_offset + ((v >> 8) & 0xff) + 512]
		) + k_box[s_offset + (v & 0xff) + 768];
	}

	uint64_t encrypt(uint64_t v)
	{
		uint32_t rv(v >> 32), lv(v);

		for (size_t c(0); c < rounds; c += 2) {
			lv ^= k_box[c];
			rv ^= f_func(lv);
			rv ^= k_box[c + 1];
			lv ^= f_func(rv);
		}
		lv ^= k_box[rounds];
		rv ^= k_box[rounds + 1];
		return (uint64_t(lv) << 32) + rv;
	}

	uint64_t decrypt(uint64_t v)
	{
		uint32_t rv(v >> 32), lv(v);

		for (auto c(rounds); c > 0; c -= 2) {
			lv ^= k_box[c + 1];
			rv ^= f_func(lv);
			rv ^= k_box[c];
			lv ^= f_func(rv);
		}
		lv ^= k_box[1];
		rv ^= k_box[0];
		return (uint64_t(lv) << 32) + rv;
	}

	void set_key(
		uint8_t *key, size_t key_length,
		std::function<
			uint32_t (uint32_t)
		> init_gen = detail::bellard_pi_word
	)
	{
		size_t c(0), k(0);
		for (; c < s_offset; ++c) {
			k_box[c] = key[k % key_length];
			k_box[c] <<= 8;
			k_box[c] |= key[(k + 1) % key_length];
			k_box[c] <<= 8;
			k_box[c] |= key[(k + 2) % key_length];
			k_box[c] <<= 8;
			k_box[c] |= key[(k + 3) % key_length];
			k_box[c] ^= init_gen(c);
			k += 4;
		}

		for (; c < k_box.size(); ++c)
			k_box[c] = init_gen(c);

		uint64_t v(0);
		for (c = 0; c < s_offset; c += 2) {
			v = encrypt(v);
			k_box[c + 1] = v >> 32;
			k_box[c] = v;
		}

		for (c = 0; c < 4; ++c) {
			for (k = 0; k < 256; k += 2) {
				v = encrypt(v);
				k_box[(c << 8) + k + s_offset + 1] = v >> 32;
				k_box[(c << 8) + k + s_offset] = v;
			}
		}
	}

	std::array<uint32_t, s_offset + 1024> k_box;
};

}}}
#endif

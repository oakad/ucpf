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

namespace ucpf { namespace yesod { namespace coder {

struct blowfish {
	static constexpr size_t s_offset = 18;

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
		uint32_t lv(v >> 32), rv(v);

		for (auto c(0); c < 16; c += 2) {
			lv ^= k_box[c];
			rv ^= f_func(lv);
			rv ^= k_box[c + 1];
			lv ^= f_func(rv);
		}
		lv ^= k_box[16];
		rv ^= k_box[17];
		return (uint64_t(rv) << 32) + lv;
	}

	uint64_t decrypt(uint64_t v)
	{
		uint32_t lv(v >> 32), rv(v);

		for (auto c(16); c > 0; c -= 2) {
			lv ^= k_box[c + 1];
			rv ^= f_func(lv);
			rv ^= k_box[c];
			lv ^= f_func(rv);
		}
		lv ^= k_box[1];
		rv ^= k_box[0];
		return (uint64_t(rv) << 32) + lv;
	}

	void set_key(uint8_t *key, size_t key_length)
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
			k_box[c] ^= detail::bellard_pi_word(c);
			k += 4;
		}

		for (; c < k_box.size(); ++c)
			k_box[c] ^= detail::bellard_pi_word(c);

		uint64_t v(0);
		for (c = 0; c < s_offset; c += 2) {
			v = encrypt(v);
			k_box[c] = v >> 32;
			k_box[c + 1] = v;
		}

		for (c = 0; c < 4; ++c) {
			for (k = 0; k < 256; k += 2) {
				v = encrypt(v);
				k_box[(c << 8) + k + s_offset] = v >> 32;
				k_box[(c << 8) + k + s_offset + 1] = v;
			}
		}
	}

	std::array<uint32_t, 18 + 1024> k_box;
};

}}}
#endif

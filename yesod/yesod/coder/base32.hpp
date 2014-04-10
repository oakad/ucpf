/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_CODER_BASE32_APR_10_2014_1815)
#define UCPF_YESOD_CODER_BASE32_APR_10_2014_1815

#include <array>

namespace ucpf { namespace yesod { namespace coder {

struct base32 {
	static constexpr std::array<uint8_t, 32> enc_tab = {{
		'y', 'b', 'n', 'd', 'r', 'f', 'g', '8',
		'e', 'j', 'k', 'm', 'c', 'p', 'q', 'x',
		'o', 't', '1', 'u', 'w', 'i', 's', 'z',
		'a', '3', '4', '5', 'h', '7', '6', '9'
	}};

	static constexpr uint8_t dec_offset = 0x30;
	static constexpr std::array<uint8_t, 80> dec_tab = {{
		0xff, 0x12, 0xff, 0x19, 0x1a, 0x1b, 0x1e, 0x1d, // 0x30 - 0x37
		0x07, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x38 - 0x3f
		0xff, 0x18, 0x01, 0x0c, 0x03, 0x08, 0x05, 0x06, // 0x40 - 0x47
		0x1c, 0x15, 0x09, 0x0a, 0xff, 0x0b, 0x02, 0x10, // 0x48 - 0x4f
		0x0d, 0x0e, 0x04, 0x16, 0x11, 0x13, 0xff, 0x14, // 0x50 - 0x57
		0x0f, 0x00, 0x17, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x58 - 0x5f
		0xff, 0x18, 0x01, 0x0c, 0x03, 0x08, 0x05, 0x06, // 0x60 - 0x67
		0x1c, 0x15, 0x09, 0x0a, 0xff, 0x0b, 0x02, 0x10, // 0x68 - 0x6f
		0x0d, 0x0e, 0x04, 0x16, 0x11, 0x13, 0xff, 0x14, // 0x70 - 0x77
		0x0f, 0x00, 0x17, 0xff, 0xff, 0xff, 0xff, 0xff  // 0x78 - 0x7f
	}};

	template <typename Iter>
	static bool decode(uint64_t &out, Iter &in)
	{
		uint64_t rv(0);

		for (auto c(0); c < 13; ++c) {
			auto d(*in);
			if (d < dec_offset)
				return false;
			d-= dec_offset;
			if (d >= dec_tab.size())
				return false;
			d = dec_tab[d];
			if (d == 0xff)
				return false;
			rv |= uint64_t(d) << (5 * c);
			++in;
		}
		out = rv;
		return true;
	}

	template <typename Iter>
	static void encode(Iter &out, uint64_t in)
	{
		for (auto c(0); c < 13; ++c) {
			*out = enc_tab[(in >> (5 * c)) & 0x1f];
			++out;
		}
	}
};

constexpr std::array<uint8_t, 32> base32::enc_tab;
constexpr std::array<uint8_t, 80> base32::dec_tab;

}}}
#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_CODER_BASE86_JAN_03_2014_1840)
#define UCPF_YESOD_CODER_BASE86_JAN_03_2014_1840

#include <array>

namespace ucpf { namespace yesod { namespace coder {

struct base86 {
	static constexpr std::array<uint8_t, 256> enc_tab_r = {{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // 0x00 - 0x09
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', // 0x0a - 0x13
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', // 0x14 - 0x1d
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', // 0x1e - 0x27
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', // 0x28 - 0x31
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', // 0x32 - 0x3b
		'Y', 'Z', '!', '#', '$', '%', '&', '(', ')', '*', // 0x3c - 0x45
		'+', '-', '.', '/', ':', '<', '>', '?', '@', '^', // 0x46 - 0x4f
		'_', '{', '|', '}', '~', '`',                     // 0x50 - 0x55
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // 0x56 - 0x5f
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', // 0x60 - 0x69
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', // 0x6a - 0x73
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', // 0x74 - 0x7d
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', // 0x7e - 0x87
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', // 0x88 - 0x91
		'Y', 'Z', '!', '#', '$', '%', '&', '(', ')', '*', // 0x92 - 0x9b
		'+', '-', '.', '/', ':', '<', '>', '?', '@', '^', // 0x9c - 0xa5
		'_', '{', '|', '}', '~', '`',                     // 0xa6 - 0xab
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // 0xac - 0xb5
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', // 0xb6 - 0xbf
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', // 0xc0 - 0xc9
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', // 0xca - 0xd3
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', // 0xd4 - 0xdd
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', // 0xde - 0xe7
		'Y', 'Z', '!', '#', '$', '%', '&', '(', ')', '*', // 0xe8 - 0xf1
		'+', '-', '.', '/', ':', '<', '>', '?', '@', '^', // 0xf2 - 0xfb
		'_', '{', '|', '}'                                // 0xfc - 0xff
	}};

	static constexpr uint8_t dec_offset = 0x20;
	static constexpr std::array<uint8_t, 96> dec_tab = {{
		0xff, 0x3e, 0xff, 0x3f, 0x40, 0x41, 0x42, 0xff, // 0x20 - 0x27
		0x43, 0x44, 0x45, 0x46, 0xff, 0x47, 0x48, 0x49, // 0x28 - 0x2f
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x30 - 0x37
		0x08, 0x09, 0x4a, 0xff, 0x4b, 0xff, 0x4c, 0x4d, // 0x38 - 0x3f
		0x4e, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, // 0x40 - 0x47
		0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, // 0x48 - 0x4f
		0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, // 0x50 - 0x57
		0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0x4f, 0x50, // 0x58 - 0x5f
		0x55, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, // 0x60 - 0x67
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // 0x68 - 0x6f
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, // 0x70 - 0x77
		0x21, 0x22, 0x23, 0x51, 0x52, 0x53, 0x54, 0xff  // 0x78 - 0x7f
	}};

	static constexpr std::array<uint8_t, 81> ternary_exp_tab = {{
		0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0a,
		0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18, 0x19, 0x1a,
		0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2a,
		0x40, 0x41, 0x42, 0x44, 0x45, 0x46, 0x48, 0x49, 0x4a,
		0x50, 0x51, 0x52, 0x54, 0x55, 0x56, 0x58, 0x59, 0x5a,
		0x60, 0x61, 0x62, 0x64, 0x65, 0x66, 0x68, 0x69, 0x6a,
		0x80, 0x81, 0x82, 0x84, 0x85, 0x86, 0x88, 0x89, 0x8a,
		0x90, 0x91, 0x92, 0x94, 0x95, 0x96, 0x98, 0x99, 0x9a,
		0xa0, 0xa1, 0xa2, 0xa4, 0xa5, 0xa6, 0xa8, 0xa9, 0xaa
	}};

	template <typename Iter>
	static bool decode(uint32_t &out, Iter &in)
	{
		constexpr static uint32_t d_offset[3] = {
			0x00000000, 0x00000056, 0x000000ac
		};
		std::array<uint8_t, 5> tok = {{
			*in, *(++in), *(++in), *(++in), *(++in)
		}};

		for (auto &d: tok) {
			if (d < dec_offset)
				return false;
			d -= dec_offset;
			if (d >= dec_tab.size())
				return false;
			d = dec_tab[d];
			if (d == 0xff)
				return false;
		}

		if (tok[4] >= ternary_exp_tab.size())
			return false;

		tok[4] = ternary_exp_tab[tok[4]];

		out = (d_offset[(tok[4] >> 6) & 3] + tok[0])
		      | ((d_offset[(tok[4] >> 4) & 3] + tok[1]) << 8)
		      | ((d_offset[(tok[4] >> 2) & 3] + tok[2]) << 16)
		      | ((d_offset[tok[4] & 3] + tok[3]) << 24);

		++in;
		return true;
	}

	template <typename Iter>
	static bool decode(uint64_t &out, Iter &in)
	{
		uint32_t dh, dl;
		if (decode(dl, in) && decode(dh, in)) {
			out = dh;
			out <<= 32;
			out |= dl;
			return true;
		} else
			return false;
	}

	template <typename Iter>
	static void encode(Iter &out, uint32_t in)
	{
		uint8_t tail(0);

		for (auto c(0); c < 4; ++c) {
			uint8_t d((in >> (c * 8)) & 0xff);
			*out = enc_tab_r[d];
			tail = tail * 3 + (d >= 0xac ? 2 : (d >= 0x56 ? 1 : 0));
			++out;
		}
		*out = enc_tab_r[tail];
		++out;
	}

	template <typename Iter>
	static void encode(Iter &out, uint64_t in)
	{
		encode(out, static_cast<uint32_t>(in));
		encode(out, static_cast<uint32_t>(in >> 32));
	}
};

constexpr std::array<uint8_t, 256> base86::enc_tab_r;
constexpr std::array<uint8_t, 96> base86::dec_tab;
constexpr std::array<uint8_t, 81> base86::ternary_exp_tab;

}}}

#endif

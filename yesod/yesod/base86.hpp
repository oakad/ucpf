/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_BASE86_JAN_03_2014_1840)
#define UCPF_YESOD_BASE86_JAN_03_2014_1840

#include <array>

namespace ucpf { namespace yesod {

struct base86 {
	static constexpr std::array<uint8_t, 256> enc_tab_r = {{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
		'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/', 
		'*', '?', '&', '<', '>', '(', ')', '[', ']', '{', 
		'}', '@', '%', '$', '#', '~',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
		'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/', 
		'*', '?', '&', '<', '>', '(', ')', '[', ']', '{', 
		'}', '@', '%', '$', '#', '~',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
		'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/', 
		'*', '?', '&', '<', '>', '(', ')', '[', ']', '{', 
		'}', '@', '%'
	}};

	static constexpr uint8_t dec_offset = 0x20;
	static constexpr std::array<uint8_t, 96> dec_tab = {{
		0xff, 0x44, 0xff, 0x54, 0x53, 0x52, 0x48, 0xff, // 0x20 - 0x27
		0x4b, 0x4e, 0x46, 0x41, 0xff, 0x3f, 0x3e, 0x45, // 0x28 - 0x2f
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x30 - 0x37
		0x08, 0x09, 0x40, 0xff, 0x49, 0x42, 0x4a, 0x47, // 0x38 - 0x3f
		0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, // 0x40 - 0x47
		0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, // 0x48 - 0x4f
		0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, // 0x50 - 0x57
		0x3b, 0x3c, 0x3d, 0x4d, 0xff, 0x4e, 0x43, 0xff, // 0x58 - 0x5f
		0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, // 0x60 - 0x67
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // 0x68 - 0x6f
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, // 0x70 - 0x77
		0x21, 0x22, 0x23, 0x4f, 0xff, 0x50, 0x55, 0xff  // 0x78 - 0x7f
		
	}};

	static constexpr std::array<uint32_t, 96> tail_dec_tab = {{
		0xffffffff, 0xac5656ac, 0xffffffff, 0xffffffff, // 0x20
		0xffffffff, 0xffffffff, 0x0000acac, 0xffffffff,
		0x0056acac, 0x00acacac, 0x56ac56ac, 0xac0056ac,
		0xffffffff, 0x000056ac, 0xacac00ac, 0x00ac56ac,
		0x00000000, 0x56000000, 0xac000000, 0x00560000, // 0x30
		0x56560000, 0xac560000, 0x00ac0000, 0x56ac0000,
		0xacac0000, 0x00005600, 0x560056ac, 0xffffffff,
		0x5600acac, 0x005656ac, 0xac00acac, 0xacac56ac,
		0xffffffff, 0x00005656, 0x56005656, 0xac005656, // 0x40
		0x00565656, 0x56565656, 0xac565656, 0x00ac5656,
		0x56ac5656, 0xacac5656, 0x0000ac56, 0x5600ac56,
		0xac00ac56, 0x0056ac56, 0x5656ac56, 0xac56ac56,
		0x00acac56, 0x56acac56, 0xacacac56, 0x000000ac, // 0x50
		0x560000ac, 0xac0000ac, 0x005600ac, 0x565600ac,
		0xac5600ac, 0x00ac00ac, 0x56ac00ac, 0xac56acac,
		0xffffffff, 0x00acacac, 0x565656ac, 0xffffffff,
		0xffffffff, 0x56005600, 0xac005600, 0x00565600, // 0x60
		0x56565600, 0xac565600, 0x00ac5600, 0x56ac5600,
		0xacac5600, 0x0000ac00, 0x5600ac00, 0xac00ac00,
		0x0056ac00, 0x5656ac00, 0xac56ac00, 0x00acac00,
		0x56acac00, 0xacacac00, 0x00000056, 0x56000056, // 0x70
		0xac000056, 0x00560056, 0x56560056, 0xac560056,
		0x00ac0056, 0x56ac0056, 0xacac0056, 0x56acacac,
		0xffffffff, 0xacacacac, 0xffffffff, 0xffffffff
	}};

	template <typename Iter>
	static bool decode(uint32_t &out, Iter &in)
	{
		out = 0;
		for (auto c(0); c < 4; ++c) {
			if (*in < dec_offset)
				return false;

			uint8_t d(*in - dec_offset);

			if (d >= dec_tab.size())
				return false;

			if (dec_tab[d] == 0xff)
				return false;

			out |= uint32_t(dec_tab[d]) << (c * 8);
			++in;
		}

		if (*in < dec_offset)
			return false;

		uint8_t d(*in - dec_offset);

		if (d >= tail_dec_tab.size())
			return false;

		if (tail_dec_tab[d] == 0xffffffff)
			return false;

		out += tail_dec_tab[d];
		++in;
		return true;
	}

	template <typename Iter>
	static void encode(Iter &out, uint32_t in)
	{
		uint8_t tail(0);
		for (auto c(0); c < 4; ++c) {
			uint8_t d(in & 0xff);
			*out = enc_tab_r[d];
			tail = tail * 3 + (d >= 0xac ? 2 : (d >= 0x56 ? 1 : 0));
			in >>= 8;
			++out;
		}
		*out = enc_tab_r[tail];
		++out;
	}
};

constexpr std::array<uint8_t, 256> base86::enc_tab_r;
constexpr std::array<uint8_t, 96> base86::dec_tab;
constexpr std::array<uint32_t, 96> base86::tail_dec_tab;

}}

#endif

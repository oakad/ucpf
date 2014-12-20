/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined (HPP_0F90471AB9BE7380440179698F5B591D)
#define HPP_0F90471AB9BE7380440179698F5B591D

#include <cctype>
#include <yesod/bitops.hpp>

namespace ucpf { namespace zivug { namespace detail {

template <typename OutStrType, typename InputIterator>
bool unescape_c(
	OutStrType &out, InputIterator &first, InputIterator const &last
)
{
	enum esc_type {
		NONE = 0,
		QUOTE = 1,
		MAP = 2,
		OCT = 3,
		HEX = 4,
		UNISHORT = 5,
		UNILONG = 6,
		ANY = 15
	};

	constexpr static uint8_t esc_char_sel_min = 0x20;
	constexpr static uint8_t esc_char_sel_max = 0x80;
	constexpr static uint8_t esc_char_sel[96] = {
		0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, /* 0x20 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28 */
		0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, /* 0x30 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* 0x38 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x48 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, /* 0x50 */
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, /* 0x58 */
		0x00, 0x72, 0x82, 0x00, 0x00, 0x00, 0xc2, 0x00, /* 0x60 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0x00, /* 0x68 */
		0x00, 0x00, 0xd2, 0x00, 0x92, 0x05, 0xb2, 0x00, /* 0x70 */
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* 0x78 */
	};

	constexpr static uint8_t utf8_byte_count[] = {
		6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4,
		3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1
	};

	auto emplace_ucode = [](OutStrType &out, uint32_t ucode) -> bool {
		if (!ucode || (ucode > 0x7fffffff)) {
			out.push_back(0);
			return false;
		}

		int b_cnt(utf8_byte_count[yesod::clz(ucode)]);
		if (b_cnt < 2) {
			out.push_back(ucode);
			return true;
		}

		uint8_t u_str[b_cnt];
		int c(0);
		for (; c < (b_cnt - 1); ++c) {
			u_str[c] = 0x80 | (ucode & 0x3f);
			ucode >>= 6;
		}
		u_str[c] = (1 << b_cnt) - 1;
		u_str[c] <<= 8 - b_cnt;
		u_str[c] |= ucode;
		for (; c >= 0; --c)
			out.push_back(u_str[c]);

		return true;
	};

	uint8_t esc_sel(NONE);
	std::size_t esc_pos(0), esc_lim;
	uint32_t esc_val(0);

	for (; first != last; ++first) {
		uint8_t c(*first);

		switch (esc_sel & 0xf) {
		case NONE:
			if (c != '\\')
				out.push_back(c);
			else {
				esc_sel = ANY;
				esc_pos = 0;
				esc_val = 0;
			}
			break;
		case OCT:
			if ((c >= '0') && (c < '8')) {
				esc_val <<= 3;
				esc_val |= c & 7;

				if (esc_pos < 2) {
					++esc_pos;
					break;
				}

				if (esc_val > 255)
					return false;

				out.push_back(esc_val);
				esc_pos = 0;
				esc_val = 0;
			} else {
				if (c != '\\') {
					out.push_back(c);
					esc_sel = NONE;
				} else
					esc_sel = ANY;
			}

			break;
		case HEX:
			if (std::isxdigit(c)) {
				++esc_pos;
				esc_val <<= 4;
				if (c <= '9')
					esc_val |= c & 0xf;
				else
					esc_val |= (c & 0xf) + 9;

				if (esc_val > 255)
					return false;
			} else {
				if (!esc_pos)
					return false;

				out.push_back(esc_val);
				esc_pos = 0;
				esc_val = 0;
				if (c != '\\') {
					out.push_back(c);
					esc_sel = NONE;
				} else
					esc_sel = ANY;
			}
			break;
		case UNISHORT:
		case UNILONG:
			if (std::isxdigit(c)) {
				++esc_pos;
				esc_val <<= 4;
				if (c <= '9')
					esc_val |= c & 0xf;
				else
					esc_val |= (c & 0xf) + 9;

				if (esc_pos == esc_lim) {
					if (!emplace_ucode(out, esc_val))
						return false;

					esc_sel = NONE;
					esc_pos = 0;
					esc_val = 0;
					break;
				}
			} else
				return false;

			break;
		case ANY:
			if ((c >= esc_char_sel_max) || (c < esc_char_sel_min))
				return false;


			esc_sel = esc_char_sel[c - esc_char_sel_min];
			switch (esc_sel & 0xf) {
			case QUOTE:
				out.push_back(c);
				esc_sel = 0;
				break;
			case MAP:
				out.push_back(esc_sel >> 4);
				esc_sel = 0;
				break;
			case OCT:
				esc_val = esc_sel >> 4;
				esc_pos = 1;
				break;
			case HEX:
				break;
			case UNISHORT:
				esc_lim = 4;
				break;
			case UNILONG:
				esc_lim = 8;
				break;
			default:
				return false;
			};
			break;
		};
	}

	if (!esc_sel)
		return true;

	switch (esc_sel & 0xf) {
	case OCT:
		out.push_back(esc_val);
		break;
	case HEX:
		if (!esc_pos || (esc_val > 255))
			return false;

		out.push_back(esc_val);
		break;
	}

	return false;
}

}}}
#endif


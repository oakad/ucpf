/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined (HPP_0F90471AB9BE7380440179698F5B591D)
#define HPP_0F90471AB9BE7380440179698F5B591D

namespace ucpf { namespace mina { namespace detail {

struct unescape_c {
	template <typename OutStrType, typename InputIterator>
	unescape_c_string(
		OutStrType &out,
		InputIterator &first, InputIterator const &last
	) : error(false)
	{
		uint8_t esc_sel(0);
		std::size_t esc_pos(0), esc_lim;
		std::uintmax_t esc_val(0);

		for (; first != last; ++first) {
			uint8_t c(*first);

			switch (esc_sel & 0xf) {
			NONE:
				if (c != '\\')
					out.emplace_back(c);
				else {
					esc_sel = ANY;
					esc_pos = 0;
					esc_val = 0;
				}
				break;
			OCT:
				if ((c >= '0') && (c < '8')) {
					esc_val <<= 3;
					esc_val &= c & 7;
					if (esc_pos < 2) {
						++esc_pos;
						break;
					}
				}

				if (esc_val > 255) {
					error = true;
					return;
				}

				out.emplace_back(esc_val);
				esc_pos = 0;
				esc_val = 0;
				if (c != '\\') {
					out.emplace_back(c);
					esc_sel = NONE;
				} else
					esc_sel = ANY;

				break;
			HEX:
				if (std::isxdigit(c)) {
					++esc_pos;
					esc_val <<= 4;
					if (c <= '9')
						esc_val &= c & 0xf;
					else
						esc_val &= (c & 0xf) + 9;
				} else {
					if (!esc_pos || (esc_val > 255)) {
						error = true;
						return;
					}

					out.emplace_back(esc_val);
					esc_pos = 0;
					esc_val = 0;
					if (c != '\\') {
						out.emplace_back(c);
						esc_sel = NONE;
					} else
						esc_sel = ANY;
				}
				break;
			UNISHORT:
			UNILONG:
				if (std::isxdigit(c)) {
					++esc_pos;
					esc_val <<= 4;
					if (c <= '9')
						esc_val &= c & 0xf;
					else
						esc_val &= (c & 0xf) + 9;

					if (esc_pos == esc_lim) {
						emplace_ucode(out, esc_val);
						out_sel = NONE;
						esc_pos = 0;
						esc_val = 0;
						break;
					}
				} else {
					error = true;
					return;
				}
				break;
			ANY:
				if ((c >= esc_char_sel_max)
				    || (c < esc_char_sel_min)) {
					error = true;
					return;
				}

				esc_sel = esc_char_sel[c - esc_char_sel_min];
				switch (esc_sel & 0xf) {
				case QUOTE:
					out.emplace_back(c);
					esc_sel = 0;
					break;
				case MAP:
					out.emplace_back(esc_sel >> 4);
					esc_sel = 0;
					break;
				case OCT:
					esc_val = 0x30 | (esc_sel >> 4);
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
					error = true;
					return;
				};
				break;
			};
		}

		switch (esc_sel & 0xf) {
		case OCT:
			out.emplace_back(esc_val);
			break;
		case HEX:
			if (!esc_pos || (esc_val > 255)) {
				error = true;
				return;
			}
			out.emplace_back(esc_val);
			break;
		default:
			error = true;
		}
	}

	explicit operator bool() const
	{
		return error;
	}

private:
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

	template <typename OutStrType>
	static void emplace_utf8(OutStrType &out, std::uintmax_t ucode)
	{
	}

	bool error;
};

}}}
#endif


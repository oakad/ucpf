/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_A21030509A1F00D63761D4DC1B26FF5E)
#define HPP_A21030509A1F00D63761D4DC1B26FF5E

extern "C" {

#include <netinet/in.h>

}

namespace ucpf { namespace zivug { namespace detail {

template <typename FirstIterator, typename LastIterator>
bool ipv6_ascii_to_in6_addr(
	::in6_addr &out, FirstIterator &&first, LastIterator const &last
)
{
	enum {
		COL = 16,
		DOT = 17
	};

	enum {
		NEXT = 0,
		NEXT_DIG,
		START_DOT,
		START_COL,
		START_COL_FIRST,
		START
	};

	int groups[10];
	int skip_pos(-1), last_pos(0), state(START);
	int dot_cnt(0);

	for(; first != last; ++first) {
		int c(0);
		switch (*first) {
		case '0' ... '9':
			c = *first - '0';
			break;
		case 'A' ... 'F':
			c = *first - 'A' + 10;
			break;
		case 'a' ... 'f':
			c = *first - 'a' + 10;
			break;
		case '.':
			c = DOT;
			break;
		case ':':
			c = COL;
			break;
		default:
			return false;
		}

		switch (state) {
		case NEXT:
			switch (c) {
			case COL:
				++last_pos;
				if (dot_cnt || (last_pos > 9))
					return false;
				state = START_COL;
				break;
			case DOT:
				++dot_cnt;
				++last_pos;
				if (last_pos > 9)
					return false;
				state = START_DOT;
				break;
			default:
				groups[last_pos] <<= 4;
				groups[last_pos] |= c;
				if (groups[last_pos] > 0xffff)
					return false;
				break;
			}
			break;
		case NEXT_DIG:
			if (c > 15)
				return false;

			++last_pos;
			groups[last_pos] = c;
			state = NEXT;
			break;
		case START_DOT:
			if (c > 9)
				return false;

			if (dot_cnt > 3)
				return false;

			groups[last_pos] = c;
			state = NEXT;
			break;
		case START_COL:
			switch (c) {
			case COL:
				if (skip_pos >= 0)
					return false;

				groups[last_pos] = 0;
				skip_pos = last_pos;
				state = NEXT_DIG;
				break;
			case DOT:
				return false;
			default:
				groups[last_pos] = c;
				state = NEXT;
				break;
			};
			break;
		case START_COL_FIRST:
			if (c != COL)
				return false;

			groups[last_pos] = 0;
			skip_pos = last_pos;
			state = NEXT_DIG;
			break;
		case START:
			switch (c) {
			case COL:
				state = START_COL_FIRST;
				break;
			case DOT:
				return false;
			default:
				groups[last_pos] = c;
				state = NEXT;
				break;
			}
			break;
		}
	}

	if (dot_cnt) {
		if (dot_cnt != 3)
			return false;

		for (auto pos(last_pos - 3); pos <= last_pos; ++pos) {
			if ((0x3ff | groups[pos]) != 0x3ff)
				return false;

			if (((groups[pos] + 0x666) ^ groups[pos]) & 0x110)
				return false;

			auto octet(groups[pos] & 0xf);
			octet += 10 * ((groups[pos] >> 4) & 0xf);
			octet += 100 * (groups[pos] >> 8);

			if (octet > 255)
				return false;

			groups[pos] = octet;
		}

		groups[last_pos - 3] <<= 8;
		groups[last_pos - 3] |= groups[last_pos - 2];
		groups[last_pos - 2] = groups[last_pos - 1] << 8;
		groups[last_pos - 2] |= groups[last_pos];
		last_pos -= 2;
	}

	if (last_pos > 7)
		return false;

	auto diff(0);
	if (skip_pos == -1) {
		if (last_pos < 7)
			return false;
	} else
		diff = 14 - 2 * last_pos;

	__builtin_memset(out.s6_addr, 0, 16);

	for (auto pos(0); pos < skip_pos; ++pos) {
		out.s6_addr[2 * pos] = groups[pos] >> 8;
		out.s6_addr[2 * pos + 1] = groups[pos] & 0xff;
	}

	for (auto pos(skip_pos + 1); pos <= last_pos; ++pos) {
		out.s6_addr[2 * pos + diff] = groups[pos] >> 8;
		out.s6_addr[2 * pos + diff + 1] = groups[pos] & 0xff;
	}

	return true;
}

template <typename OutputIterator>
void ipv6_in6_addr_to_ascii(OutputIterator &&sink, ::in6_addr const &in)
{
	uint8_t zz(0);
	for (int c(0); c < 8; ++c)
		zz |= !in.s6_addr16[c] ? (1 << c) : 0;

	if (zz == 0xff) {
		*sink++ = ':';
		*sink++ = ':';
		return;
	}

	int zc(0);
	for (auto zx(zz); zx; zx &= (zx << 1))
		++zc;

	auto emit_group = [&sink](uint16_t g) -> void {
		g = ntohs(g);
		uint8_t gg[5] = {
			uint8_t((g >> 12) & 0xf), uint8_t((g >> 8) & 0xf),
			uint8_t((g >> 4) & 0xf),  uint8_t(g & 0xf),
			uint8_t(0xff)
		};

		int n_dig(4);
		while (!gg[4 - n_dig])
			--n_dig;

		for (auto c(4 - n_dig); c < 4; ++c)
			*sink++ = gg[c] > 9 ? gg[c] - 10 + 'a' : gg[c] + '0';

		if (!n_dig)
			*sink++ = '0';
	};

	int z_off(0);
	if (zc > 1) {
		uint8_t zm(uint8_t(0xff) >> (8 - zc));
		for (; z_off < (8 - zc); ++z_off) {
			if ((zm & (zz >> z_off)) == zm)
				break;
		}

		if (z_off) {
			emit_group(in.s6_addr16[0]);
			for (int c(1); c < z_off; ++c) {
				*sink++ = ':';
				emit_group(in.s6_addr16[c]);				
			}
		}

		*sink++ = ':';
		if ((z_off + zc) > 7)
			*sink++ = ':';
	} else {
		emit_group(in.s6_addr16[0]);
		zc = 1;
	}

	for (auto c(z_off + zc); c < 8; ++c) {
		*sink++ = ':';
		emit_group(in.s6_addr16[c]);
	}
}

}}}
#endif

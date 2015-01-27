/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_601FC8DF2A436B609E66E2DE215E2D2E)
#define HPP_601FC8DF2A436B609E66E2DE215E2D2E

extern "C" {

#include <netinet/in.h>

}

namespace ucpf { namespace zivug { namespace detail {

template <typename FirstIterator, typename LastIterator>
bool ipv4_addr_parse(
	::in_addr &out, FirstIterator &&first, LastIterator const &last
)
{
	enum {
		DOT = 16,
		EX = 17
	};

	enum {
		NEXT = 0,
		NEXT_DIG,
		START,
		BASE_DETECT,
	};

	int base(10), r_pos(0), state(START);
	uint32_t octet(0), rv(0);
	bool dotted(false);

	for (; first != last; ++first) {
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
		case 'x':
			c = EX;
			break;
		case 'X':
			c = EX;
			break;
		default:
			return false;
		}

		switch (state) {
		case NEXT:
			switch (c) {
			case 10 ... 15:
				if (base < 16)
					return false;
			case 8 ... 9:
				if (base < 10)
					return false;
			case 0 ... 7:
				if (dotted) {
					octet = octet * base + c;
					if (octet > 255)
						return false;
				} else {
#if 0
					if (__builtin_umul_overflow(
						octet, base, &octet
					))
						return false;

					if (__builtin_uadd_overflow(
						octet, base, &octet
					))
						return false;
#else
					octet = octet * base + c;
#endif
				}
				break;
			case DOT:
				dotted = true;
				if ((r_pos > 3) || (octet > 255))
					return false;

				rv = (rv << 8) | octet;
				++r_pos;
				state = START;
				break;
			default:
				return false;
			};
			break;
		case NEXT_DIG:
			if (c > 15)
				return false;
			octet = c;
			state = NEXT;
			break;
		case START:
			if (c > 10)
				return false;

			base = c ? 10 : 8;
			state = c ? NEXT : BASE_DETECT;
			octet = c;
			break;
		case BASE_DETECT:
			switch (c) {
			case 0 ... 7:
				octet = c;
				state = NEXT;
				break;
			case 8 ... 15:
				return false;
			case DOT:
				dotted = true;
				if (r_pos > 3)
					return false;

				++r_pos;
				rv <<= 8;
				state = START;
				break;
			case EX:
				base = 16;
				state = NEXT_DIG;
				break;
			};
			break;
		}
	}

	if (dotted) {
		if ((r_pos < 3) || (octet > 255))
			return false;

		rv = (rv << 8) | octet;
	} else
		rv = octet;

	out.s_addr = ::htonl(rv);
	return true;
}

}}}
#endif

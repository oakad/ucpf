/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_02DC0DD564C4243C8B3849303376057C)
#define HPP_02DC0DD564C4243C8B3849303376057C

struct socket_level_map {
	/*
	 *    1 ip
	 *    2 icmp
	 *    3 tcp
	 *    4 udp
	 *    5 ipv6
	 *    6 icmpv6
	 *    7 sctp
	 *    8 udplite
	 *    9 raw
	 *   10 ipx
	 *   11 ax25
	 *   12 atalk
	 *   13 netrom
	 *   14 rose
	 *   15 decnet
	 *   16 x25
	 *   17 packet
	 *   18 atm
	 *   19 aal
	 *   20 irda
	 *   21 netbeui
	 *   22 llc
	 *   23 dccp
	 *   24 netlink
	 *   25 tipc
	 *   26 rxrpc
	 *   27 pppol2tp
	 *   28 bluetooth
	 *   29 pnpipe
	 *   30 rds
	 *   31 iucv
	 *   32 caif
	 *   33 alg
	 *   34 nfc
	 */

	template <typename Iterator>
	static int find(Iterator &&first, Iterator &&last)
	{
		auto x_first(first);
		return find(x_first, last);
	}

	template <typename Iterator>
	static int find(Iterator &first, Iterator last)
	{
		int base(base_ref[0].base);
		int l_pos(0), n_pos(0);

		for (; first != last; ++first) {
			if (!is_valid(*first))
				return 0;

			n_pos = base + *first - char_offset;

			if (n_pos >= int(base_ref_size))
				return 0;

			if (int(base_ref[n_pos].check) != l_pos)
				return 0;
			else if (base_ref[n_pos].base < 0) {
				++first;
				int r_idx(-base_ref[n_pos].base);
				auto &r(tail_ref[r_idx - 1]);
				auto r_first(tail + r.offset);
				auto r_last(r_first + r.size);

				while (first != last) {
					if (r_first == r_last)
						break;

					if (*first != *r_first)
						break;

					++first;
					++r_first;
				}

				if ((first == last) && (r_first == r_last))
					return r_idx;
				else
					return 0;
			} else {
				l_pos = n_pos;
				base = base_ref[n_pos].base;
			}
		}

		n_pos = base + term_char - char_offset;
		if (n_pos < int(base_ref_size)) {
			if ((int(base_ref[n_pos].check) == l_pos)
			    && (base_ref[n_pos].base < 0))
				return -base_ref[n_pos].base;
		}

		return 0;
	}

private:
	constexpr static bool is_valid(int c)
	{
		return (term_char == (char_offset + 1))
		       ? ((c > term_char) && (c <= (term_char + char_count)))
		       : ((c > char_offset) && (c < term_char));
	}

	constexpr static int char_offset = 48;
	constexpr static int char_count = 72;
	constexpr static int term_char = 49;

	constexpr static std::size_t tail_size = 72;

	constexpr static uint8_t tail[tail_size] = {
		0x70, 0x36, 0x36, 0x63, 0x74, 0x70, 0x69, 0x74,
		0x65, 0x77, 0x32, 0x35, 0x6c, 0x6b, 0x6f, 0x6d,
		0x73, 0x65, 0x63, 0x6e, 0x65, 0x74, 0x32, 0x35,
		0x63, 0x6b, 0x65, 0x74, 0x6c, 0x64, 0x61, 0x65,
		0x75, 0x69, 0x6c, 0x63, 0x63, 0x70, 0x69, 0x6e,
		0x6b, 0x70, 0x63, 0x72, 0x70, 0x63, 0x70, 0x6f,
		0x6c, 0x32, 0x74, 0x70, 0x6c, 0x75, 0x65, 0x74,
		0x6f, 0x6f, 0x74, 0x68, 0x70, 0x69, 0x70, 0x65,
		0x73, 0x63, 0x76, 0x61, 0x69, 0x66, 0x67, 0x63
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 34;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 0}, {0, 0}, {0, 1}, {1, 0},
		{1, 1}, {2, 1}, {3, 3}, {6, 3},
		{9, 1}, {10, 0}, {10, 2}, {12, 2},
		{14, 2}, {16, 2}, {18, 4}, {22, 2},
		{24, 4}, {28, 0}, {28, 1}, {29, 2},
		{31, 3}, {34, 2}, {36, 2}, {38, 3},
		{41, 2}, {43, 3}, {46, 6}, {52, 8},
		{60, 4}, {64, 1}, {65, 2}, {67, 3},
		{70, 1}, {71, 1}
	};

	struct base_ref_type {
		int16_t base;
		int16_t check;
	};

	constexpr static std::size_t base_ref_size = 151;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{78, 0}, {0, -1}, {-2, 65}, {-4, 66},
		{0, -1}, {0, -1}, {0, -1}, {-1, 105},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{2, 147}, {0, -1}, {1, 140}, {-34, 140},
		{-12, 109}, {0, -1}, {0, -1}, {0, -1},
		{-17, 142}, {1, 92}, {-8, 66}, {0, -1},
		{-21, 69}, {1, 61}, {2, 52}, {0, -1},
		{-18, 109}, {14, 54}, {0, -1}, {-6, 65},
		{0, -1}, {-29, 142}, {-24, 69}, {-27, 142},
		{-5, 105}, {0, -1}, {-10, 105}, {-9, 144},
		{-13, 69}, {0, -1}, {-30, 144}, {-23, 130},
		{0, -1}, {-15, 130}, {0, -1}, {0, -1},
		{0, -1}, {-3, 146}, {-19, 127}, {0, -1},
		{0, 135}, {-14, 144}, {0, -1}, {-25, 146},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {-33, 127}, {-26, 144}, {0, -1},
		{0, -1}, {6, 135}, {0, -1}, {-20, 135},
		{0, -1}, {7, 127}, {-31, 135}, {0, -1},
		{0, -1}, {-11, 127}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {41, 0},
		{-28, 0}, {-32, 0}, {32, 0}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {41, 0},
		{0, -1}, {0, -1}, {-22, 0}, {0, -1},
		{1, 0}, {0, -1}, {11, 0}, {0, -1},
		{30, 0}, {-7, 0}, {38, 0}, {0, 0},
		{0, -1}, {0, -1}, {-16, 0}
	};
};

constexpr uint8_t socket_level_map::tail[socket_level_map::tail_size];
constexpr socket_level_map::tail_ref_type socket_level_map::tail_ref[socket_level_map::tail_ref_size];
constexpr socket_level_map::base_ref_type socket_level_map::base_ref[socket_level_map::base_ref_size];

#endif

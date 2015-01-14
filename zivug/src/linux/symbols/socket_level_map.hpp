/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_39D1A37B2CE18FB6D69D321606A73AB8)
#define HPP_39D1A37B2CE18FB6D69D321606A73AB8

struct socket_level_map {
	/*
	 *    1 socket
	 *    2 ip
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

	struct locus {
		locus()
		: offset(-1), tail_pos(0)
		{}

		explicit operator bool() const
		{
			return offset >= 0;
		}

	private:
		friend struct socket_level_map;

		locus(int offset_, int tail_pos_)
		: offset(offset_), tail_pos(tail_pos_)
		{}

		int offset;
		int tail_pos;
	};

	static locus search_root()
	{
		return locus(0, 0);
	}

	template <typename Iterator>
	static locus locate_rel(locus loc, Iterator first, Iterator last)
	{
		int l_pos(loc.offset), n_pos(l_pos);		

		if (base_ref[l_pos].base >= 0) {
			for (; first != last; ++first) {
				if (!is_valid(*first))
					return locus();

				n_pos = base_ref[l_pos].base + *first
					- char_offset;

				if (n_pos >= int(base_ref_size))
					return locus();

				if (int(base_ref[n_pos].check) != l_pos)
					return locus();

				if (int(base_ref[n_pos].base) < 0)
					break;

				l_pos = n_pos;
			}

			if (first == last)
				return locus(n_pos, 0);
			else
				++first;
		}

		
		auto &r(tail_ref[-1 - base_ref[n_pos].base]);
		auto r_first(&tail[r.offset]);
		auto d(std::distance(first, last));

		if (d > (r.size - loc.tail_pos))
			return locus();

		while (first != last) {
			if (*first != r_first[loc.tail_pos])
				return locus();

			++first;
			++loc.tail_pos;
		}
		return locus(n_pos, loc.tail_pos);
	}

	template <typename Iterator>
	static int find_rel(locus loc, Iterator first, Iterator last)
	{
		loc = locate_rel(loc, first, last);
		if (!loc)
			return 0;

		auto b(base_ref[loc.offset].base);
		if (b >= 0) {
			auto n_pos(b + term_char - char_offset);
			if (base_ref[n_pos].check != loc.offset)
				return 0;
			else
				return -base_ref[n_pos].base;
		} else {
			auto &r(tail_ref[-1 - b]);
			if (r.size == loc.tail_pos)
				return -b;
			else
				return 0;
		} 
	}

	template <typename Iterator>
	static int find(Iterator first, Iterator last)
	{
		return find_rel(search_root(), first, last);
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

	constexpr static std::size_t tail_size = 78;

	constexpr static uint8_t tail[tail_size] = {
		0x63, 0x6b, 0x65, 0x74, 0x70, 0x36, 0x6d, 0x70,
		0x76, 0x36, 0x74, 0x70, 0x69, 0x74, 0x65, 0x77,
		0x32, 0x35, 0x6c, 0x6b, 0x6f, 0x6d, 0x73, 0x65,
		0x63, 0x6e, 0x65, 0x74, 0x32, 0x35, 0x63, 0x6b,
		0x65, 0x74, 0x6c, 0x64, 0x61, 0x65, 0x75, 0x69,
		0x6c, 0x63, 0x63, 0x70, 0x69, 0x6e, 0x6b, 0x70,
		0x63, 0x72, 0x70, 0x63, 0x70, 0x6f, 0x6c, 0x32,
		0x74, 0x70, 0x6c, 0x75, 0x65, 0x74, 0x6f, 0x6f,
		0x74, 0x68, 0x70, 0x69, 0x70, 0x65, 0x73, 0x63,
		0x76, 0x61, 0x69, 0x66, 0x67, 0x63
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 34;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 4}, {4, 0}, {4, 1}, {5, 0},
		{5, 1}, {6, 4}, {10, 2}, {12, 3},
		{15, 1}, {16, 0}, {16, 2}, {18, 2},
		{20, 2}, {22, 2}, {24, 4}, {28, 2},
		{30, 4}, {34, 0}, {34, 1}, {35, 2},
		{37, 3}, {40, 2}, {42, 2}, {44, 3},
		{47, 2}, {49, 3}, {52, 6}, {58, 8},
		{66, 4}, {70, 1}, {71, 2}, {73, 3},
		{76, 1}, {77, 1}
	};

	struct base_ref_type {
		int8_t base;
		int8_t check;
	};

	constexpr static std::size_t base_ref_size = 127;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{54, 0}, {0, -1}, {-4, 66}, {0, -1},
		{0, -1}, {0, -1}, {-2, 67}, {0, -1},
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
		{0, -1}, {0, -1}, {-12, 97}, {0, -1},
		{2, 123}, {-7, 121}, {-6, 111}, {0, 116},
		{-34, 116}, {0, -1}, {-23, 106}, {0, -1},
		{-15, 106}, {-8, 66}, {-18, 97}, {0, -1},
		{-3, 122}, {-1, 121}, {1, 52}, {5, 111},
		{24, 55}, {-20, 111}, {-25, 122}, {0, -1},
		{-31, 111}, {0, -1}, {-21, 68}, {-5, 67},
		{0, -1}, {-10, 67}, {-19, 103}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{-24, 68}, {-9, 120}, {0, -1}, {-17, 118},
		{-30, 120}, {-33, 103}, {-13, 68}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {1, 103}, {0, -1}, {-14, 120},
		{-29, 118}, {-11, 103}, {-27, 118}, {29, 0},
		{-28, 0}, {-32, 0}, {7, 0}, {0, -1},
		{-26, 120}, {0, -1}, {0, -1}, {3, 0},
		{0, -1}, {0, -1}, {-22, 0}, {0, -1},
		{2, 0}, {0, -1}, {38, 0}, {0, -1},
		{36, 0}, {2, 0}, {13, 0}, {0, 0},
		{0, -1}, {0, -1}, {-16, 0}
	};
};

constexpr uint8_t socket_level_map::tail[socket_level_map::tail_size];
constexpr socket_level_map::tail_ref_type socket_level_map::tail_ref[socket_level_map::tail_ref_size];
constexpr socket_level_map::base_ref_type socket_level_map::base_ref[socket_level_map::base_ref_size];

#endif

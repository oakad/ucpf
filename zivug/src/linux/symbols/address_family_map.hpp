/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_87740CA376E7B1C939F100DC33CE362A)
#define HPP_87740CA376E7B1C939F100DC33CE362A

struct address_family_map {
	/*
	 *    1 unix
	 *    2 local
	 *    3 inet
	 *    4 ax25
	 *    5 ipx
	 *    6 appletalk
	 *    7 netrom
	 *    8 bridge
	 *    9 atmpvc
	 *   10 x25
	 *   11 inet6
	 *   12 rose
	 *   13 decnet
	 *   14 netbeui
	 *   15 security
	 *   16 key
	 *   17 netlink
	 *   18 route
	 *   19 packet
	 *   20 ash
	 *   21 econet
	 *   22 atmsvc
	 *   23 rds
	 *   24 sna
	 *   25 irda
	 *   26 pppox
	 *   27 wanpipe
	 *   28 llc
	 *   29 ib
	 *   30 can
	 *   31 tipc
	 *   32 bluetooth
	 *   33 iucv
	 *   34 rxrpc
	 *   35 isdn
	 *   36 phonet
	 *   37 ieee802154
	 *   38 caif
	 *   39 alg
	 *   40 nfc
	 *   41 vsock
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
		friend struct address_family_map;

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

	constexpr static int char_offset = 46;
	constexpr static int char_count = 75;
	constexpr static int term_char = 47;

	constexpr static std::size_t tail_size = 110;

	constexpr static uint8_t tail[tail_size] = {
		0x6e, 0x69, 0x78, 0x63, 0x61, 0x6c, 0x32, 0x35,
		0x78, 0x70, 0x6c, 0x65, 0x74, 0x61, 0x6c, 0x6b,
		0x6f, 0x6d, 0x69, 0x64, 0x67, 0x65, 0x76, 0x63,
		0x32, 0x35, 0x65, 0x65, 0x63, 0x6e, 0x65, 0x74,
		0x65, 0x75, 0x69, 0x63, 0x75, 0x72, 0x69, 0x74,
		0x79, 0x65, 0x79, 0x69, 0x6e, 0x6b, 0x74, 0x65,
		0x63, 0x6b, 0x65, 0x74, 0x68, 0x63, 0x6f, 0x6e,
		0x65, 0x74, 0x76, 0x63, 0x73, 0x61, 0x64, 0x61,
		0x70, 0x6f, 0x78, 0x61, 0x6e, 0x70, 0x69, 0x70,
		0x65, 0x63, 0x69, 0x70, 0x63, 0x75, 0x65, 0x74,
		0x6f, 0x6f, 0x74, 0x68, 0x63, 0x76, 0x72, 0x70,
		0x63, 0x64, 0x6e, 0x6f, 0x6e, 0x65, 0x74, 0x65,
		0x65, 0x38, 0x30, 0x32, 0x31, 0x35, 0x34, 0x66,
		0x67, 0x63, 0x73, 0x6f, 0x63, 0x6b
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 41;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 3}, {3, 3}, {6, 0}, {6, 2},
		{8, 1}, {9, 7}, {16, 2}, {18, 4},
		{22, 2}, {24, 2}, {26, 0}, {26, 1},
		{27, 5}, {32, 3}, {35, 6}, {41, 2},
		{43, 3}, {46, 2}, {48, 4}, {52, 1},
		{53, 5}, {58, 2}, {60, 1}, {61, 1},
		{62, 2}, {64, 3}, {67, 6}, {73, 1},
		{74, 0}, {74, 0}, {74, 3}, {77, 7},
		{84, 2}, {86, 3}, {89, 2}, {91, 4},
		{95, 8}, {103, 1}, {104, 1}, {105, 1},
		{106, 4}
	};

	struct base_ref_type {
		int16_t base;
		int16_t check;
	};

	constexpr static std::size_t base_ref_size = 154;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{79, 0}, {-3, 70}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{-11, 70}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, 132},
		{-14, 72}, {0, -1}, {-19, 145}, {0, 122},
		{2, 143}, {-40, 143}, {-15, 148}, {-38, 51},
		{0, -1}, {-36, 145}, {-17, 72}, {9, 115},
		{-30, 51}, {0, -1}, {0, -1}, {-24, 148},
		{-7, 72}, {-26, 145}, {0, 55}, {0, -1},
		{0, 56}, {-23, 147}, {-12, 84}, {-9, 63},
		{-18, 84}, {0, -1}, {-22, 63}, {-28, 141},
		{0, -1}, {0, -1}, {-2, 141}, {0, -1},
		{5, 147}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{-32, 131}, {-34, 147}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {-8, 131}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {-39, 130},
		{0, -1}, {0, -1}, {-29, 138}, {-6, 130},
		{0, -1}, {-37, 138}, {-20, 130}, {0, 130},
		{0, -1}, {0, -1}, {0, -1}, {-4, 130},
		{0, -1}, {0, -1}, {0, 138}, {0, -1},
		{-5, 138}, {0, -1}, {-25, 138}, {-35, 138},
		{0, -1}, {-33, 138}, {45, 0}, {30, 0},
		{0, 0}, {-13, 0}, {-21, 0}, {0, -1},
		{0, -1}, {0, -1}, {58, 0}, {0, -1},
		{-16, 0}, {17, 0}, {0, -1}, {1, 0},
		{0, -1}, {3, 0}, {0, -1}, {19, 0},
		{3, 0}, {-31, 0}, {-1, 0}, {-41, 0},
		{-27, 0}, {-10, 0}
	};
};

constexpr uint8_t address_family_map::tail[address_family_map::tail_size];
constexpr address_family_map::tail_ref_type address_family_map::tail_ref[address_family_map::tail_ref_size];
constexpr address_family_map::base_ref_type address_family_map::base_ref[address_family_map::base_ref_size];

#endif

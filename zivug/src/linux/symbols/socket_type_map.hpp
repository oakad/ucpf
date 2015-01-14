/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_3C750D15FD87935E09DE05F1458F626C)
#define HPP_3C750D15FD87935E09DE05F1458F626C

struct socket_type_map {
	/*
	 *    1 stream
	 *    2 dgram
	 *    3 raw
	 *    4 rdm
	 *    5 seqpacket
	 *    6 dccp
	 *    7 packet
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
		friend struct socket_type_map;

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

	constexpr static int char_offset = 95;
	constexpr static int char_count = 24;
	constexpr static int term_char = 96;

	constexpr static std::size_t tail_size = 23;

	constexpr static uint8_t tail[tail_size] = {
		0x72, 0x65, 0x61, 0x6d, 0x72, 0x61, 0x6d, 0x77,
		0x6d, 0x71, 0x70, 0x61, 0x63, 0x6b, 0x65, 0x74,
		0x63, 0x70, 0x61, 0x63, 0x6b, 0x65, 0x74
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 7;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 4}, {4, 3}, {7, 1}, {8, 1},
		{9, 7}, {16, 2}, {18, 5}
	};

	struct base_ref_type {
		int8_t base;
		int8_t check;
	};

	constexpr static std::size_t base_ref_size = 23;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{0, 0}, {0, -1}, {0, -1}, {-3, 19},
		{-6, 5}, {0, 0}, {-4, 19}, {-5, 20},
		{-2, 5}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {-7, 0}, {0, -1}, {1, 0},
		{1, 0}, {0, -1}, {-1, 20}
	};
};

constexpr uint8_t socket_type_map::tail[socket_type_map::tail_size];
constexpr socket_type_map::tail_ref_type socket_type_map::tail_ref[socket_type_map::tail_ref_size];
constexpr socket_type_map::base_ref_type socket_type_map::base_ref[socket_type_map::base_ref_size];

#endif

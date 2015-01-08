/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F619E36FAFFE1B86F123CAE8AF566A96)
#define HPP_F619E36FAFFE1B86F123CAE8AF566A96

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

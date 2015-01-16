/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_5116ADD47866CA7C193A1477699FAC77)
#define HPP_5116ADD47866CA7C193A1477699FAC77

struct socket_cmd_map {
	/*
	 *    1 bind
	 *    2 option
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

	constexpr static int char_offset = 96;
	constexpr static int char_count = 20;
	constexpr static int term_char = 97;

	constexpr static std::size_t tail_size = 8;

	constexpr static uint8_t tail[tail_size] = {
		0x69, 0x6e, 0x64, 0x70, 0x74, 0x69, 0x6f, 0x6e
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 2;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 3}, {3, 5}
	};

	struct base_ref_type {
		int8_t base;
		int8_t check;
	};

	constexpr static std::size_t base_ref_size = 16;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{0, 0}, {0, -1}, {-1, 0}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {0, -1}, {-2, 0}
	};
};

constexpr uint8_t socket_cmd_map::tail[socket_cmd_map::tail_size];
constexpr socket_cmd_map::tail_ref_type socket_cmd_map::tail_ref[socket_cmd_map::tail_ref_size];
constexpr socket_cmd_map::base_ref_type socket_cmd_map::base_ref[socket_cmd_map::base_ref_size];

#endif

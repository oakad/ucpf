/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_{{guard}})
#define HPP_{{guard}}

struct {{name}} {
{{roster}}

	template <typename Iterator>
	static int find(Iterator &first, Iterator last)
	{
		int base(base_ref[0].first);
		int l_pos(0), n_pos(0);

		for (; first != last; ++first) {
			if (!is_valid(*first))
				return 0;

			n_pos = base + *first - char_offset;

			if (int(base_ref[n_pos].second) != l_pos)
				return 0;
			else if (base_ref[n_pos].first < 0) {
				++first;
				int r_idx(-base_ref[n_pos].first);
				auto &r(tail_ref[r_idx - 1]);

				if (std::equal(
					first, last,
					tail.begin() + r.first,
					tail.begin() + r.first + r.second
				))
					return r_idx;
				else
					return 0;
			} else {
				l_pos = n_pos;
				base = base_vec[n_pos].first;
			}
		}

		n_pos = base + term_char - char_offset;
		if (n_pos < int(base_ref.size())) {
			if ((int(base_ref[n_pos].second) == l_pos)
			    && (base_ref[n_pos].first < 0))
				return -base_ref[n_pos].first;
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

{{tables}}
};

#endif

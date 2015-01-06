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

{{tables}}
};

{{table_defs}}
#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <set>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <zivug/detail/unescape_c.hpp>

struct fixed_map {
	fixed_map(int char_off_, int term_char_)
	: char_off(char_off_), term_char(term_char_)
	{
		base_vec.emplace_back(0, 0);
	}

	bool is_vacant(int pos) const
	{
		return (int(base_vec.size()) <= pos)
		       || (base_vec[pos].second < 0);
	}

	template <typename Iter>
	int append_leaf(Iter first, Iter last, int val)
	{
		tail_ref r{0, std::size_t(std::distance(first, last)), val};
		if (r.tail_sz) {
			r.tail_pos = tail_vec.size();
			for (; first != last; ++first)
				tail_vec.push_back(*first);
		}

		ref_vec.push_back(r);
		return -ref_vec.size();
	}

	template <typename Iter>
	std::size_t common_length(int pos, Iter first, Iter last)
	{
		auto r(ref_vec[-1 - base_vec[pos].first]);
		auto s_ptr(&tail_vec[r.tail_pos]);
		auto s_pos(0);

		while ((s_pos < r.tail_sz) && (first != last)) {
			if (s_ptr[s_pos] != *first)
				break;

			++first;
			++s_pos;
		}
		return s_pos;
	}

	int get_vacant(int first)
	{
		for (; first < base_vec.size(); ++first) {
			if (base_vec[first].second < 0)
				return first;
		}

		base_vec.resize(first + 1, std::make_pair(0, -1));
		return first;
	}

	void relocate(int pos, int leaf_idx)
	{
	}

	void unroll_suffix(
		int pos, std::size_t count, int other_char, int leaf_idx
	)
	{
		auto cur_leaf_idx(base_vec[pos].first);
		auto &cur_leaf(ref_vec[-1 - cur_leaf_idx]);

	}

	void append(std::vector<uint8_t> const &in, int val)
	{
		auto c_base(base_vec[0].first);
		int l_pos(0);
		auto first(in.begin());
		auto last(in.end());

		while (first != last) {
			auto n_pos(c_base + *first - char_off);
			if (is_vacant(n_pos)) {
				get_vacant(n_pos);
				++first;
				base_vec[n_pos] = std::make_pair(
					append_leaf(first, last, val),
					l_pos
				);
				return;
			} else if (base_vec[n_pos].second != l_pos) {
				++first;
				relocate(n_pos, append_leaf(first, last, val));
				return;
			} else if (base_vec[n_pos].first < 0) {
				++first;
				auto n_char(term_char);
				auto c_len(common_length(n_pos, first, last));
				if (std::distance(first, last) == c_len)
					first = last;
				else {
					std::advance(first, c_len);
					n_char = *first;
					++first;
				}

				unroll_suffix(
					n_pos, c_len, n_char,
					append_leaf(first, last, val)
				);
				return;
			} else {
				c_base = base_vec[n_pos].first;
				l_pos = n_pos;
				++first;
			}
		}
	}

	void compact_tails();

	struct tail_ref {
		std::size_t tail_pos;
		std::size_t tail_sz;
		int val;
	};

	std::vector<uint8_t> tail_vec;
	std::vector<tail_ref> ref_vec;
	std::vector<std::pair<int, int>> base_vec;

	int char_off;
	int term_char;
	int base_off;
};

void emit_tails(fixed_map &fm)
{
	printf(
		"\tconstexpr static std::array<uint8_t, %zd> tails = {\n",
		fm.tail_vec.size()
	);

	int c(0);
	{
		int lc(std::min(int(fm.tail_vec.size()), c + 8));
		if (lc) {
			int xc(c);
			printf("\t\t0x%02x", fm.tail_vec[xc]);
			for (; xc < lc; ++xc)
				printf(", 0x%02x", fm.tail_vec[xc]);
		}
		c += lc;

	}

	for (; c < int(fm.tail_vec.size()); c += 8) {
		printf(",\n");
		int lc(std::min(int(fm.tail_vec.size()), c + 8));
		if (!lc)
			continue;

		int xc(c);
		printf("\t\t0x%02x", fm.tail_vec[xc]);
		for (; xc < lc; ++xc)
			printf(", 0x%02x", fm.tail_vec[xc]);
	}

	printf("\n\t};\n");
}

void emit_map(fixed_map &fm)
{
	emit_tails(fm);
}

int main(int argc, char **argv)
{
	using ucpf::zivug::detail::unescape_c;

	std::string s_in;
	std::vector<uint8_t> v_in;
	std::list<decltype(v_in)> l_in;
	std::set<uint8_t> ab;
	std::vector<std::pair<int, int>> v_out;

	while (std::getline(std::cin, s_in)) {
		if (s_in.empty())
			continue;

		printf("in %s\n", s_in.c_str());
		auto s_iter(s_in.begin());
		if (!unescape_c(v_in, s_iter, s_in.end()))
			return -1;

		for (auto c: v_in)
			ab.emplace(c);

		l_in.emplace_back(std::move(v_in));
		v_in.clear();
		s_in.clear();
	}

	int c_low(*ab.begin());
	int c_high(*ab.end());
	int c_term(-1);

	if (c_low > 1) {
		c_term = c_low - 1;
		--c_low;
	} else {
		int c_last(c_low);
		for (auto iter(ab.begin()); iter != ab.end(); ++iter) {
			if ((int(*iter) - c_last) > 1) {
				c_term = c_last + 1;
				break;
			}
			c_last = *iter;
		}
		if (c_term < 0) {
			c_term = c_high + 1;
			++c_high;
		}
	}

	int char_off(c_low > 0 ? c_low : -1);
	fixed_map fm(char_off, c_term);
	int i_cnt(1);

	for (auto const &v: l_in) {
		printf("append %d\n", i_cnt);
		fm.append(v, i_cnt++);
	}

	printf("struct fixed_string_map {\n");
	emit_map(fm);
	printf("};\n");
	return 0;
}

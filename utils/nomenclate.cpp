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
	fixed_map(int char_off_, int char_cnt_, int term_char_)
	: char_off(char_off_), char_cnt(char_cnt_), term_char(term_char_)
	{
		base_vec.emplace_back(0, 0);
	}

	bool is_vacant(int pos) const
	{
		return (int(base_vec.size()) <= pos)
		       || (base_vec[pos].second < 0);
	}

	template <typename Iter>
	int append_leaf(Iter first, Iter last)
	{
		tail_ref r{0, std::size_t(std::distance(first, last))};
		if (r.tail_sz) {
			r.tail_pos = tail_vec.size();
			for (; first != last; ++first)
				tail_vec.push_back(*first);
		}

		ref_vec.push_back(r);
		return -ref_vec.size();
	}

	template <typename Iter>
	int common_length(int pos, Iter first, Iter last)
	{
		auto r(ref_vec[-1 - base_vec[pos].first]);
		auto s_ptr(&tail_vec[r.tail_pos]);
		auto s_pos(0);

		while ((s_pos < int(r.tail_sz)) && (first != last)) {
			if (s_ptr[s_pos] != *first)
				break;

			++first;
			++s_pos;
		}
		return s_pos;
	}

	int get_vacant(int first)
	{
		for (; first < int(base_vec.size()); ++first) {
			if (base_vec[first].second < 0)
				return first;
		}

		base_vec.resize(first + 1, std::make_pair(0, -1));
		return first;
	}

	int get_vacant_set(std::vector<int> const &c_set)
	{
		auto base(get_vacant(c_set[0]) - c_set[0]);
		auto iter(c_set.begin());

		while (true) {
			for (++iter; iter != c_set.end(); ++iter) {
				if (get_vacant(base + *iter)
				    != (base + *iter))
					break;
			}
			if (iter == c_set.end())
				return base;

			iter = c_set.begin();
			base = get_vacant(c_set[0] + base + 1) - c_set[0];
		}
	}

	void update_children(int base, int old_ppos, int new_ppos)
	{
		for (int c(1); c < char_cnt; ++c) {
			if (base_vec[base + c].second == old_ppos)
				base_vec[base + c].second = new_ppos;
		}
	}

	void relocate(int pos, int other_ppos, int leaf_idx)
	{
		auto base(base_vec[other_ppos].first);
		auto n_char(pos - base);

		std::vector<int> c_set;
		int c;

		for (c = 1; c < n_char; ++c) {
			if (base_vec[base + c].second == other_ppos)
				c_set.push_back(c);
		}

		int n_char_pos(c_set.size());
		c_set.push_back(n_char);

		for (c = n_char + 1; c < char_cnt; ++c) {
			if (base_vec[base + c].second == other_ppos)
				c_set.push_back(c);
		}

		auto n_base(get_vacant_set(c_set));

		for (c = 0; c < n_char_pos; ++c) {
			auto l_pos(base + c_set[c]);
			auto n_pos(n_base + c_set[c]);

			base_vec[n_pos] = base_vec[l_pos];

			if (base_vec[l_pos].first >= 0)
				update_children(
					base_vec[l_pos].first,
					l_pos, n_pos
				);

			base_vec[l_pos] = std::make_pair(0, -1);
		}

		base_vec[n_base + c_set[c++]]
		= std::make_pair(leaf_idx, other_ppos);

		for (; c < char_cnt; ++c) {
			auto l_pos(base + c_set[c]);
			auto n_pos(n_base + c_set[c]);

			base_vec[n_pos] = base_vec[l_pos];

			if (base_vec[l_pos].first >= 0)
				update_children(
					base_vec[l_pos].first,
					l_pos, n_pos
				);

			base_vec[l_pos] = std::make_pair(0, -1);
		}
	}

	void unroll_suffix(
		int pos, std::size_t count, int other_char, int leaf_idx
	)
	{
		auto cur_leaf_idx(base_vec[pos].first);
		auto &cur_leaf(ref_vec[-1 - cur_leaf_idx]);

		while (count) {
			auto n_idx(tail_vec[cur_leaf.tail_pos] - char_off);
			auto n_pos(get_vacant(n_idx));
			base_vec[n_pos] = std::make_pair(cur_leaf_idx, pos);
			base_vec[pos].first = n_pos - n_idx;
			++cur_leaf.tail_pos;
			--cur_leaf.tail_sz;
			--count;
			pos = n_pos;
		}

		int min_char(term_char);
		int max_char(other_char);

		if (cur_leaf.tail_sz)
			min_char = tail_vec[cur_leaf.tail_pos];

		if (min_char > max_char)
			std::swap(min_char, max_char);

		min_char -= char_off;
		max_char -= char_off;
		int base(min_char);

		while (true) {
			base = get_vacant(base);
			auto max_pos(base + max_char - min_char);
			if (max_pos == get_vacant(max_pos))
				break;
			else
				++base;
		}

		base -= min_char;
		other_char -= char_off;

		base_vec[base + other_char] = std::make_pair(leaf_idx, pos);
		if (min_char == other_char)
			min_char = max_char;

		base_vec[base + min_char] = std::make_pair(cur_leaf_idx, pos);
		base_vec[pos].first = base;
		++cur_leaf.tail_pos;
		--cur_leaf.tail_sz;
	}

	void append(std::vector<uint8_t> const &in)
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
					append_leaf(first, last),
					l_pos
				);
				return;
			} else if (base_vec[n_pos].second != l_pos) {
				++first;
				relocate(
					n_pos, l_pos, append_leaf(first, last)
				);
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
					append_leaf(first, last)
				);
				return;
			} else {
				c_base = base_vec[n_pos].first;
				l_pos = n_pos;
				++first;
			}
		}
	}

	int find(std::vector<uint8_t> const &in) const
	{
		auto first(in.begin());
		auto last(in.end());
		auto base(base_vec[0].first);
		auto l_pos(0), n_pos(0);

		for (; first != last; ++first) {
			n_pos = base + *first - char_off;
			if (base_vec[n_pos].second != l_pos)
				return 0;
			else if (base_vec[n_pos].first < 0) {
				auto r_idx(-base_vec[n_pos].first);
				auto &r(ref_vec[r_idx - 1]);
				if (std::equal(
					first, last,
					tail_vec.begin() + r.tail_pos,
					tail_vec.end() + r.tail_pos + r.tail_sz
				))
					return r_idx;
				else
					return 0;
			} else {
				l_pos = n_pos;
				base = base_vec[n_pos].first;
			}
		}
		n_pos = base + term_char - char_off;
		if (n_pos < int(base_vec.size())) {
			if ((base_vec[n_pos].second == l_pos)
			    && (base_vec[n_pos].first < 0))
				return -base_vec[n_pos].first;
		}
		return 0;
	}

	void compact_tails()
	{
		decltype(tail_vec) t_vec;

		for (auto &r: ref_vec) {
			auto n_pos(t_vec.size());
			t_vec.insert(
				t_vec.end(), tail_vec.begin() + r.tail_pos,
				tail_vec.begin() + r.tail_pos + r.tail_sz
			);
			r.tail_pos = n_pos;
		}
		tail_vec = std::move(t_vec);
	}

	struct tail_ref {
		std::size_t tail_pos;
		std::size_t tail_sz;
	};

	std::vector<uint8_t> tail_vec;
	std::vector<tail_ref> ref_vec;
	std::vector<std::pair<int, int>> base_vec;

	int char_off;
	int char_cnt;
	int term_char;
};

void emit_tail(fixed_map const &fm)
{
	printf(
		"\tconstexpr static std::array<uint8_t, %zd> tail = {\n",
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

void emit_tail_ref(fixed_map const &fm)
{
	std::string off_type("std::pair<uint32_t, uint32_t>");

	if (fm.tail_vec.size() < 256)
		off_type = "std::pair<uint8_t, uint8_t>";
	else if (fm.tail_vec.size() < 65536)
		off_type = "std::pair<uint16_t, uint16_t>";

	printf("\tconstexpr static std::array<\n");
	printf("\t\t%s, %zd\n", off_type.c_str(), fm.ref_vec.size());
	printf("\t> tail_ref = {");
	int c(0);
	if (!fm.ref_vec.empty()) {
		printf(
			"\n\t\t{%zd, %zd}", fm.ref_vec[c].tail_pos,
			fm.ref_vec[c].tail_sz
		);
		++c;
	}

	for (; c < int(fm.ref_vec.size()); ++c) {
		printf(",\n\t\t{%zd, %zd}", fm.ref_vec[c].tail_pos,
			fm.ref_vec[c].tail_sz
		);
	}
	printf("\n\t};\n");
}

void emit_base_ref(fixed_map const &fm)
{
	std::string off_type("std::pair<int32_t, int32_t>");
	auto m_sz(std::max(fm.ref_vec.size(), fm.base_vec.size()));

	if (m_sz < 128)
		off_type = "std::pair<int8_t, int8_t>";
	else if (m_sz < 32768)
		off_type = "std::pair<int16_t, int16_t>";

	printf("\tconstexpr static std::array<\n");
	printf("\t\t%s, %zd\n", off_type.c_str(), fm.base_vec.size());
	printf("\t> base_ref = {");
	int c(0);
	if (!fm.ref_vec.empty()) {
		printf(
			"\n\t\t{%d, %d}", fm.base_vec[c].first,
			fm.base_vec[c].second
		);
		++c;
	}

	for (; c < int(fm.ref_vec.size()); ++c) {
		printf(",\n\t\t{%d, %d}", fm.base_vec[c].first,
			fm.base_vec[c].second
		);
	}
	printf("\n\t};\n");
}

void emit_map(fixed_map const &fm)
{
	emit_tail(fm);
	printf("\n");
	emit_tail_ref(fm);
	printf("\n");
	emit_base_ref(fm);
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
	fixed_map fm(char_off, c_high - c_low + 1, c_term);

	for (auto const &v: l_in)
		fm.append(v);

	fm.compact_tails();

	int l_cnt(1);
	for (auto &v: l_in) {
		if (fm.find(v) != l_cnt) {
			v.push_back(0);
			fprintf(
				stderr,
				"Validation failed, value %s, cnt %d\n",
				&v.front(), l_cnt
			);
			return -1;
		}
		++l_cnt;
	}

	printf("struct fixed_string_map {\n");
	emit_map(fm);
	printf("};\n");
	return 0;
}

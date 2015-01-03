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
		printf("r --4- c_set %zd\n", c_set.size());
		for (auto xx: c_set) {
			printf("r -x4- %d\n", xx);
		}

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
		printf("r --3- c_cnt %d b %d old %d new %d\n", char_cnt, base, old_ppos, new_ppos);
		for (int c(1); c <= char_cnt; ++c) {
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

		printf("r --0- pos %d, other %d, base %d, n_char %d\n", pos, other_ppos, base, n_char);

		for (c = 1; c < n_char; ++c) {
			if (base_vec[base + c].second == other_ppos)
				c_set.push_back(c);
		}

		int n_char_pos(c_set.size());
		c_set.push_back(n_char);
		auto x_char_cnt(std::min(
			int(base_vec.size()) - base, char_cnt
		));

		for (c = n_char + 1; c < x_char_cnt; ++c) {
			if (base_vec[base + c].second == other_ppos)
				c_set.push_back(c);
		}

		auto n_base(get_vacant_set(c_set));

		for (c = 0; c < n_char_pos; ++c) {
			auto l_pos(base + c_set[c]);
			auto n_pos(n_base + c_set[c]);

			printf("r --1- l_pos %d n_pos %d\n", l_pos, n_pos);
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

		for (; c < int(c_set.size()); ++c) {
			auto l_pos(base + c_set[c]);
			auto n_pos(n_base + c_set[c]);

			printf("r --2- l_pos %d n_pos %d\n", l_pos, n_pos);
			base_vec[n_pos] = base_vec[l_pos];

			if (base_vec[l_pos].first >= 0)
				update_children(
					base_vec[l_pos].first,
					l_pos, n_pos
				);

			base_vec[l_pos] = std::make_pair(0, -1);
		}

		base_vec[other_ppos].first = n_base;
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

		printf("u --2- pos %zd, sz %zd\n", cur_leaf.tail_pos, cur_leaf.tail_sz);
		int min_char(term_char);
		int max_char(other_char);

		if (cur_leaf.tail_sz)
			min_char = tail_vec[cur_leaf.tail_pos];

		if (min_char > max_char)
			std::swap(min_char, max_char);

		printf("u --0- min %c max %c\n", min_char, max_char);
		min_char -= char_off;
		max_char -= char_off;
		int base(min_char);

		printf("u --1- min %d max %d\n", min_char, max_char);
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
		if (cur_leaf.tail_sz) {
			++cur_leaf.tail_pos;
			--cur_leaf.tail_sz;
		}
	}

	void append(std::vector<uint8_t> const &in)
	{
		auto base(base_vec[0].first);
		int l_pos(0);
		auto first(in.begin());
		auto last(in.end());

		printf("a --0- %s\n", std::string(first, last).c_str());
		while (first != last) {
			auto n_pos(base + *first - char_off);
			printf("a --1- %d (%c), base %d, n_pos %d\n", *first, *first, base, n_pos);
			++first;

			if (is_vacant(n_pos)) {
				printf("a --2- leaf\n");
				get_vacant(n_pos);
				base_vec[n_pos] = std::make_pair(
					append_leaf(first, last), l_pos
				);
				return;
			} else if (base_vec[n_pos].second != l_pos) {
				printf("a --3- conflict\n");
				relocate(
					n_pos, l_pos, append_leaf(first, last)
				);
				return;
			} else if (base_vec[n_pos].first < 0) {
				auto n_char(term_char);
				auto c_len(common_length(n_pos, first, last));
				printf("a --4- unroll len %d\n", c_len);
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
				base = base_vec[n_pos].first;
				l_pos = n_pos;
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
			printf("f --0- %d (%c), base %d, n_pos %d\n", *first, *first, base, n_pos);
			if (base_vec[n_pos].second != l_pos) {
				printf("f --1- bad l_pos %d - %d\n", base_vec[n_pos].second, l_pos);
				return 0;
			} else if (base_vec[n_pos].first < 0) {
				printf("f --2- leaf %d\n", base_vec[n_pos].first);
				++first;
				auto r_idx(-base_vec[n_pos].first);
				auto &r(ref_vec[r_idx - 1]);
				std::string x_tmp(
					tail_vec.begin() + r.tail_pos,
					tail_vec.begin() + r.tail_pos + r.tail_sz
				);
				std::string y_tmp(first, last);
				printf("f -a2- %zd, %zd: %s\n", r.tail_pos, r.tail_sz, x_tmp.c_str());
				printf("f -b2- %s\n", y_tmp.c_str());

				if (std::equal(
					first, last,
					tail_vec.begin() + r.tail_pos,
					tail_vec.begin() + r.tail_pos + r.tail_sz
				))
					return r_idx;
				else
					return 0;
			} else {
				printf("f --3- node\n");
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
			printf(
				"c --0- n_pos %zd, pos %zd, sz %zd\n",
				n_pos, r.tail_pos, r.tail_sz
			);
			t_vec.insert(
				t_vec.end(), tail_vec.begin() + r.tail_pos,
				tail_vec.begin() + r.tail_pos + r.tail_sz
			);
			r.tail_pos = n_pos;
		}
		tail_vec = std::move(t_vec);
	}

	void dump() const
	{
		printf("root: %d, %d\n", base_vec[0].first, base_vec[0].second);

		for (std::size_t pos(1); pos < base_vec.size(); ++pos) {
			auto v(base_vec[pos]);

			printf("%zd: %d, %d", pos, v.first, v.second);
			if (v.second < 0) {
				printf("\n");
				continue;
			}

			auto nc(pos - base_vec[v.second].first);
			if (int(nc + char_off) != term_char)
				printf(": %zd (%c)", nc, int(nc + char_off));
			else
				printf(": %zd (term)", nc);

			if (v.first >= 0) {
				printf("\n");
				continue;
			}

			auto r(ref_vec[-1 - v.first]);
			printf(" -> %s\n", std::string(
				tail_vec.begin() + r.tail_pos,
				tail_vec.begin() + r.tail_pos + r.tail_sz
			).c_str());
		}
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

template <typename Seq, typename Printer>
void emit_sep(
	Seq const &s, std::size_t step, Printer &&p
)
{
	std::size_t c(0);
	auto lc(std::min(s.size(), c + step));

	if (lc) {
		auto xc(c);
		printf("\t\t");
		p(s[xc]);
		for (++xc; xc < lc; ++xc) {
			printf(", ");
			p(s[xc]);
		}
	}
	c += lc;

	for (; c < s.size(); c += step) {
		printf(",\n");
		lc = std::min(s.size(), c + step);
		if (!lc)
			break;

		auto xc(c);
		printf("\t\t");
		p(s[xc]);
		for (++xc; xc < lc; ++xc) {
			printf(", ");
			p(s[xc]);
		}
	}
}

void emit_tail(fixed_map const &fm)
{
	printf(
		"\tconstexpr static std::array<uint8_t, %zd> tail = {\n",
		fm.tail_vec.size()
	);

	emit_sep(fm.tail_vec, 8, [](auto v) -> void {
		printf("0x%02x", v);
	});

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
	printf("\t> tail_ref = {\n");

	emit_sep(fm.ref_vec, 4, [](auto v) -> void {
		printf("{%zd, %zd}", v.tail_pos, v.tail_sz);
	});

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
	printf("\t> base_ref = {\n");

	emit_sep(fm.base_vec, 4, [](auto v) -> void {
		printf("{%d, %d}", v.first, v.second);
	});

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
	int c_high(*ab.rbegin());
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

	fixed_map fm(c_low - 1, c_high - c_low + 1, c_term);

	for (auto const &v: l_in) {
		fm.append(v);
		fm.dump();
		printf("====================\n");
	}

	printf("---- after append\n");

	fm.compact_tails();

	printf("---- after compact\n");

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

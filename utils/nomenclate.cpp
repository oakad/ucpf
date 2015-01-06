/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <set>
#include <list>
#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <cinttypes>
#include <unordered_map>
#include <zivug/detail/unescape_c.hpp>

#include <fcntl.h>
#include <unistd.h>

constexpr static struct {
	char const *text;
	std::size_t size;
} outp_template = {
#include "nomenclate_tmpl_p.hpp"
};

template <typename Seq, typename Printer>
void emit_sep(
	int out_fd, Seq const &s, std::size_t step, Printer &&p
)
{
	std::size_t c(0);
	auto lc(std::min(s.size(), c + step));

	if (lc) {
		auto xc(c);
		dprintf(out_fd, "\t\t");
		p(s[xc]);
		for (++xc; xc < lc; ++xc) {
			dprintf(out_fd, ", ");
			p(s[xc]);
		}
	}
	c += lc;

	for (; c < s.size(); c += step) {
		dprintf(out_fd, ",\n");
		lc = std::min(s.size(), c + step);
		if (!lc)
			break;

		auto xc(c);
		dprintf(out_fd, "\t\t");
		p(s[xc]);
		for (++xc; xc < lc; ++xc) {
			dprintf(out_fd, ", ");
			p(s[xc]);
		}
	}
}

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

		while (first != last) {
			auto n_pos(base + *first - char_off);
			++first;

			if (is_vacant(n_pos)) {
				get_vacant(n_pos);
				base_vec[n_pos] = std::make_pair(
					append_leaf(first, last), l_pos
				);
				return;
			} else if (base_vec[n_pos].second != l_pos) {
				relocate(
					n_pos, l_pos, append_leaf(first, last)
				);
				return;
			} else if (base_vec[n_pos].first < 0) {
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

			if (base_vec[n_pos].second != l_pos)
				return 0;
			else if (base_vec[n_pos].first < 0) {
				++first;
				auto r_idx(-base_vec[n_pos].first);
				auto &r(ref_vec[r_idx - 1]);

				if (std::equal(
					first, last,
					tail_vec.begin() + r.tail_pos,
					tail_vec.begin() + r.tail_pos + r.tail_sz
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

	void dump(int out_fd) const
	{
		dprintf(out_fd, "root: %d, %d\n", base_vec[0].first, base_vec[0].second);

		for (std::size_t pos(1); pos < base_vec.size(); ++pos) {
			auto v(base_vec[pos]);

			dprintf(out_fd, "%zd: %d, %d", pos, v.first, v.second);
			if (v.second < 0) {
				dprintf(out_fd, "\n");
				continue;
			}

			auto nc(pos - base_vec[v.second].first);
			if (int(nc + char_off) != term_char)
				dprintf(out_fd, ": %zd (%c)", nc, int(nc + char_off));
			else
				dprintf(out_fd, ": %zd (term)", nc);

			if (v.first >= 0) {
				dprintf(out_fd, "\n");
				continue;
			}

			auto r(ref_vec[-1 - v.first]);
			dprintf(out_fd, " -> %s\n", std::string(
				tail_vec.begin() + r.tail_pos,
				tail_vec.begin() + r.tail_pos + r.tail_sz
			).c_str());
		}
	}

	void emit_tail(int out_fd) const
	{
		dprintf(
			out_fd,
			"\tconstexpr static std::size_t tail_size = %zd;\n\n",
			tail_vec.size()
		);
		dprintf(
			out_fd,
			"\tconstexpr static uint8_t tail[tail_size] = {\n"
		);

		emit_sep(out_fd, tail_vec, 8, [out_fd](auto v) -> void {
			dprintf(out_fd, "0x%02x", v);
		});

		dprintf(out_fd, "\n\t};\n");
	}

	void emit_tail_ref(int out_fd)
	{
		tail_ref_type = "uint32_t";

		if (tail_vec.size() < 256)
			tail_ref_type = "uint8_t";
		else if (tail_vec.size() < 65536)
			tail_ref_type = "uint16_t";

		dprintf(out_fd, "\tstruct tail_ref_type {\n");
		dprintf(out_fd, "\t\t%s offset;\n", tail_ref_type.c_str());
		dprintf(out_fd, "\t\t%s size;\n", tail_ref_type.c_str());
		dprintf(out_fd, "\t};\n\n");
		dprintf(
			out_fd, "\tconstexpr static std::size_t "
			"tail_ref_size = %zd;\n\n", ref_vec.size()
		);
		dprintf(
			out_fd, "\tconstexpr static tail_ref_type "
			"tail_ref[tail_ref_size] = {\n"
		);

		emit_sep(out_fd, ref_vec, 4, [out_fd](auto v) -> void {
			dprintf(out_fd, "{%zd, %zd}", v.tail_pos, v.tail_sz);
		});

		dprintf(out_fd, "\n\t};\n");
	}

	void emit_base_ref(int out_fd)
	{
		base_ref_type = "int32_t";
		auto m_sz(std::max(ref_vec.size(), base_vec.size()));

		if (m_sz < 128)
			base_ref_type = "int8_t";
		else if (m_sz < 32768)
			base_ref_type = "int16_t";

		dprintf(out_fd, "\tstruct base_ref_type {\n");
		dprintf(out_fd, "\t\t%s base;\n", base_ref_type.c_str());
		dprintf(out_fd, "\t\t%s check;\n", base_ref_type.c_str());
		dprintf(out_fd, "\t};\n\n");

		dprintf(
			out_fd, "\tconstexpr static std::size_t "
			"base_ref_size = %zd;\n\n", base_vec.size()
		);
		dprintf(
			out_fd, "\tconstexpr static base_ref_type "
			"base_ref[base_ref_size] = {\n"
		);

		emit_sep(out_fd, base_vec, 4, [out_fd](auto v) -> void {
			dprintf(out_fd, "{%d, %d}", v.first, v.second);
		});

		dprintf(out_fd, "\n\t};");
	}

	void emit_map(int out_fd)
	{
		dprintf(
			out_fd, "\tconstexpr static int char_offset = %d;\n",
			char_off
		);
		dprintf(
			out_fd, "\tconstexpr static int char_count = %d;\n",
			char_cnt
		);
		dprintf(
			out_fd, "\tconstexpr static int term_char = %d;\n\n",
			term_char
		);

		emit_tail(out_fd);
		dprintf(out_fd, "\n");
		emit_tail_ref(out_fd);
		dprintf(out_fd, "\n");
		emit_base_ref(out_fd);
	}

	void emit_map_def(int out_fd, std::string const &r_name)
	{
		dprintf(
			out_fd, "constexpr uint8_t "
			"%s::tail[%s::tail_size];\n",
			r_name.c_str(), r_name.c_str()
		);
		dprintf(
			out_fd, "constexpr %s::tail_ref_type "
			"%s::tail_ref[%s::tail_ref_size];\n",
			r_name.c_str(), r_name.c_str(), r_name.c_str()
		);
		dprintf(
			out_fd, "constexpr %s::base_ref_type "
			"%s::base_ref[%s::base_ref_size];\n",
			r_name.c_str(), r_name.c_str(), r_name.c_str()
		);
	}

	struct tail_ref {
		std::size_t tail_pos;
		std::size_t tail_sz;
	};

	std::vector<uint8_t> tail_vec;
	std::vector<tail_ref> ref_vec;
	std::vector<std::pair<int, int>> base_vec;
	std::string tail_ref_type;
	std::string base_ref_type;

	int char_off;
	int char_cnt;
	int term_char;
};


std::vector<char> gen_random_str128()
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint64_t> dis;

	std::vector<char> rv(32, 0);
	sprintf(rv.data(), "%016" PRIX64, dis(gen));
	sprintf(rv.data() + 16, "%016" PRIX64, dis(gen));
	return rv;
}

int main(int argc, char **argv)
{
	using ucpf::zivug::detail::unescape_c;

	int out_fd(STDOUT_FILENO);
	std::string r_name("fixed_string_map");
	std::string s_in;
	std::vector<uint8_t> v_in;
	std::list<decltype(v_in)> l_in;
	std::set<uint8_t> ab;
	std::vector<std::pair<int, int>> v_out;

	if (argc > 1) {
		r_name.assign(argv[1]);
		auto f_name(r_name + ".hpp");
		out_fd = open(
			f_name.c_str(), O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
		);
		if (out_fd < 0) {
			dprintf(
				STDERR_FILENO, "Error %d creating file %s\n",
				errno, f_name.c_str()
			);
			return -1;

		}
	}

	while (std::getline(std::cin, s_in)) {
		if (s_in.empty())
			continue;

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
	int c_term(0);

	if (c_low > 1) {
		c_term = c_low - 1;
		--c_low;
	} else {
		c_term = c_high + 1;
		++c_high;
	}

	fixed_map fm(c_low - 1, c_high - c_low + 1, c_term);

	for (auto const &v: l_in)
		fm.append(v);

	fm.compact_tails();

	int l_cnt(1);
	for (auto &v: l_in) {
		if (fm.find(v) != l_cnt) {
			v.push_back(0);
			dprintf(
				STDERR_FILENO,
				"Validation failed, value %s, cnt %d\n",
				&v.front(), l_cnt
			);
			return -1;
		}
		++l_cnt;
	}

	std::regex tag_rx(R"(\{\{[[:alnum:]_]+\}\})");

	std::regex_token_iterator<decltype(outp_template.text)> tag_iter(
		outp_template.text, outp_template.text + outp_template.size,
		tag_rx
	), tag_iter_end;

	auto text_pos(outp_template.text);
	auto guard_val(gen_random_str128());

	static const std::unordered_map<
		std::string, std::function<void ()>
	> cmd_map = {
		{"guard", [out_fd, &guard_val]() -> void {
			if (write(out_fd, guard_val.data(), guard_val.size()));
		}},
		{"name", [out_fd, &r_name]() -> void {
			if (write(out_fd, r_name.c_str(), r_name.size()));
		}},
		{"roster", [out_fd, &l_in]() -> void {
			dprintf(out_fd, "\t/*\n");
			int l_cnt(1);
			for (auto const &v: l_in) {
				dprintf(out_fd, "\t * %4d ", l_cnt++);
				if (write(out_fd, v.data(), v.size()));
				dprintf(out_fd, "\n");
			}
			dprintf(out_fd, "\t */");
		}},
		{"tables", [out_fd, &fm]() -> void {
			fm.emit_map(out_fd);
		}},
		{"table_defs", [out_fd, &fm, &r_name]() -> void {
			fm.emit_map_def(out_fd, r_name);
		}}
	};

	while (tag_iter != tag_iter_end) {
		for (; tag_iter != tag_iter_end; ++tag_iter) {
			if (write(out_fd, text_pos, tag_iter->first - text_pos));

			auto cmd_iter(cmd_map.find(std::string(
				tag_iter->first + 2, tag_iter->second - 2
			)));

			if (cmd_iter != cmd_map.end())
				cmd_iter->second();

			text_pos = tag_iter->second;
		}

		tag_iter = decltype(tag_iter)(
			text_pos, outp_template.text + outp_template.size,
			tag_rx
		);
	}

	if (write(
		out_fd, text_pos,
		outp_template.text + outp_template.size - text_pos
	));

	return 0;
}

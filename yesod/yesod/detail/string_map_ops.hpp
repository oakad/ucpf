/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 *
 * Based on algorithm from:
 *
 *      An Efficient Implementation of Trie Structures (Jun-Ichi Aoe,
 *      Katsushi Morimoto, Takashi Sato) in Software - Practice and Experience,
 *      Vol. 22(9), 695 - 721 (September 1992)
 *
 */
#if !defined(UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145)
#define UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145

namespace ucpf { namespace yesod {

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::emplace_at(
	Iterator first_, Iterator last_, Args&&... args
) -> std::pair<reference, bool>
{
	encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));
	auto &a(items.get_allocator());
	auto value_deleter([a](value_pair *p) -> void {
		value_pair::destroy(a, p);
	});

	std::unique_ptr<
		value_pair, decltype(value_deleter)
	> vp(nullptr, value_deleter);

	uintptr_t l_pos(0);
	auto p(items.ptr_at(0));

	while (first != last) {
		auto n_index(index_offset(first));
		auto q(p->pair_at(items, n_index));

		if (!q.first || q.first->is_vacant()) {
			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			reserve_vacant(l_pos, q, vp.get());
			return std::make_pair(
				vp.release()->value, true
			);
		} else if (q.first->parent() != l_pos) {
			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			detangle(p, q, n_index, vp.get());
			return std::make_pair(
				vp.release()->value, true
			);
		} else if (q.first->is_leaf()) {
			auto l_ptr(q.first->leaf_ptr());
			auto c_len(l_ptr->common_length(first, last));

			if (std::distance(first, last) == c_len) {
				if (l_ptr->suffix_length == c_len)
					return std::make_pair(
						l_ptr->value, false
					);

				first = last;
				n_index = terminator_index;
			} else {
				std::advance(first, c_len);
				n_index = index_offset(first);
				++first;
			}

			vp.reset(value_pair::construct(
				a, first, last, std::forward<Args>(args)...
			));
			unroll_suffix(q, c_len, n_index, vp.get());
			return std::make_pair(
				vp.release()->value, true
			);
		} else {
			++first;
			p = q;
		}
	}
}

template <typename CharType, typename ValueType, typename Policy>
std::basic_ostream<
	CharType, typename Policy::char_traits_type
> &string_map<CharType, ValueType, Policy>::dump(
	std::basic_ostream<
		CharType, typename Policy::char_traits_type
	> &os
) const
{
	{
		auto p(items.ptr_at(0));
		os << "r: " << p->base_offset() << ", ";
		os << "freelist: " << p->parent() << '\n';
	}

	items.for_each(
		1, [&os](size_type pos, pair_type const &p) -> bool {
			os << pos << ": ";
			if (p.is_vacant()) {
				os << "vacant (next: " << p.base_offset();
				os << ", prev: " << p.parent() << ")\n";
			} else if (p.is_leaf()) {
				auto l_ptr(p.leaf_ptr());

				os << "(parent: " << p.parent();
				os << ", leaf: " << l_ptr << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset);
				if (n_off < base_index)
					os << "'<eok>' [";
				else {
					auto n_char(offset_char(n_off));
					os << '\'' <<  n_char << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << ") [";
				}

				os << '"';
				auto suffix(l_ptr->suffix());
				for (
					size_type c(0);
					c < l_ptr->suffix_length;
					++c
				)
					os << std::get<1>(
						tup_breadth_map
					).value(suffix[c]);

				os << "\"] -> " << l_ptr->value << '\n';
			} else {
				os << "(parent: " << p.parent();
				os << ", offset: " << p.base_offset() << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset);
				if (n_off < base_index)
					os << "'<eok>'\n";
				else {
					auto n_char(offset_char(n_off));
					os << '\'' <<  n_char << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << ")\n";
				}
			}
			return false;
		}
	);

	return os;
}

#if 0
template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::unroll_key(
	pair_type *p, uintptr_t pos, size_type count, uintptr_t other
) -> std::pair<pair_type *, uintptr_t>
{
	size_type shrink(0);
	auto deleter([&shrink, this](value_pair *v) -> void {
		if (shrink)
			v->shrink_suffix(items.get_allocator(), shrink);
	});

	std::unique_ptr<value_pair, decltype(deleter)> v_ptr(
		p->leaf_ptr(), deleter
	);

	auto suffix(v_ptr->suffix());
	auto k_char(terminator_char);
	if (count < v_ptr->suffix_length)
		k_char = suffix[count] + null_char;

	uintptr_t next_pos(0);
	while (count > shrink) {
		next_pos = char_offset(1, suffix[shrink] + null_char);
		auto xp(items.find_empty_above(vec_offset(next_pos)));
		auto &q(items.emplace_at(
			xp, pair_type::make(v_ptr.get(), pos)
		));
		p->base = adjust_encoded(1, xp - vec_offset(next_pos));
		pos = log_offset(xp);
		++shrink;
		p = &q;
	}

	auto min_char(std::min(k_char, other));
	auto max_char(std::max(k_char, other));
	uintptr_t adj_pos(1);

	while (true) {
		next_pos = char_offset(adj_pos, min_char);
		auto xp(items.find_empty_above(vec_offset(next_pos)));

		adj_pos = adjust_encoded(adj_pos, xp - vec_offset(next_pos));

		if (!items.ptr_at(
			vec_offset(char_offset(adj_pos, max_char))
		))
			break;

		adj_pos  = adjust_encoded(adj_pos, 1);
	}

	items.emplace_at(
		vec_offset(char_offset(adj_pos, k_char)),
		pair_type::make(v_ptr.get(), pos)
	);
	p->base = adj_pos;
	++shrink;

	return std::make_pair(
		&items.emplace_at(
			vec_offset(char_offset(adj_pos, other)),
			pair_type::make(uintptr_t(0), uintptr_t(0))
		), pos
	);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::split_subtree(
	uintptr_t r_pos, uintptr_t l_pos, uintptr_t k_char
)-> std::pair<pair_type *, uintptr_t>
{
	index_entry_set r_set(items.get_allocator());
	index_entry_set l_set(items.get_allocator());
	uintptr_t adj_pos(l_pos > 1 ? items[vec_offset(l_pos)].base : root);

	items.for_each(
		std::min(
			r_pos > 1 ? items[vec_offset(r_pos)].base : root,
			adj_pos
		), [&r_set, &l_set, r_pos, l_pos, this](
			size_type pos, pair_type const &p
		) -> bool {
			if (p.check == r_pos)
				r_set.push_back(std::make_tuple(
					const_cast<pair_type *>(&p), pos,
					pair_ptr_list(items.get_allocator())
				));
			else if (p.check == l_pos)
				l_set.push_back(std::make_tuple(
					const_cast<pair_type *>(&p), pos,
					pair_ptr_list(items.get_allocator())
				));

			return false;
		}	
	);

	if (r_set.size() < (l_set.size() + 1))
		advance_edges(r_pos, r_set, 0);
	else
		adj_pos = advance_edges(l_pos, l_set, k_char);

	return std::make_pair(
		&items[vec_offset(char_offset(adj_pos, k_char))], l_pos
	);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::advance_edges(
	uintptr_t pos, index_entry_set &b_set, uintptr_t k_char
) -> uintptr_t
{
	auto adj_orig(pos > 1 ? items[vec_offset(pos)].base : root);
	auto adj(adj_orig);
	bool fit(true);

	do {
		fit = true;
		adj = adjust_encoded(adj, 1);
		auto iter(b_set.begin());
		auto n_pos(char_offset(
			adj, offset_to_char(std::get<1>(*iter), adj_orig)
		));
		auto xp(vec_offset(n_pos));
		auto p(items.ptr_at(xp));

		if (p && (p->check != pos)) {
			if (!items.for_each(
				xp + 1, [&xp, pos](
					size_type t_pos, pair_type const &t_p
				) -> bool {
					if ((t_pos - xp) > 1) {
						++xp;
						return true;
					}

					xp = t_pos;

					return t_p.check == pos;
				}
			))
				++xp;
		}
		adj = adjust_encoded(adj, xp - vec_offset(n_pos));

		for (++iter; iter != b_set.end(); ++iter) {
			n_pos = char_offset(adj, offset_to_char(
				std::get<1>(*iter), adj_orig
			));
			p = items.ptr_at(vec_offset(n_pos));
			if (p && (p->check != pos)) {
				fit = false;
				break;
			}
		}

		if (k_char && fit) {
			n_pos = char_offset(adj, k_char);
			p = items.ptr_at(vec_offset(n_pos));
			if (p && (p->check != pos))
				fit = false;
		}
	} while (!fit);

	uintptr_t min_base(0);
	/* Touch all target cells to avoid exceptions down the line. */
	for (auto iter(b_set.rbegin()); iter != b_set.rend(); ++iter) {
		auto n_pos(char_offset(
			adj, offset_to_char(std::get<1>(*iter), adj_orig)
		));
		auto p(items.ptr_at(vec_offset(n_pos)));
		if (!p)
			items.emplace_at(vec_offset(n_pos), pair_type::make(
				uintptr_t(0), uintptr_t(0)
			));

		if (!min_base || (min_base > std::get<0>(*iter)->base))
			min_base = std::get<0>(*iter)->base;
	}

	items.for_each(
		min_base > 1 ? vec_offset(min_base) : 0,
		[&b_set](size_type pos, pair_type &p) -> bool {
			auto iter(std::lower_bound(
				b_set.begin(), b_set.end(), p.check,
				[](
					index_entry_type const &idx,
					uintptr_t q
				) -> bool {
					return q <= log_offset(
						std::get<1>(idx)
					);
				}
			));

			if (
				(iter != b_set.end())
				&& (log_offset(std::get<1>(*iter)) == p.check)
			)
				std::get<2>(*iter).push_front(&p);

			return true;
		}
	);

	for (auto iter(b_set.rbegin()); iter != b_set.rend(); ++iter) {
		auto n_pos(char_offset(
			adj, offset_to_char(std::get<1>(*iter), adj_orig)
		));

		for (auto cp: std::get<2>(*iter))
			cp->check = n_pos;

		items.emplace_at(vec_offset(n_pos), *std::get<0>(*iter));
		*std::get<0>(*iter) = pair_type::make(uintptr_t(0), 0);
	}

	if (pos > 1)
		items[vec_offset(pos)].base = adj;
	else
		root = adj;

	return adj;
}
#endif

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc, typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::value_pair::construct(
	Alloc const &a, Iterator first, Iterator last, Args&&... args
) -> value_pair *
{
	typedef allocator::array_helper<char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	auto suffix_length(std::distance(first, last));

	auto s_deleter(
		[&a, suffix_length](char_type *p) -> void {
			ah_c::destroy(a, p, suffix_length, true);
		}
	);

	std::unique_ptr<char_type[], decltype(s_deleter)> s_ptr(
		nullptr, s_deleter
	);

	if (suffix_length > Policy::short_suffix_length)
		s_ptr.reset(ah_c::alloc(a, first, last));

	auto p_deleter([&a](value_pair *p) -> void {
		ah_p::destroy(a, p, 1, true);
	});

	std::unique_ptr<value_pair, decltype(p_deleter)> p_ptr(
		ah_p::alloc_n(a, 1, std::forward<Args>(args)...), p_deleter
	);

	if (s_ptr)
		p_ptr->long_suffix.data = s_ptr.get();
	else
		ah_c::make(a, p_ptr->short_suffix, first, last);

	p_ptr->suffix_length = suffix_length;
	s_ptr.release();
	return p_ptr.release();
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::destroy(
	Alloc const &a, value_pair *p
)
{
	typedef allocator::array_helper<char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	auto s(p->suffix());

	ah_c::destroy(a, s, p->suffix_length, false);

	if (p->suffix_length > Policy::short_suffix_length)
		ah_c::deallocate(
			a, p->long_suffix.data,
			p->suffix_length + p->long_suffix.offset
		);

	ah_p::destroy(a, p, 1, true);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::shrink_suffix(
	Alloc const &a, size_type count
)
{
	typedef allocator::array_helper<char_type, Alloc> ah;

	int mode(suffix_length > Policy::short_suffix_length ? 2 : 0);

	if (count >= suffix_length) {
		if (mode) {
			ah::destroy(
				a, long_suffix.data + long_suffix.offset,
				suffix_length, false
			);
			ah::free(
				a, long_suffix.data,
				long_suffix.offset + suffix_length
			);
		}
		suffix_length = 0;
		return;
	}
	auto next_length(suffix_length - count);
	mode |= next_length > Policy::short_suffix_length ? 1 : 0;

	switch (mode) {
	case 0: { // short to short
		std::move(
			&short_suffix[count], &short_suffix[suffix_length],
			&short_suffix[0]
		);
		ah::destroy(
			a, short_suffix + suffix_length - count,
			suffix_length - count, false
		);
		suffix_length = next_length;
		break;
	}
	case 2: { // long to short
		auto data(long_suffix.data);
		auto offset(long_suffix.offset);

		ah::make(
			a, short_suffix, data + offset + count,
			data + offset + count + next_length
		);
		ah::destroy(a, data + offset, suffix_length, false);
		ah::free(a, data, offset + suffix_length);
		suffix_length = next_length;
		break;
	}
	case 3: { // long to long
		ah::destroy(
			a, long_suffix.data + long_suffix.offset, count, false
		);
		long_suffix.offset += count;
		suffix_length = next_length;

		if (long_suffix.offset > suffix_length) {
			auto s_ptr(ah::alloc(
				a, long_suffix.data + long_suffix.offset,
				long_suffix.data + long_suffix.offset
				+ suffix_length
			));
			ah::destroy(
				a, long_suffix.data + long_suffix.offset,
				suffix_length, false
			);
			std::swap(s_ptr, long_suffix.data);
			ah::free(
				a, s_ptr, long_suffix.offset + suffix_length
			);
			long_suffix.offset = 0;
		}

		break;
	}
	}
}



}}
#endif

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
	Iterator first, Iterator last, Args&&... args
) -> std::pair<reference, bool>
{
	value_pair *rv(nullptr);
	uintptr_t l_pos(1);
	uintptr_t adj_pos(trie_root);
	bool inserted(true);

	do {
		auto n_pos(char_offset(adj_pos, deref_char(first)));
		auto &p(trie.at(vec_offset(n_pos)));
		auto n_char(deref_char(first));
		++first;
		if (!p.check) {
			rv = value_pair::construct(
				trie.get_allocator(), first, last,
				std::forward<Args>(args)...
			);
			p = pair_type::make(rv, l_pos);
			break;
		} else if (p.is_leaf()) {
			rv = p.leaf_ptr();
			auto c_len(rv->common_length(first, last));

			if ((last - first) == c_len) {
				if (rv->suffix_length == c_len) {
					inserted = false;
					break;
				}
				first = last;
				n_char = terminator_char;
			} else {
				first += c_len;
				n_char = deref_char(first);
				++first;
			}

			auto loc(unroll_key(&p, n_pos, c_len, n_char));
			rv = value_pair::construct(
				trie.get_allocator(), first, last,
				std::forward<Args>(args)...
			);
			*loc.first = pair_type::make(rv, loc.second);
			break;
		} else {
			if (p.check == l_pos) {
				adj_pos = p.base;
				l_pos = n_pos;
				continue;
			} else {
				auto loc(split_subtree(p.check, l_pos, n_char));
				rv = value_pair::construct(
					trie.get_allocator(), first, last,
					std::forward<Args>(args)...
				);
				*loc.first = pair_type::make(rv, loc.second);
				break;
			}
		}
	} while (first != last);

	return std::pair<reference, bool>(rv->value, inserted);
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
	os << "r: " << trie_root << '\n';
	trie.for_each_above(
		0, [&os](size_type pos, pair_type const &p) -> bool {
			os << log_offset(pos) << " (" << pos << "): ";
			if (!p.is_leaf()) {
				os << p.base << " (";
				if (p.base > 1)
					os << vec_offset(p.base);
				else
					os << 'r';

				os << "), ";
				os << p.check << " (";
				if (p.check > 1)
					os << vec_offset(p.check);
				else
					os << (p.check ? 'r' : 'n');

				os << ")\n";
			} else {
				auto vp(reinterpret_cast<value_pair const *>(
					p.base
				));
				os << vp << ", " << p.check << " (";

				if (p.check > 1)
					os << vec_offset(p.check);
				else
					os << (p.check ? 'r' : 'n');

				os << ')';

				if (vp) {
					os << " -> \"";
					auto sp(vp->suffix());
					for (
						size_type c(0);
						c < vp->suffix_length;
						++c
					) {
						if (sp[c])
							os << sp[c];
						else
							os << "\\0";
					}
					os << "\" <" << vp->value << '>';
				}
				os << '\n';
			}
			return true;
		}
	);

	return os;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::find_impl(
	Iterator &first, Iterator const &last
) const -> std::tuple<uintptr_t, uintptr_t, uintptr_t>
{
	uintptr_t l_pos(1);
	uintptr_t adj_pos(trie_root);
	do {
		auto n_pos(char_offset(adj_pos, deref_char(first)));
		auto p(trie.ptr_at(vec_offset(n_pos)));
		++first;

		if (!p || !p->base || (p->check != l_pos))
			return std::make_tuple(0, 0, 0);

		if (p->is_leaf())
			return std::tie(p->base, p->check, n_pos);

		adj_pos = p->base;
		l_pos = n_pos;
	} while (first != last);

	/* Check for virtual key terminator (will appear if one key is a
	 * substring of another).
	 */
	auto n_pos(char_offset(adj_pos, terminator_char));
	auto p(trie.ptr_at(vec_offset(n_pos)));
	if (p && p->base && (p->check == l_pos) && p->is_leaf())
		return std::tie(p->base, p->check, n_pos);

	return std::make_tuple(0, 0, 0);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::unroll_key(
	pair_type *p, uintptr_t pos, size_type count, index_char_type other
) -> std::pair<pair_type *, uintptr_t>
{
	size_type shrink(0);
	std::unique_ptr<value_pair, std::function<void (value_pair *)>> v_ptr(
		p->leaf_ptr(), [&shrink, this](value_pair *v) -> void {
			if (shrink)
				v->shrink_suffix(trie.get_allocator(), shrink);
		}
	);

	auto suffix(v_ptr->suffix());
	auto k_char(terminator_char);
	if ((count + 1) < v_ptr->suffix_length)
		k_char = suffix[count];

	uintptr_t next_pos(0);
	while (count > shrink) {
		next_pos = char_offset(1, suffix[shrink]);
		auto xp(trie.find_empty_above(vec_offset(next_pos)));
		auto &q(trie.emplace_at(
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
		auto xp(trie.find_empty_above(vec_offset(next_pos)));

		adj_pos = adjust_encoded(adj_pos, xp - vec_offset(next_pos));

		if (!trie.ptr_at(
			vec_offset(char_offset(adj_pos, max_char))
		))
			break;

		adj_pos  = adjust_encoded(adj_pos, 1);
	}

	trie.emplace_at(
		vec_offset(char_offset(adj_pos, k_char)),
		pair_type::make(v_ptr.get(), pos)
	);
	p->base = adj_pos;
	++shrink;

	return std::make_pair(
		&trie.emplace_at(
			vec_offset(char_offset(adj_pos, other)),
			pair_type::make(uintptr_t(0), uintptr_t(0))
		), pos
	);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::split_subtree(
	uintptr_t r_pos, uintptr_t l_pos, index_char_type k_char
)-> std::pair<pair_type *, uintptr_t>
{
	std::vector<index_entry_type> r_set, l_set;
	uintptr_t adj_pos(l_pos > 1 ? trie[vec_offset(l_pos)].base : trie_root);

	trie.for_each_above(
		std::min(
			r_pos > 1 ? trie[vec_offset(r_pos)].base : trie_root,
			adj_pos
		), [&r_set, &l_set, r_pos, l_pos](
			size_type pos, pair_type const &p
		) -> bool {
			if (p.check == r_pos)
				r_set.push_back(std::make_tuple(
					const_cast<pair_type *>(&p), pos,
					std::forward_list<pair_type *>()
				));
			else if (p.check == l_pos)
				l_set.push_back(std::make_tuple(
					const_cast<pair_type *>(&p), pos,
					std::forward_list<pair_type *>()
				));

			return true;
		}	
	);

	if (r_set.size() < (l_set.size() + 1))
		advance_edges(r_pos, r_set, 0);
	else
		adj_pos = advance_edges(l_pos, l_set, k_char);

	return std::make_pair(
		&trie[vec_offset(char_offset(adj_pos, k_char))], l_pos
	);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::advance_edges(
	uintptr_t pos,
	std::vector<index_entry_type> &b_set,
	index_char_type k_char
) -> uintptr_t
{
	auto adj_orig(pos > 1 ? trie[vec_offset(pos)].base : trie_root);
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
		auto p(trie.ptr_at(xp));

		if (p && (p->check != pos)) {
			if (trie.for_each_above(
				xp + 1, [&xp, pos](
					size_type t_pos, pair_type const &t_p
				) -> bool {
					if ((t_pos - xp) > 1) {
						++xp;
						return false;
					}

					xp = t_pos;

					return t_p.check != pos;
				}
			))
				++xp;
		}
		adj = adjust_encoded(adj, xp - vec_offset(n_pos));

		for (++iter; iter != b_set.end(); ++iter) {
			n_pos = char_offset(adj, offset_to_char(
				std::get<1>(*iter), adj_orig
			));
			p = trie.ptr_at(vec_offset(n_pos));
			if (p && (p->check != pos)) {
				fit = false;
				break;
			}
		}

		if (k_char && fit) {
			n_pos = char_offset(adj, k_char);
			p = trie.ptr_at(vec_offset(n_pos));
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
		auto p(trie.ptr_at(vec_offset(n_pos)));
		if (!p)
			trie.emplace_at(vec_offset(n_pos), pair_type::make(
				uintptr_t(0), uintptr_t(0)
			));

		if (!min_base || (min_base > std::get<0>(*iter)->base))
			min_base = std::get<0>(*iter)->base;
	}

	trie.for_each_above(
		min_base > 1 ? vec_offset(min_base) : 0,
		[&b_set](size_type pos, pair_type &p) -> bool {
			auto iter(std::lower_bound(
				b_set.begin(), b_set.end(), p.check,
				[](
					index_entry_type const &idx,
					uintptr_t q
				) -> bool {
					return q > log_offset(
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

		trie.emplace_at(vec_offset(n_pos), *std::get<0>(*iter));
		*std::get<0>(*iter) = pair_type::make(uintptr_t(0), 0);
	}

	if (pos > 1)
		trie[vec_offset(pos)].base = adj;
	else
		trie_root = adj;

	return adj;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc, typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::value_pair::construct(
	Alloc const &a_, Iterator first, Iterator last, Args&&... args
) -> value_pair *
{
	char_allocator_type ca(a_);
	auto suffix_length(last - first);
	decltype(suffix_length) init_length(0);

	std::unique_ptr<char_type[], std::function<void (char_type *)>> s_ptr(
		nullptr, [
			&ca, &init_length, suffix_length
		](char_type *p) -> void {
			for (; init_length > 0; --init_length)
				char_allocator_traits::destroy(
					ca, &p[init_length - 1]
				);

			char_allocator_traits::deallocate(
				ca, p, suffix_length
			);
		}
	);

	if (suffix_length > Policy::short_suffix_length) {
		s_ptr.reset(char_allocator_traits::allocate(
			ca, suffix_length
		));

		for (; init_length < suffix_length; ++init_length) {
			char_allocator_traits::construct(
				ca, &s_ptr[init_length], *first
			);
			++first;
		}
	}

	pair_allocator_type pa(a_);
	std::unique_ptr<value_pair, std::function<void (value_pair *)>> p_ptr(
		pair_allocator_traits::allocate(pa, 1),
		[&pa](value_pair *p) -> void {
			pair_allocator_traits::deallocate(pa, p, 1);
		}
	);

	pair_allocator_traits::construct(
		pa, p_ptr.get(), std::forward<Args>(args)...
	);
	if (s_ptr)
		p_ptr->long_suffix.data = s_ptr.release();
	else {
		decltype(s_ptr) ss_ptr(
			p_ptr->suffix(), [
				&ca, &init_length
			](char_type *p) -> void {
				for (; init_length > 0; --init_length)
					char_allocator_traits::destroy(
						ca, &p[init_length - 1]
					);
			}
		);

		for (; init_length < suffix_length; ++init_length) {
			char_allocator_traits::construct(
				ca, &ss_ptr[init_length], *first
			);
			++first;
		}
		ss_ptr.release();
	}

	p_ptr->suffix_length = suffix_length;
	return p_ptr.release();
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::destroy(
	Alloc const &a_, value_pair *p
)
{
	char_allocator_type ca(a_);
	auto s(p->suffix());

	for (auto cnt(p->suffix_length); cnt > 0; --cnt)
		char_allocator_traits::destroy(ca, &s[cnt - 1]);

	if (p->suffix_length > Policy::short_suffix_length)
		char_allocator_traits::deallocate(
			ca, p->long_suffix.data,
			p->suffix_length + p->long_suffix.offset
		);

	pair_allocator_type pa(a_);
	pair_allocator_traits::destroy(pa, p);
	pair_allocator_traits::deallocate(pa, p, 1);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::shrink_suffix(
	Alloc const &a_, size_type count
)
{
	int mode(suffix_length > Policy::short_suffix_length ? 2 : 0);
	mode |= (suffix_length - count) > Policy::short_suffix_length
		? 1 : 0;
	char_allocator_type ca(a_);

	switch (mode) {
	case 0: { // short to short
		std::move(
			&short_suffix[count], &short_suffix[suffix_length],
			&short_suffix[0]
		);
		for (auto c(suffix_length - count); c < suffix_length; ++c)
			char_allocator_traits::destroy(
				ca, &short_suffix[c]
			);
		break;
	}
	case 2: { // long to short
		auto data(long_suffix.data);
		auto offset(long_suffix.offset);

		for (size_type c(0); c < (suffix_length - count); ++c)
			char_allocator_traits::construct(
				ca, &short_suffix[c],
				std::move(data[offset + count + c])
			);

		while (true) {
			auto c(suffix_length - 1);
			char_allocator_traits::destroy(
				ca, &data[offset + c]
			);
			if (!c)
				break;
			--c;
		}

		char_allocator_traits::deallocate(
			ca, data, offset + suffix_length
		);
		break;
	}
	case 3: { // long to long
		auto next_offset(long_suffix.offset + count);
		for (; long_suffix.offset < next_offset; ++long_suffix.offset)
			char_allocator_traits::destroy(
				ca, long_suffix.data + long_suffix.offset
			);
		break;
	}
	}

	suffix_length -= count;
}

}}
#endif

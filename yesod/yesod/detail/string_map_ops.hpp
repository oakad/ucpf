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
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::find_rel(
	locus base, Iterator first_, Iterator last_
) const -> const_pointer
{
	c_encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));

	auto p(items.ptr_at(base.offset));
	if (p->is_leaf()) {
		auto l_ptr(p->leaf_ptr());
		size_type d(std::distance(first, last));

		if (
			(l_ptr->common_length(first, last, base.leaf_pos) == d)
			&& ((base.leaf_pos + d) == l_ptr->suffix_length)
		)
			return &l_ptr->value;
		else
			return nullptr;
	}

	auto q(find_impl(first, last, base.offset));
	if (!q.first)
		return nullptr;

	if (!q.first->is_leaf()) {
		q = q.first->child_at(
			items, q.second, terminator_index
		);
		if (!q.first)
			return nullptr;
	}

	auto l_ptr(q.first->leaf_ptr());
	if (l_ptr->match(first, last))
		return &l_ptr->value;

	return nullptr;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::locate_rel(
	locus base, Iterator first_, Iterator last_
) const -> locus
{
	c_encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));

	auto p(items.ptr_at(base.offset));
	if (p->is_leaf()) {
		auto l_ptr(p->leaf_ptr());
		auto d(std::distance(first, last));

		if (l_ptr->common_length(first, last, base.leaf_pos) == d)
			return locus(base.offset, base.leaf_pos + d);
		else
			return locus();
	}

	auto q(find_impl(first, last, base.offset));
	if (!q.first)
		return locus();

	auto d(std::distance(first, last));
	if (!d)
		return locus(q.second, 0);

	if (q.first->is_leaf()) {
		auto l_ptr(q->leaf_ptr());
		if (l_ptr->common_length(first, last, 0) == d)
			return locus{q.second, d};
	}

	return locus();
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::emplace(
	Iterator first_, Iterator last_, Args&&... args
) -> std::pair<reference, bool>
{
	encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));
	auto a(items.get_allocator());
	auto value_deleter([a](value_pair *p) -> void {
		value_pair::destroy(a, p);
	});

	std::unique_ptr<
		value_pair, decltype(value_deleter)
	> vp(nullptr, value_deleter);

	pair_loc p(items.ptr_at(0), 0);

	while (first != last) {
		auto n_index(index_offset(*first));
		auto q(p.first->pair_at(items, n_index));

		if (!q.first || q.first->is_vacant()) {
			if (items.ptr_at(0)->parent() <= q.second)
				grow_storage(q.second);

			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			reserve_vacant(p.second, q.second, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else if (q.first->parent() != p.second) {
			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			relocate(p, n_index, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else if (q.first->is_leaf()) {
			++first;
			auto l_ptr(q.first->leaf_ptr());
			auto c_len(l_ptr->common_length(first, last, 0));

			if (size_type(std::distance(first, last)) == c_len) {
				if (l_ptr->suffix_length == c_len)
					return std::pair<reference, bool>(
						l_ptr->value, false
					);

				first = last;
				n_index = terminator_index;
			} else {
				std::advance(first, c_len);
				n_index = index_offset(*first);
				++first;
			}

			vp.reset(value_pair::construct(
				a, first, last, std::forward<Args>(args)...
			));
			unroll_suffix(q, c_len, n_index, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else {
			++first;
			p = q;
		}
	}

	auto q(p.first->pair_at(items, terminator_index));
	if (!q.first || q.first->is_vacant()) {
		if (items.ptr_at(0)->parent() <= q.second)
			grow_storage(q.second);

		vp.reset(value_pair::construct(
			a, last, last, std::forward<Args>(args)...
		));
		reserve_vacant(p.second, q.second, vp.get());
		return std::pair<reference, bool>(
			vp.release()->value, true
		);
	} else if (q.first->parent() != p.second) {
		vp.reset(value_pair::construct(
			a, last, last, std::forward<Args>(args)...
		));
		relocate(p, terminator_index, vp.get());
		return std::pair<reference, bool>(
			vp.release()->value, true
		);
	} else
		return std::pair<reference, bool>(
			q.first->leaf_ptr()->value, false
		);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::erase(
	Iterator first_, Iterator last_
) -> size_type
{
	c_encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));

	auto p(find_impl(first, last, 0));
	if (!p.first)
		return 0;

	if (!p.first->is_leaf()) {
		p = p.first->child_at(items, p.second, terminator_index);
		if (!p.first)
			return 0;
	}

	auto l_ptr(p.first->leaf_ptr());
	if (!l_ptr->match(first, last))
		return 0;

	auto p_pos(p.first->parent());
	value_pair::destroy(items.get_allocator(), l_ptr);
	release_pair(p.second);
	release_ancestors(p_pos);

	return 1;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::erase_prefix(
	Iterator first_, Iterator last_
) -> size_type
{
	c_encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));
	auto a(items.get_allocator());
	auto const breadth(index_offset(std::get<0>(tup_breadth_map)));
	auto p(find_impl(first, last, 0));
	if (!p.first)
		return 0;

	if ((first != last) || p.first->is_leaf()) {
		if (!p.first->is_leaf())
			return 0;

		auto l_ptr(p.first->leaf_ptr());
		if (l_ptr->common_length(first, last, 0) < std::distance(
			first, last
		))
			return 0;

		auto p_pos(p.first->parent());
		value_pair::destroy(a, l_ptr);
		release_pair(p.second);
		release_ancestors(p_pos);
		return 1;
	}

	auto p_pos(p.first->parent());
	std::vector<iter_loc, allocator_type> tree_loc({iter_loc(p)}, a);
	size_type rv(0);

	while (true) {
		auto q(tree_loc.back().next_child(items, breadth));

		if (!q.first) {
			release_pair(tree_loc.back().position());
			tree_loc.pop_back();
			if (tree_loc.empty())
				break;

			++tree_loc.back().index;
			continue;
		}

		if (q.first->is_leaf()) {
			value_pair::destroy(a, q.first->leaf_ptr());
			release_pair(q.second);
			++rv;
			++tree_loc.back().index;
			continue;
		}

		tree_loc.emplace_back(q);
	}

	release_ancestors(p_pos);
	return rv;
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

	auto print_char = [](auto &os, auto c) {
		if (std::isprint(c))
			os << c;
		else {
			char hc[] = {
				'\\', 'x', char((c >> 4) & 0xf), char(c & 0xf), 0
			};
			hc[2] += hc[2] > 9 ? 87 : 48;
			hc[3] += hc[3] > 9 ? 87 : 48;
			os << hc;
		}
	};

	items.for_each(
		1, [&os, &print_char, this](size_type pos, pair_type const &p) -> bool {
			os << pos << ": ";
			if (p.is_vacant()) {
				os << "vacant (next: " << p.base_offset();
				os << ", prev: " << p.parent() << ")\n";
			} else if (p.is_leaf()) {
				auto l_ptr(p.leaf_ptr());

				os << "(parent: " << p.parent();
				os << ", leaf: " << l_ptr << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset());
				if (n_off < base_index)
					os << "'<eok>' [";
				else {
					os << '\'';
					auto n_char(offset_char(n_off));
					print_char(os, n_char);
					os << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << " | " << n_off << ") [";
				}

				os << '"';
				auto suffix(l_ptr->suffix());
				for (
					size_type c(0);
					c < l_ptr->suffix_length;
					++c
				)
					print_char(os, std::get<1>(
						tup_breadth_map
					).value(suffix[c]));

				os << "\"] -> " << l_ptr->value << '\n';
			} else {
				os << "(parent: " << p.parent();
				os << ", offset: " << p.base_offset() << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset());
				if (n_off < base_index)
					os << "'<eok>'\n";
				else {
					auto n_char(offset_char(n_off));
					os << '\'';
					print_char(os, n_char);
					os << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << " | " << n_off << ")\n";
				}
			}
			return false;
		}
	);

	return os;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename ValueRefType, typename PairType, typename Pred>
bool string_map<CharType, ValueType, Policy>::for_each_impl(
	PairType first, key_string_type &prefix, Pred &&pred
) const
{
	std::vector<iter_loc, allocator_type> tree_loc(
		{iter_loc(first)}, items.get_allocator()
	);
	auto const breadth(index_offset(std::get<0>(tup_breadth_map)));
	c_decoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);

	while (true) {
		auto p(tree_loc.back().next_child(items, breadth));
		auto c_index(tree_loc.back().index);

		if (!p.first) {
			tree_loc.pop_back();
			if (tree_loc.empty())
				return false;

			prefix.pop_back();
			++tree_loc.back().index;
			continue;
		}

		if (p.first->is_leaf()) {
			if (c_index > terminator_index)
				prefix.push_back(offset_char(c_index));

			auto l_ptr(p.first->leaf_ptr());
			auto sfx(l_ptr->suffix());
			auto first(iterator::make_transform(sfx, map));
			auto last(iterator::make_transform(
				sfx + l_ptr->suffix_length, map
			));
			auto sfx_iter(prefix.insert(
				prefix.end(), first, last
			));

			if (pred(
				&prefix.front(), &prefix.back() + 1,
				const_cast<ValueRefType>(l_ptr->value)
			))
				return true;

			if (c_index > terminator_index)
				--sfx_iter;

			prefix.erase(sfx_iter, prefix.end());
			++tree_loc.back().index;
			continue;
		}

		prefix.push_back(offset_char(c_index));
		tree_loc.emplace_back(p);
	}
}

template <typename CharType, typename ValueType, typename Policy>
template <typename ValueRefType, typename Iterator, typename Pred>
bool string_map<CharType, ValueType, Policy>::for_each_prefix(
	Iterator first_, Iterator last_, Pred &&pred
) const
{
	c_encoding_adapter<decltype(tup_breadth_map)> e_map(tup_breadth_map);
	c_decoding_adapter<decltype(tup_breadth_map)> d_map(tup_breadth_map);
	auto first(iterator::make_transform(first_, e_map));
	auto last(iterator::make_transform(last_, e_map));
	key_string_type prefix(items.get_allocator());

	c_pair_loc prefix_pos(items.ptr_at(0), 0);

	while (first != last) {
		prefix_pos = prefix_pos.first->child_at(
			items, prefix_pos.second, index_offset(*first)
		);

		if (!prefix_pos.first)
			return false;

		prefix.push_back(*first.base());
		++first;

		if (prefix_pos.first->is_leaf()) {
			auto l_ptr(prefix_pos.first->leaf_ptr());
			if (l_ptr->common_length(
				first, last, 0
			) < size_type(std::distance(first, last)))
				return false;

			auto sfx(l_ptr->suffix());
			auto s_first(iterator::make_transform(sfx, d_map));
			auto s_last(iterator::make_transform(
				sfx + l_ptr->suffix_length, d_map
			));
			prefix.insert(prefix.end(), s_first, s_last);

			return pred(
				&prefix.front(), &prefix.back() + 1,
				const_cast<ValueRefType>(l_ptr->value)
			);
		}
	}

	return for_each_impl<ValueRefType>(
		prefix_pos, prefix, std::forward<Pred>(pred)
	);
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::grow_storage(uintptr_t pos)
{
	auto r(items.ptr_at(0));
	auto p(items.ptr_at(r->parent()));
	auto q(items.ptr_at(p->base_offset()));

	items.for_each_pos(
		r->parent() + 1, pos + 2,
		[&r, &p, &q](auto pos_, auto &item) -> void {
			item = pair_type::make_vacant(
				p->base_offset(), q->parent()
			);
			p->set_base_offset(pos_);
			q->set_parent(pos_, false);
			r->set_parent(pos_, true);
			p = &item;
		}
	);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::find_vacant(
	uintptr_t first
) -> uintptr_t
{
	auto r(items.ptr_at(0));
	auto p_pos(r->parent());

	if (first >= p_pos) {
		grow_storage(first);
		return first;
	}

	if (items.ptr_at(first)->is_vacant())
		return first;

	auto p(items.ptr_at(p_pos));
	auto q_pos(p->base_offset());
	while (true) {
		if (q_pos > first) {
			p_pos = q_pos;
			break;
		}

		if (p_pos < first) {
			p_pos = items.ptr_at(p_pos)->base_offset();
			break;
		}

		q_pos = items.ptr_at(q_pos)->base_offset();
		p_pos = items.ptr_at(p_pos)->parent();
	}

	if (p_pos == r->parent())
		grow_storage(p_pos);

	return p_pos;
}

template <typename CharType, typename ValueType, typename Policy>
uintptr_t string_map<CharType, ValueType, Policy>::find_vacant_set(
	uintptr_t first, uintptr_t const *index_set, size_type index_count
)
{
	auto s_pos(find_vacant(first));

	while (true) {
		auto base_diff(s_pos - first);
		auto max_pos(items.ptr_at(0)->parent());
		bool found(true);

		for (size_type c(0); c < index_count; ++c) {
			auto n_pos(index_set[c] + base_diff);
			if (n_pos >= max_pos) {
				grow_storage(
					index_set[index_count - 1] + base_diff
				);
				return s_pos;
			}

			found = items.ptr_at(n_pos)->is_vacant();
			if (!found)
				break;
		}

		if (found)
			return s_pos;

		s_pos = items.ptr_at(s_pos)->base_offset();
		if (s_pos == max_pos) {
			base_diff = s_pos - first;
			grow_storage(index_set[index_count - 1] + base_diff);
			return s_pos;
		}
	}
}

template <typename CharType, typename ValueType, typename Policy>
uintptr_t string_map<CharType, ValueType, Policy>::reserve_vacant(
	uintptr_t parent_pos, uintptr_t child_pos, value_pair *v
)
{
	auto p(items.ptr_at(child_pos));
	auto rv(p->base_offset());
	auto pp(items.ptr_at(p->parent()));
	auto pq(items.ptr_at(p->base_offset()));
	pp->set_base_offset(p->base_offset());
	pq->set_parent(p->parent(), false);
	*p = pair_type::make_leaf(v, parent_pos);
	return rv;
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::release_pair(
	uintptr_t taken_pos
)
{
	auto p_pos(find_vacant(taken_pos));
	auto q_pos(items.ptr_at(p_pos)->parent());

	*items.ptr_at(taken_pos) = pair_type::make_vacant(p_pos, q_pos);
	items.ptr_at(p_pos)->set_parent(taken_pos, false);
	items.ptr_at(q_pos)->set_base_offset(taken_pos);
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::release_ancestors(
	uintptr_t taken_pos
)
{
	while (taken_pos) {
		auto p(items.ptr_at(taken_pos));
		auto last_index(
			p->base_offset()
			+ index_offset(std::get<0>(tup_breadth_map))
		);
		uintptr_t found_index(last_index);

		items.for_each(
			p->base_offset() + terminator_index,
			[&found_index, taken_pos](
				size_type pos, pair_type const &pp
			) -> bool {
				if (pos >= found_index)
					return true;

				if (pp.parent() == taken_pos) {
					found_index = pos;
					return true;
				} else
					return false;
			}
		);

		if (found_index < last_index)
			break;

		auto n_pos(p->parent());
		release_pair(taken_pos);
		taken_pos = n_pos;
	}
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::move_pair(
	uintptr_t src_taken_pos, uintptr_t dst_vacant_pos,
	uintptr_t src_vacant_hint
)
{
	auto const breadth(index_offset(std::get<0>(tup_breadth_map)));
	auto src_pair(items.ptr_at(src_taken_pos));
	auto dst_pair(items.ptr_at(dst_vacant_pos));

	iter_loc tree_loc(src_pair->base_offset(), src_taken_pos);
	while (true) {
		auto p(tree_loc.next_child(items, breadth));
		if (!p.first)
			break;

		p.first->set_parent(dst_vacant_pos, true);
		++tree_loc.index;
	}

	items.ptr_at(dst_pair->parent())->set_base_offset(
		dst_pair->base_offset()
	);
	items.ptr_at(dst_pair->base_offset())->set_parent(
		dst_pair->parent(), false
	);

	*dst_pair = *src_pair;

	auto prev_pos(items.ptr_at(0)->parent());
	auto next_pos(items.ptr_at(prev_pos)->base_offset());

	if (next_pos < src_taken_pos) {
		next_pos = src_vacant_hint;
		while (src_taken_pos < next_pos)
			next_pos = items.ptr_at(next_pos)->parent();

		while (src_taken_pos > next_pos)
			next_pos = items.ptr_at(next_pos)->base_offset();

		prev_pos = items.ptr_at(next_pos)->parent();
	}

	*src_pair = pair_type::make_vacant(next_pos, prev_pos);
	items.ptr_at(prev_pos)->set_base_offset(src_taken_pos);
	items.ptr_at(next_pos)->set_parent(src_taken_pos, false);
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::relocate(
	pair_loc loc, uintptr_t c_index, value_pair *v
)
{
	std::vector<
		uintptr_t, allocator_type
	> index_set(items.get_allocator());
	iter_loc tree_loc(loc);
	auto const breadth(index_offset(std::get<0>(tup_breadth_map)));

	while (true) {
		auto p(tree_loc.next_child(items, c_index));
		if (!p.first)
			break;

		index_set.push_back(tree_loc.index);
		++tree_loc.index;
	}

	index_set.push_back(c_index);
	auto c_index_pos(index_set.size() - 1);

	while (true) {
		auto p(tree_loc.next_child(items, breadth));
		if (!p.first)
			break;

		index_set.push_back(tree_loc.index);
		++tree_loc.index;
	}

	auto n_pos(find_vacant_set(
		index_set[0], &index_set[1], index_set.size() - 1
	));
	auto base_offset(loc.first->base_offset());
	auto new_base_offset(n_pos - index_set[0]);
	uintptr_t vacant_pos_hint(items.ptr_at(0)->parent());

	for (size_type c(0); c < c_index_pos; ++c) {
		move_pair(
			base_offset + index_set[c],
			new_base_offset + index_set[c],
			vacant_pos_hint
		);
		vacant_pos_hint = base_offset + index_set[c];
	}

	reserve_vacant(loc.second, new_base_offset + c_index, v);

	for (size_type c(c_index_pos + 1); c < index_set.size(); ++c) {
		move_pair(
			base_offset + index_set[c],
			new_base_offset + index_set[c],
			vacant_pos_hint
		);
		vacant_pos_hint = base_offset + index_set[c];
	}

	loc.first->set_base_offset(new_base_offset);
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::unroll_suffix(
	pair_loc loc, size_type count, uintptr_t other_index, value_pair *v
)
{
	auto a(items.get_allocator());
	size_type shrink_count(0);
	auto shrink = [&shrink_count, &a](value_pair *v) -> void {
		v->shrink_suffix(a, shrink_count);
	};

	std::unique_ptr<value_pair, decltype(shrink)> leaf_ptr(
		loc.first->leaf_ptr(), shrink
	);
	auto s_ptr(leaf_ptr->suffix());

	for (; shrink_count < count; ++shrink_count) {
		auto pos(index_offset(s_ptr[shrink_count]));
		auto n_pos(find_vacant(pos));
		reserve_vacant(loc.second, n_pos, leaf_ptr.get());
		loc.first->set_base_offset(n_pos - pos);
		loc = pair_loc(items.ptr_at(n_pos), n_pos);
	}

	auto c_index(terminator_index);
	if (leaf_ptr->suffix_length > count)
		c_index = index_offset(s_ptr[count]);

	bool ord(other_index > c_index);
	auto n_pos(
		ord
		? find_vacant_set(c_index, &other_index, 1)
		: find_vacant_set(other_index, &c_index, 1)
	);
	auto base_offset(ord ? (n_pos - c_index) : (n_pos - other_index));

	reserve_vacant(loc.second, base_offset + c_index, leaf_ptr.get());
	loc.first->set_base_offset(base_offset);
	if (c_index > terminator_index)
		++shrink_count;

	reserve_vacant(loc.second, base_offset + other_index, v);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc, typename IndexIterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::value_pair::construct(
	Alloc const &a, IndexIterator first, IndexIterator last, Args&&... args
) -> value_pair *
{
	typedef allocator::array_helper<index_char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	size_type suffix_length(std::distance(first, last));

	auto s_deleter(
		[&a, suffix_length](index_char_type *p) -> void {
			ah_c::destroy(a, p, suffix_length, true);
		}
	);

	std::unique_ptr<index_char_type[], decltype(s_deleter)> s_ptr(
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
	typedef allocator::array_helper<index_char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	auto s(p->suffix());

	ah_c::destroy(a, s, p->suffix_length, false);

	if (p->suffix_length > Policy::short_suffix_length)
		ah_c::free(
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
	typedef allocator::array_helper<index_char_type, Alloc> ah;

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

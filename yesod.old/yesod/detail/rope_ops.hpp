/*
 * Copyright (c) 2010-2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on original implementation of __gnu_cxx::rope:

    Copyright (c) 2001-2013 Free Software Foundation, Inc.

    This file is part of the GNU ISO C++ Library.  This library is free
    software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3, or (at your option)
    any later version.


    Copyright (c) 1997 Silicon Graphics Computer Systems, Inc.

    Permission to use, copy, modify, distribute and sell this software
    and its documentation for any purpose is hereby granted without fee,
    provided that the above copyright notice appear in all copies and
    that both that copyright notice and this permission notice appear
    in supporting documentation.  Silicon Graphics makes no
    representations about the suitability of this software for any
    purpose.  It is provided "as is" without express or implied warranty.
==============================================================================*/

#if !defined(UCPF_YESOD_DETAIL_ROPE_OPS_OCT_31_2013_1840)
#define UCPF_YESOD_DETAIL_ROPE_OPS_OCT_31_2013_1840

namespace ucpf { namespace yesod {

template <typename ValueType, typename Policy>
constexpr typename rope<ValueType, Policy>::rope_tag
rope<ValueType, Policy>::node::leaf::ref_tag;

template <typename ValueType, typename Policy>
constexpr typename rope<ValueType, Policy>::rope_tag
rope<ValueType, Policy>::node::concat::ref_tag;

template <typename ValueType, typename Policy>
size_t const rope<ValueType, Policy>::node::concat::extra_size = sizeof(
	allocator::aligned_storage_t<
		typename rope<ValueType, Policy>::node::concat
	>
);

template <typename ValueType, typename Policy>
constexpr typename rope<ValueType, Policy>::rope_tag
rope<ValueType, Policy>::node::substr::ref_tag;

template <typename ValueType, typename Policy>
size_t const rope<ValueType, Policy>::node::substr::extra_size = sizeof(
	allocator::aligned_storage_t<
		typename rope<ValueType, Policy>::node::substr
	>
);

template <typename ValueType, typename Policy>
constexpr typename rope<ValueType, Policy>::rope_tag
rope<ValueType, Policy>::node::func::ref_tag;

template <typename ValueType, typename Policy>
size_t const rope<ValueType, Policy>::node::func::extra_size = sizeof(
	allocator::aligned_storage_t<
		typename rope<ValueType, Policy>::node::func
	>
);

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::leaf::apply(
	node_ptr const &r, apply_func_t &&f, size_type begin, size_type end
) -> bool
{
	return f(extra(r) + begin, end - begin);

	return true;
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::leaf::substring(
	node_ptr const &r, size_type begin, size_type end, size_type adj_end
) -> node_ptr
{
	if (begin >= adj_end)
		return node_ptr();

	size_type res_len(adj_end - begin);

	if (res_len > Policy::lazy_threshold)
		return node::make_substr(r, begin, adj_end - begin);
	else
		return node::make_leaf(
			r, res_len, extra(r) + begin
		);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::concat::apply(
	node_ptr const &r, apply_func_t &&f, size_type begin, size_type end
) -> bool
{
	node_ptr left(extra(r)->left);
	size_type left_len(left->size);

	if (begin < left_len) {
		size_type left_end(std::min(left_len, end));

		if (!rope_type::apply(
			left, std::forward<apply_func_t>(f), begin, left_end
		))
			return false;
	}

	if (end > left_len) {
		node_ptr right(extra(r)->right);
		size_type right_begin(std::max(left_len, begin));

		if (!rope_type::apply(
			right, std::forward<apply_func_t>(f),
			right_begin - left_len, end - left_len
		))
			return false;
	}

	return true;
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::concat::substring(
	node_ptr const &r, size_type begin, size_type end, size_type adj_end
) -> node_ptr
{
	auto c(node::concat::extra(r));

	node_ptr left(c->left), right(c->right);
	size_type left_len(left->size);

	if (adj_end <= left_len)
		return rope_type::substring(left, begin, end);
	else if (begin >= left_len)
		return rope_type::substring(
			right, begin - left_len, adj_end - left_len
		);

	return rope_type::concat(
		rope_type::substring(left, begin, left_len),
		rope_type::substring(right, 0, end - left_len)
	);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::substr::apply(
	node_ptr const &r, apply_func_t &&f, size_type begin, size_type end
) -> bool
{
	auto s(extra(r));
	return rope_type::apply(
		s->base, std::forward<apply_func_t>(f), begin + s->offset,
		std::min(s->base->size, end + s->offset)
	);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::substr::substring(
	node_ptr const &r, size_type begin, size_type end, size_type adj_end
) -> node_ptr
{
	if (begin >= adj_end)
		return node_ptr();

	/* Avoid introducing multiple layers of substring nodes. */
	auto c(node::substr::extra(r));
	size_type res_len(adj_end - begin);

	if (res_len > Policy::lazy_threshold)
		return node::make_substr(
			c->base, begin + c->offset, res_len
		);
	else
		return rope_type::substring(
			c->base, begin + c->offset, begin + c->offset + res_len

		);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::func::apply(
	node_ptr const &r, apply_func_t &&f, size_type begin, size_type end
) -> bool
{
	auto len(end - begin);

	auto l(node::make_leaf(r, len));
	extra(r)->fn(leaf::extra(l), len, begin);
	return f(leaf::extra(l), len);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::node::func::substring(
	node_ptr const &r, size_type begin, size_type end,
	size_type adj_end
) -> node_ptr
{
	if (begin >= adj_end)
		return node_ptr();

	size_type res_len(adj_end - begin);

	if (res_len > Policy::lazy_threshold)
		return node::make_substr(r, begin, adj_end - begin);
	else {
		auto rv(node::make_leaf(r, res_len));
		extra(r)->fn(node::leaf::extra(rv), res_len, begin);
		return rv;
	}
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::add_leaf_to_forest(
	forest_t &forest, node_ptr const &r
) -> void
{
	int i; /* forest[0..i-1] is empty */
	auto s(r->size);
	node_ptr too_tiny;

	for (i = 0; s >= min_len::value[i + 1]/* not this bucket */; ++i) {
		if (bool(forest[i])) {
			too_tiny = concat_and_set_balanced(forest[i], too_tiny);
			forest[i].reset();
		}
	}

	node_ptr insertee(concat_and_set_balanced(too_tiny, r));

	for (;; ++i) {
		if (forest[i]) {
			insertee = concat_and_set_balanced(forest[i], insertee);
			forest[i].reset();
		}
		if ((i == Policy::max_rope_depth)
		    || (insertee->size < min_len::value[i+1])) {
			forest[i] = insertee;
			return;
		}
	}
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::add_to_forest(
	forest_t &forest, node_ptr const &r
) -> void
{
	if ((r->tag != rope_tag::concat) || r->is_balanced)
		add_leaf_to_forest(forest, r);
	else {
		auto c(node::concat::extra(r));
		add_to_forest(forest, c->left);
		add_to_forest(forest, c->right);
	}
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::balance(
	node_ptr const &r
) -> node_ptr
{
	forest_t forest;
	node_ptr rv;

	/* Invariant:
	 * The concatenation of forest in descending order is equal to r.
	 * forest[i].size >= min_len::value[i]
	 * forest[i].depth = i
	 */

	add_to_forest(forest, r);

	for (auto &p: forest)
		if (bool(p)) {
			rv = concat(p, rv);
			p.reset();
		}

	if (rv->depth > Policy::max_rope_depth)
		throw std::length_error("rope::balance");

	return rv;
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::concat(
	node_ptr const &l, node_ptr const &r
) -> node_ptr
{
	if (!l)
		return r;

	if (!r)
		return l;

	if (r->tag != rope_tag::leaf)
		return tree_concat(l, r);

	if (l->tag == rope_tag::leaf) {
		if ((l->size + r->size) <= Policy::max_copy)
			return leaf_concat_value_iter(
				l, node::leaf::extra(r), r->size
			);
	} else if (l->tag == rope_tag::concat) {
		auto lr(node::concat::extra(l)->right);

		if (
			(lr->tag == rope_tag::leaf)
			&& ((lr->size + r->size) <= Policy::max_copy)
		) {
			auto ll(node::concat::extra(l)->left);
			node_ptr rest(leaf_concat_value_iter(
				lr, node::leaf::extra(r), r->size
			));

			return tree_concat(ll, rest);
		}
	}

	return tree_concat(l, r);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::tree_concat(
	node_ptr const &l, node_ptr const &r
) -> node_ptr
{
	auto rv(node::make_concat(l, r));
	size_type depth(rv->depth);

	if ((depth > 20)
	    && ((rv->size < 1000)
		|| (depth > size_type(Policy::max_rope_depth))))
		return balance(rv);
	else
		return rv;
}

template <typename ValueType, typename Policy>
template <typename Iterator>
auto rope<ValueType, Policy>::leaf_concat_value_iter(
	node_ptr const &l, Iterator first, size_type n
) -> node_ptr
{
	auto rv(node::make_leaf(l, l->size + n));
	auto s(node::leaf::extra(rv));

	std::copy_n(node::leaf::extra(l), l->size, s);
	s += l->size;
	std::copy_n(first, n, s);
	return rv;
}

template <typename ValueType, typename Policy>
template <typename Iterator>
auto rope<ValueType, Policy>::concat_value_iter(
	node_ptr const &l, Iterator iter, size_type n
) -> node_ptr
{
	if (!n)
		return l;

	if (l->tag == rope_tag::leaf) {
		if ((l->size + n) <= Policy::max_copy)
			return leaf_concat_value_iter(l, iter, n);
	} else if (l->tag == rope_tag::concat) {
		auto c(node::concat::extra(l));
		auto r(c->right);
		if (r->tag == rope_tag::leaf) {
			if ((r->size + n) <= Policy::max_copy) {
				auto rr(leaf_concat_value_iter(r, iter, n));
				return tree_concat(c->left, rr);
			}
		}
	}

	return tree_concat(l, node::make_leaf(l, n, iter));
}

template <typename ValueType, typename Policy>
template <typename Iterator>
auto rope<ValueType, Policy>::flatten(
	node_ptr const &r, size_type offset, size_type n, Iterator iter
) -> Iterator
{
	size_type end(std::min(offset + n, r->size));

	apply(
		r, [&iter](ValueType const *in, size_type in_sz) -> bool {
			std::copy_n(in, in_sz, iter);
			iter += in_sz;
			return true;
		}, offset, end
	);
	return iter;
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::fetch(
	node_ptr const &r, size_type pos
) -> value_type
{
	value_type c;
	apply(
		r, [&c](value_type const *in, size_type in_sz) -> bool {
			std::copy_n(in, in_sz, &c);
			return true;
		},
		pos, pos + 1
	);

	return c;
}

template <typename ValueType, typename Policy>
void rope<ValueType, Policy>::iterator_base::set_buf(iterator_base &iter)
{
	size_type leaf_pos(iter.leaf_pos);
	size_type pos(iter.current_pos);
	auto l(iter.path_end[iter.path_index]);

	if (l->tag == rope_tag::leaf) {
		iter.buf_begin = node::leaf::extra(l);
		iter.buf_cur = iter.buf_begin + (pos - leaf_pos);
		iter.buf_end = iter.buf_begin + l->size;
	} else {
		size_type len(Policy::iterator_buf_len);
		size_type buf_start_pos(leaf_pos);
		size_t leaf_end(l->size + leaf_pos);

		if (buf_start_pos + len <= pos) {
			buf_start_pos = pos - len / 4;

			if (buf_start_pos + len > leaf_end)
				buf_start_pos = leaf_end - len;
		}

		if (buf_start_pos + len > leaf_end)
			len = leaf_end - buf_start_pos;

		apply(
			l,
			[&iter](
				value_type const *in, size_type in_sz
			) -> bool {
				std::copy_n(in, in_sz, iter.tmp_buf.begin());
				return true;
			},
			buf_start_pos - leaf_pos,
			buf_start_pos - leaf_pos + len
		);

		iter.buf_cur = &iter.tmp_buf[pos - buf_start_pos];
		iter.buf_begin = &iter.tmp_buf[0];
		iter.buf_end = &iter.tmp_buf[len];
	}
}

template <typename ValueType, typename Policy>
void rope<ValueType, Policy>::iterator_base::set_cache(iterator_base &iter)
{
	size_type pos(iter.current_pos);

	if (pos >= iter.root->size) {
		iter.buf_cur = 0;
		return;
	}

	std::array<node_ptr, Policy::max_rope_depth + 1> path;
	/* Bit vector marking right turns in the path. */
	decltype(iter.path_directions) dirns;
	int cur_depth(-1);
	size_type cur_start_pos(0);
	auto cur_rope(iter.root);

	while (true) {
		++cur_depth;
		path[cur_depth] = cur_rope;

		if (cur_rope->tag == rope_tag::concat) {
			auto c(node::concat::extra(cur_rope));
			size_type left_len(c->left->size);

			dirns <<= 1;

			if (pos >= cur_start_pos + left_len) {
				dirns.set(0, true);
				cur_rope = c->right;
				cur_start_pos += left_len;
			} else
				cur_rope = c->left;
		} else {
			iter.leaf_pos = cur_start_pos;
			break;
		}
	}

	{
		int i(-1);
		int j(cur_depth + 1 - Policy::path_cache_len);

		if (j < 0)
			j = 0;

		while (j <= cur_depth)
			iter.path_end[++i] = path[j++];

		iter.path_index = i;
	}

	iter.path_directions = dirns;
	set_buf(iter);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::iterator_base::set_cache_for_incr(
	iterator_base &iter
) -> void
{
	auto current_index(iter.path_index);
	auto len(iter.path_end[current_index]->size);
	auto node_start_pos(iter.leaf_pos);
	auto dirns(iter.path_directions);

	if (iter.current_pos - node_start_pos < len) {
		/* More stuff in this leaf, we just didn't cache it. */
		set_buf(iter);
		return;
	}

	/*  node_start_pos is starting position of last node. */
	while (--current_index >= 0) {
		if (!dirns.test(0)) /* Path turned left */
			break;

		auto c(node::concat::extra(iter.path_end[current_index]));

		/* Otherwise we were in the right child. Thus we should pop the
		 * concatenation node.
		 */
		node_start_pos -= c->left->size;
		dirns >>= 1;
	}

	if (current_index < 0) {
		/* We underflowed the cache. Punt. */
		set_cache(iter);
		return;
	}

	/* Node at current_index is a concatenation node. We are positioned on
	 * the first character in its right child. node_start_pos is starting
	 * position of current_node.
	 */
	auto l(iter.path_end[current_index]);
	auto c(node::concat::extra(l));
	node_start_pos += c->left->size;
	iter.path_end[++current_index] = c->right;
	dirns.set(0, true);
	l = c->right;

	while (l->tag == rope_tag::concat) {
		++current_index;

		if (Policy::path_cache_len == current_index) {
			for (
				int i(0); i < (Policy::path_cache_len - 1); ++i
			)
				iter.path_end[i] = iter.path_end[i+1];

			--current_index;
		}

		c = node::concat::extra(l);
		iter.path_end[current_index] = c->left;
		dirns <<= 1;
		l = c->left;
		/* node_start_pos is unchanged. */
	}

	iter.path_index = current_index;
	iter.leaf_pos = node_start_pos;
	iter.path_directions = dirns;
	set_buf(iter);
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::iterator_base::incr(size_type n) -> void
{
	current_pos += n;

	if (buf_cur) {
		size_type chars_left(buf_end - buf_cur);

		if (chars_left > n)
			buf_cur += n;
		else if (chars_left == n) {
			buf_cur += n;
			set_cache_for_incr(*this);
		} else
			buf_cur = 0;
	}
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::iterator_base::decr(size_type n) -> void
{
	if (buf_cur) {
		size_type chars_left(buf_cur - buf_begin);

		if (chars_left >= n)
			buf_cur -= n;
		else
			buf_cur = 0;
	}

	current_pos -= n;
}

template <typename ValueType, typename Policy>
template <typename CharType>
auto rope<ValueType, Policy>::dump(
	node_ptr const &r, std::basic_ostream<CharType> &os,
	int indent
) -> std::basic_ostream<CharType> &
{
	typedef std::ostream_iterator<value_type, CharType> iter_type;

	iter_type out_iter(os);

	for (int c(0); c < indent; ++c)
		os.put(os.fill());

	if (!r) {
		os << "NULL\n";
		return os;
	}

	if (r->tag == rope_tag::concat) {
		/* To maintain precise diagnostics we must avoid incrementing
		 * reference counts here.
		 */
		auto c(node::concat::extra(r));
		auto &left(c->left);
		auto &right(c->right);

		os << "Concatenation " << r.get() << " (rc = " << r.use_count()
		   << ", depth = " << static_cast<int>(r->depth) << ", size = "
		   << r->size << ", " << (r->is_balanced ? "" : "not")
		   << " balanced)\n";

		return dump(right, dump(left, os, indent + 2), indent + 2);
	} else {
		char const *kind;

		switch (r->tag) {
		case rope_tag::leaf:
			kind = "Leaf";
			break;
		case rope_tag::substr:
			kind = "Substring";
			break;
		case rope_tag::func:
			kind = "Function";
			break;
		default:
			kind = "(corrupted kind field!)";
		};

		os << kind << " " << r.get() << " (rc = " << r.use_count()
		   << ", depth = " << static_cast<int>(r->depth) << ", size = "
		   << r->size << ") ";

		std::array<value_type, Policy::max_printout_len> s;
		size_type s_len;

		{
			node_ptr prefix(
				substring(r, 0, Policy::max_printout_len)
			);

			flatten(prefix, s.begin());
			s_len = prefix->size;
		}

		out_iter = os;

		std::copy_n(s.begin(), s_len, out_iter);

		os << (r->size > s_len ? "...\n" : "\n");

		return os;
	}
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::compare(
	node_ptr const &left, node_ptr const &right
) -> int
{
	if (!right)
		return (!left) ? 1 : 0;

	if (!left)
		return -1;

	if (left->tag == rope_tag::leaf) {
		if (right->tag == rope_tag::leaf)
			return __gnu_cxx::lexicographical_compare_3way(
				node::leaf::extra(left),
				node::leaf::extra(left) + left->size,
				node::leaf::extra(right),
				node::leaf::extra(right) + right->size
			);
		else {
			const_iterator rstart(right, 0);
			const_iterator rend(right, right->size);
			return __gnu_cxx::lexicographical_compare_3way(
				node::leaf::extra(left),
				node::leaf::extra(left) + left->size,
				rstart, rend
			);
		}
	} else {
		const_iterator lstart(left, 0);
		const_iterator lend(left, left->size);

		if (right->tag == rope_tag::leaf)
			return __gnu_cxx::lexicographical_compare_3way(
				lstart, lend,
				node::leaf::extra(right),
				node::leaf::extra(right) + right->size
			);
		else {
			const_iterator rstart(right, 0);
			const_iterator rend(right, right->size);
			return __gnu_cxx::lexicographical_compare_3way(
				lstart, lend, rstart, rend
			);
		}
	}
}

template <typename ValueType, typename Policy>
template <typename Alloc>
rope<ValueType, Policy>::rope(
	size_type n, value_type const &v, Alloc const &a
)
{
	typedef rope<ValueType, Policy> rope_type;

	size_type const exponentiate_threshold(32);

	if (n == 0)
		return;

	size_type exponent(n / exponentiate_threshold);
	size_type rest(n % exponentiate_threshold);
	node_ptr remainder;

	if (rest)
		remainder = node::make_leaf(a, rest, v);

	rope_type remainder_rope(remainder), rv;

	if (exponent) {
		auto base_leaf(node::make_leaf(a, exponentiate_threshold, v));
		rope_type base_rope(base_leaf);

		rv = base_rope;

		if (exponent > 1) {
			/* A sort of power raising operation. */
			while ((exponent & 1) == 0) {
				exponent >>= 1;
				base_rope = base_rope + base_rope;
			}

			rv = base_rope;
			exponent >>= 1;

			while (exponent) {
				base_rope = base_rope + base_rope;

				if ((exponent & 1))
					rv = rv + base_rope;

				exponent >>= 1;
			}
		}

		if (remainder)
			rv += remainder_rope;
	} else
		rv = remainder_rope;

	root_node = rv.root_node;
}

template <typename ValueType, typename Policy>
auto rope<ValueType, Policy>::find(
	value_type const &v, size_type pos
) const -> size_type
{
	const_iterator iter(
		std::search_n(cbegin() + pos, cend(), 1, v)
	);
	auto iter_pos(iter.index());

	if (iter_pos == size())
		iter_pos = npos;

	return iter_pos;
}

template <typename ValueType, typename Policy>
template <typename Iterator>
auto rope<ValueType, Policy>::find(
	Iterator first, Iterator last, size_type pos
) const -> size_type
{
	const_iterator iter(std::search(cbegin() + pos, cend(), first, last));
	size_type iter_pos(iter.index());

	if (iter_pos == size())
		iter_pos = npos;

	return iter_pos;
}

template <typename CharType, typename ValueType, typename Policy>
std::basic_ostream<CharType> &operator<<(
	std::basic_ostream<CharType> &os, rope<ValueType, Policy> const &r
)
{
	typedef rope<ValueType, Policy> rope_type;
	std::ostream_iterator<ValueType, CharType> out_iter(os);
	auto rope_len(r.size());
	decltype(rope_len) w(os.width());
	auto pad_len((rope_len < w) ? (w - rope_len) : 0);
	bool left(os.flags() & std::ios::left);

	if (!left && pad_len > 0)
		std::fill_n(out_iter, pad_len, os.fill());

	rope_type::apply(
		r.root_node,
		[&out_iter](
			ValueType const *in, decltype(rope_len) in_sz
		) -> bool {
			std::copy_n(in, in_sz, out_iter);
			return true;
		}, 0, rope_len
	);

	if (left && pad_len > 0)
		std::fill_n(out_iter, pad_len, os.fill());

	return os;
}

}}
#endif

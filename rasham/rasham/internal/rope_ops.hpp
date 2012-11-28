/*
 * Copyright (C) 2010, 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 *
 * ***
 *
 * Derived from original implementation
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 * Free Software Foundation, Inc.
 *
 * ***
 *
 * May contain parts
 * Copyright (c) 1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#if !defined(_RASHAM_INTERNAL_ROPE_OPS_HPP)
#define _RASHAM_INTERNAL_ROPE_OPS_HPP

#include <stdexcept>
#include <ext/algorithm>

namespace rasham
{

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::size_type const
rope<char_type, traits_type_, alloc_type>::npos = static_cast<size_type>(-1);

/* These are Fibonacci numbers < 2**32. */
template<typename char_type, typename traits_type_, typename alloc_type>
unsigned long const
rope<char_type, traits_type_, alloc_type>::min_len[
	rope_param::max_rope_depth + 1
] = {
	/* 00 */1, /* 01 */2, /* 02 */3, /* 03 */5, /* 04 */8, /* 05 */13,
	/* 06 */21, /* 07 */34, /* 08 */55, /* 09 */89, /* 10 */144,
	/* 11 */233, /* 12 */377, /* 13 */610, /* 14 */987, /* 15 */1597,
	/* 16 */2584, /* 17 */4181, /* 18 */6765, /* 19 */10946, /* 20 */17711,
	/* 21 */28657, /* 22 */46368, /* 23 */75025, /* 24 */121393,
	/* 25 */196418, /* 26 */317811, /* 27 */514229, /* 28 */832040,
	/* 29 */1346269, /* 30 */2178309, /* 31 */3524578, /* 32 */5702887,
	/* 33 */9227465, /* 34 */14930352, /* 35 */24157817, /* 36 */39088169,
	/* 37 */63245986, /* 38 */102334155, /* 39 */165580141,
	/* 40 */267914296, /* 41 */433494437, /* 42 */701408733,
	/* 43 */1134903170, /* 44 */1836311903, /* 45 */2971215073UL
};


template<typename char_type, typename traits_type_, typename alloc_type>
const typename rope<char_type, traits_type_, alloc_type>::rope_rep_ops
rope<char_type, traits_type_, alloc_type>::rep_ops[int(rope_tag::last_tag)] = {
	{&rope_rep::apply,    &rope_rep::substring},
	{&rope_leaf::apply,   &rope_leaf::substring},
	{&rope_concat::apply, &rope_concat::substring},
	{&rope_substr::apply, &rope_substr::substring},
	{&rope_func::apply,   &rope_func::substring}
};

template<typename char_type, typename traits_type_, typename alloc_type>
void rope<char_type, traits_type_, alloc_type>::add_leaf_to_forest(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr *forest
)
{
	int i; /* forest[0..i-1] is empty */
	auto s(r->size);
	rope_rep_ptr too_tiny;

	for (i = 0; s >= min_len[i + 1]/* not this bucket */; ++i) {
		if (0 != forest[i]) {
			too_tiny = concat_and_set_balanced(forest[i], too_tiny);
			forest[i].reset();
		}
	}

	rope_rep_ptr insertee(concat_and_set_balanced(too_tiny, r));

	for (;; ++i) {
		if (forest[i]) {
			insertee = concat_and_set_balanced(forest[i], insertee);
			forest[i].reset();
		}
		if ((i == rope_param::max_rope_depth)
		    || (insertee->size < min_len[i+1])) {
			forest[i] = insertee;
			return;
		}
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
void rope<char_type, traits_type_, alloc_type>::add_to_forest(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr *forest
)
{
	rope_concat_ptr c(rep_cast<rope_concat>(r));

	if (!c || c->is_balanced)
		add_leaf_to_forest(r, forest);
	else {
		add_to_forest(c->left, forest);
		add_to_forest(c->right, forest);
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::balance(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r
)
{
	rope_rep_ptr forest[rope_param::max_rope_depth + 1];
	rope_rep_ptr rv;
	int i;

	/* Invariant:
	 * The concatenation of forest in descending order is equal to r.
	 * forest[i].size >= min_len[i]
	 * forest[i].depth = i
	 */

	add_to_forest(r, forest);

	for (i = 0; i <= rope_param::max_rope_depth; ++i)
		if (forest[i]) {
			rv = concat(forest[i], rv);
			forest[i].reset();
		}

	if (rv->depth > rope_param::max_rope_depth)
		throw std::length_error("rope::balance");

	return rv;
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::concat(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &l,
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r
)
{
	if (!l)
		return r;

	if (!r)
		return l;

	rope_leaf_ptr r_leaf(rep_cast<rope_leaf>(r));

	if (!r_leaf)
		return tree_concat(l, r);

	rope_leaf_ptr l_leaf(rep_cast<rope_leaf>(l));

	if (l_leaf) {
		if ((l_leaf->size + r_leaf->size)
		    <= size_type(rope_param::max_copy))
			return leaf_concat_char_iter(
				l_leaf, r_leaf->data,
				r_leaf->size
			);
	} else {
		rope_concat_ptr l_cat(rep_cast<rope_concat>(l));

		if (l_cat) {
			rope_leaf_ptr lr_leaf(
				rep_cast<rope_leaf>(l_cat->right)
			);

			if (lr_leaf
			    && ((lr_leaf->size + r_leaf->size)
				<= size_type(rope_param::max_copy))) {
				rope_rep_ptr ll(l_cat->left);
				rope_rep_ptr rest(leaf_concat_char_iter(
					lr_leaf, r_leaf->data,
					r_leaf->size
				));

				return tree_concat(ll, rest);
			}
		}
	}

	return tree_concat(l, r);
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::tree_concat(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &l,
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r
)
{
	rope_concat_ptr rv(rope_concat::make(l, r));
	size_type depth(rv->depth);

	if ((depth > 20)
	    && ((rv->size < 1000)
		|| (depth > size_type(rope_param::max_rope_depth))))
		return balance(rv);
	else
		return rv;
}

template<typename char_type, typename traits_type_, typename alloc_type>
template<typename input_iter_t>
typename rope<char_type, traits_type_, alloc_type>::rope_leaf_ptr
rope<char_type, traits_type_, alloc_type>::leaf_concat_char_iter(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_leaf_ptr const &r,
	input_iter_t iter,
	typename rope<char_type, traits_type_, alloc_type>::size_type n
)
{
	size_type old_len(r->size);

	rope_leaf_ptr rv(
		rope_leaf::make(old_len + n, *get_allocator<alloc_type>(r))
	);

	traits_type::copy(&rv->data[0], r->data, old_len);
	traits_type::copy(&rv->data[old_len], iter, n);

	return rv;
}

template<typename char_type, typename traits_type_, typename alloc_type>
template<typename input_iter_t>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::concat_char_iter(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	input_iter_t iter,
	typename rope<char_type, traits_type_, alloc_type>::size_type n
)
{
	if (!n)
		return r;

	rope_leaf_ptr l(rep_cast<rope_leaf>(r));

	if (l && l->size + n <= rope_type::copy_max)
		return leaf_concat_char_iter(l, iter, n);
	else {
		rope_concat_ptr c(rep_cast<rope_concat>(r));
		if (c) {
			l = rep_cast<rope_leaf>(c->right);

			if (l && l->size + n <= rope_type::copy_max) {
				rope_leaf_ptr right(leaf_concat_char_iter(
						l, iter, n
				));
				return tree_concat(c->left, right);
			}
		}
	}

	l = rope_leaf::make(iter, n, *get_allocator<alloc_type>(r));

	return tree_concat(r, l);
}

template<typename char_type, typename traits_type_, typename alloc_type>
char_type *rope<char_type, traits_type_, alloc_type>::flatten(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type n,
	char_type *s
)
{
	size_type end(begin + std::min(n, r->size));

	apply(
		r, [s](char_type const *in, size_type in_sz) -> bool {
			std::copy_n(in, in_sz, s);
			return true;
		},
		begin, end
	);
	return s + (end - begin);
}

template<typename char_type, typename traits_type_, typename alloc_type>
char_type
rope<char_type, traits_type_, alloc_type>::fetch(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type pos
)
{
	char_type c;
	apply(
		r, [&c](char_type const *in, size_type in_sz) -> bool {
			std::copy_n(in, in_sz, &c);
			return true;
		},
		pos, pos + 1
	);

	return c;
}

template<typename char_type, typename traits_type_, typename alloc_type>
bool rope<char_type, traits_type_, alloc_type>::rope_concat::apply(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	std::function<bool (char_type const *, size_type)> f,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type end
)
{
	rope_concat_ptr c(static_pointer_cast<rope_concat>(r));
	rope_rep_ptr left(c->left);
	size_type left_len(left->size);

	if (begin < left_len) {
		size_type left_end(std::min(left_len, end));

		if (!rope_type::apply(left, f, begin, left_end))
			return false;
	}

	if (end > left_len) {
		rope_rep_ptr right(c->right);
		size_type right_begin(std::max(left_len, begin));

		if (!rope_type::apply(right, f, right_begin - left_len,
				      end - left_len))
			return false;
	}

	return true;
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::rope_leaf::substring(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type end,
	typename rope<char_type, traits_type_, alloc_type>::size_type adj_end
)
{
	if (begin >= adj_end)
		return rope_rep_ptr();

	rope_leaf_ptr l(static_pointer_cast<rope_leaf>(r));
	size_type res_len(adj_end - begin);

	if (res_len > rope_param::lazy_threshold)
		return rope_substr::make(
			r, begin, adj_end - begin,
			*get_allocator<alloc_type>(r)
		);
	else
		return rope_leaf::make(
			l->data + begin, res_len,
			*get_allocator<alloc_type>(l)
		);
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::rope_concat::substring(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type end,
	typename rope<char_type, traits_type_, alloc_type>::size_type adj_end
)
{
	rope_concat_ptr c(static_pointer_cast<rope_concat>(r));

	rope_rep_ptr left(c->left), right(c->right);
	size_type left_len(left->size);

	if (adj_end <= left_len)
		return rope_type::substring(left, begin, end);
	else if (begin >= left_len)
		return rope_type::substring(right, begin - left_len,
					    adj_end - left_len);

	return concat(
		rope_type::substring(left, begin, left_len),
		rope_type::substring(right, 0, end - left_len)
	);
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::rope_substr::substring(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type end,
	typename rope<char_type, traits_type_, alloc_type>::size_type adj_end
)
{
	if (begin >= adj_end)
		return rope_rep_ptr();

	// Avoid introducing multiple layers of substring nodes.
	rope_substr_ptr old(static_pointer_cast<rope_substr>(r));
	size_type res_len(adj_end - begin);

	if (res_len > rope_param::lazy_threshold)
		return rope_substr::make(
			old->base, begin + old->start, adj_end - begin,
			*get_allocator<alloc_type>(old)
		);
	else
		return rope_type::substring(
			old->base, begin + old->start, adj_end - begin
		);
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::rope_rep_ptr
rope<char_type, traits_type_, alloc_type>::rope_func::substring(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	typename rope<char_type, traits_type_, alloc_type>::size_type begin,
	typename rope<char_type, traits_type_, alloc_type>::size_type end,
	typename rope<char_type, traits_type_, alloc_type>::size_type adj_end
)
{
	if (begin >= adj_end)
		return rope_rep_ptr();

	rope_func_ptr f(static_pointer_cast<rope_func>(r));
	size_type res_len(adj_end - begin);

	if (res_len > rope_param::lazy_threshold)
		return rope_substr::make(r, begin, adj_end - begin,
					 *get_allocator<alloc_type>(r));
	else {
		rope_leaf_ptr rv(rope_leaf::make(
			res_len, *get_allocator<alloc_type>(r)
		));

		f->fn(begin, res_len, rv->data);
		return rv;
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
void
rope<char_type, traits_type_, alloc_type>::iterator_base::setbuf(
	typename rope<char_type, traits_type_, alloc_type>::iterator_base &iter
)
{
	size_type leaf_pos(iter.leaf_pos);
	size_type pos(iter.current_pos);

	rope_leaf_ptr l(rep_cast<rope_leaf>(iter.path_end[iter.path_index]));

	if (l) {
		iter.buf_begin = l->data;
		iter.buf_cur = iter.buf_begin + (pos - leaf_pos);
		iter.buf_end = iter.buf_begin + l->size;
	} else {
		size_type len(rope_param::iterator_buf_len);
		size_type buf_start_pos(leaf_pos);
		size_t leaf_end(iter.path_end[iter.path_index]->size
				+ leaf_pos);

		if (buf_start_pos + len <= pos) {
			buf_start_pos = pos - len / 4;

			if (buf_start_pos + len > leaf_end)
				buf_start_pos = leaf_end - len;
		}

		if (buf_start_pos + len > leaf_end)
			len = leaf_end - buf_start_pos;

		auto tmp_buf(iter.tmp_buf);
		apply(
			iter.path_end[iter.path_index],
			[tmp_buf](char_type const *in, size_type in_sz)
			-> bool {
				std::copy_n(in, in_sz, tmp_buf);
				return true;
			},
			buf_start_pos - leaf_pos,
			buf_start_pos - leaf_pos + len
		);

		iter.buf_cur = iter.tmp_buf + (pos - buf_start_pos);
		iter.buf_begin = iter.tmp_buf;
		iter.buf_end = iter.tmp_buf + len;
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
void rope<char_type, traits_type_, alloc_type>::iterator_base::setcache(
	typename rope<char_type, traits_type_, alloc_type>::iterator_base &iter
)
{
	size_type pos(iter.current_pos);

	if (pos >= iter.root->size) {
		iter.buf_cur = 0;
		return;
	}

	rope_rep_ptr path[rope_param::max_rope_depth + 1];
	int cur_depth(-1);

	size_type cur_start_pos(0);

	/* Bit vector marking right turns in the path. */
	unsigned long long dirns(0);

	rope_rep_ptr cur_rope(iter.root);

	while (true) {
		++cur_depth;
		path[cur_depth] = cur_rope;
		rope_concat_ptr c(rep_cast<rope_concat>(cur_rope));

		if (c) {
			size_type left_len(c->left->size);

			dirns <<= 1;

			if (pos >= cur_start_pos + left_len) {
				dirns |= 1;
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
		int j(cur_depth + 1 - rope_param::path_cache_len);

		if (j < 0)
			j = 0;

		while (j <= cur_depth)
			iter.path_end[++i] = path[j++];

		iter.path_index = i;
	}

	iter.path_directions = dirns;
	setbuf(iter);
}

template<typename char_type, typename traits_type_, typename alloc_type>
void
rope<char_type, traits_type_, alloc_type>::iterator_base::setcache_for_incr(
	typename rope<char_type, traits_type_, alloc_type>::iterator_base &iter
)
{
	auto current_index(iter.path_index);
	auto len(iter.path_end[current_index]->size);
	auto node_start_pos(iter.leaf_pos);
	auto dirns(iter.path_directions);

	if (iter.current_pos - node_start_pos < len) {
		/* More stuff in this leaf, we just didn't cache it. */
		setbuf(iter);
		return;
	}

	/*  node_start_pos is starting position of last node. */
	while (--current_index >= 0) {
		if (!(dirns & 1)) /* Path turned left */
			break;

		rope_concat_ptr c(static_pointer_cast<rope_concat>(
			iter.path_end[current_index]
		));

		/* Otherwise we were in the right child. Thus we should pop the
		 * concatenation node.
		 */
		node_start_pos -= c->left->size;
		dirns >>= 1;
	}

	if (current_index < 0) {
		/* We underflowed the cache. Punt. */
		setcache(iter);
		return;
	}

	/* Node at current_index is a concatenation node. We are positioned on
	 * the first character in its right child. node_start_pos is starting
	 * position of current_node.
	 */
	rope_concat_ptr c(static_pointer_cast<rope_concat>(
		iter.path_end[current_index]
	));

	node_start_pos += c->left->size;
	iter.path_end[++current_index] = c->right;
	dirns |= 1;
	c = rep_cast<rope_concat>(c->right);

	while (c) {
		++current_index;

		if (rope_param::path_cache_len == current_index) {
			for (int i = 0; i < (rope_param::path_cache_len - 1);
			     ++i)
				iter.path_end[i] = iter.path_end[i+1];

			--current_index;
		}

		iter.path_end[current_index] = c->left;
		dirns <<= 1;
		c = rep_cast<rope_concat>(c->left);
		/* node_start_pos is unchanged. */
	}

	iter.path_index = current_index;
	iter.leaf_pos = node_start_pos;
	iter.path_directions = dirns;
	setbuf(iter);
}

template<typename char_type, typename traits_type_, typename alloc_type>
void rope<char_type, traits_type_, alloc_type>::iterator_base::incr(
	typename rope<char_type, traits_type_, alloc_type>::size_type n
)
{
	current_pos += n;

	if (buf_cur) {
		size_type chars_left(buf_end - buf_cur);

		if (chars_left > n)
			buf_cur += n;
		else if (chars_left == n) {
			buf_cur += n;
			setcache_for_incr(*this);
		} else
			buf_cur = 0;
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
void rope<char_type, traits_type_, alloc_type>::iterator_base::decr(
	typename rope<char_type, traits_type_, alloc_type>::size_type n
)
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

template<typename char_type, typename traits_type_, typename alloc_type>
std::basic_ostream<char_type, traits_type_>
&rope<char_type, traits_type_, alloc_type>::dump(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &r,
	std::basic_ostream<char_type, traits_type_> &os,
	int indent
)
{
	typedef std::ostream_iterator<char_type, char_type, traits_type_>
	iter_type;

	iter_type out_iter(os);

	std::fill_n(out_iter, indent, os.fill());


	if (!r) {
		os << "NULL\n";
		return os;
	}

	if (r->tag == rope_tag::concat) {
		/* To maintain precise diagnostics we must avoid incrementing
		 * reference counts here.
		 */

		rope_concat const *c(static_cast<rope_concat const *>(r.get()));
		rope_rep_ptr const &left(c->left);
		rope_rep_ptr const &right(c->right);

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

		char_type s[rope_param::max_printout_len];
		size_type s_len;

		{
			rope_rep_ptr prefix(
				substring(r, 0, rope_param::max_printout_len)
			);

			flatten(prefix, s);
			s_len = prefix->size;
		}

		out_iter = os;

		std::copy_n(s, s_len, out_iter);

		os << (r->size > s_len ? "...\n" : "\n");

		return os;
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
int rope<char_type, traits_type_, alloc_type>::compare(
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &left,
	typename rope<char_type, traits_type_, alloc_type>
	::rope_rep_ptr const &right
)
{
	if (!right)
		return (!left) ? 1 : 0;

	if (!left)
		return -1;

	auto left_len(left->size);
	auto right_len(right->size);

	rope_leaf_ptr l(rep_cast<rope_leaf>(left));

	if (l) {
		rope_leaf_ptr r(rep_cast<rope_leaf>(right));
		if (r)
			return __gnu_cxx::lexicographical_compare_3way(
				l->data, l->data + left_len, r->data,
				r->data + right_len
			);
		else {
			const_iterator rstart(right, 0);
			const_iterator rend(right, right_len);
			return __gnu_cxx::lexicographical_compare_3way(
				l->data, l->data + left_len, rstart, rend
			);
		}
	} else {
		const_iterator lstart(left, 0);
		const_iterator lend(left, left_len);
		rope_leaf_ptr r(rep_cast<rope_leaf>(right));

		if (r)
			return __gnu_cxx::lexicographical_compare_3way(
				lstart, lend, r->data, r->data + right_len
			);
		else {
			const_iterator rstart(right, 0);
			const_iterator rend(right, right_len);
			return __gnu_cxx::lexicographical_compare_3way(
				lstart, lend, rstart, rend
			);
		}
	}
}

template<typename char_type, typename traits_type_, typename alloc_type>
rope<char_type, traits_type_, alloc_type>::rope(size_type n, char_type c,
						alloc_type a)
: treeplus(rope_rep_ptr(), a)
{
	typedef rope<char_type, traits_type_, alloc_type> rope_type;

	size_type const exponentiate_threshold(32);

	if (n == 0)
		return;

	size_type exponent(n / exponentiate_threshold);
	size_type rest(n % exponentiate_threshold);
	rope_leaf_ptr remainder;

	if (rest)
		remainder = rope_leaf::make(rest, c, a);

	rope_type remainder_rope(remainder, a), rv(a);

	if (exponent) {
		rope_leaf_ptr base_leaf(rope_leaf::make(
			exponentiate_threshold, c, a
		));

		rope_type base_rope(base_leaf, a);

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

	std::get<0>(this->treeplus) = std::get<0>(rv.treeplus);
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::size_type
rope<char_type, traits_type_, alloc_type>::find(
	char_type c,
	typename rope<char_type, traits_type_, alloc_type>::size_type pos
) const
{
	const_iterator iter(
		std::search_n(const_begin() + pos, const_end(), 1, c)
	);
	auto iter_pos(iter.index());

	if (iter_pos == size())
		iter_pos = npos;

	return iter_pos;
}

template<typename char_type, typename traits_type_, typename alloc_type>
typename rope<char_type, traits_type_, alloc_type>::size_type
rope<char_type, traits_type_, alloc_type>::find(
	char_type const *s,
	typename rope<char_type, traits_type_, alloc_type>::size_type pos
) const
{
	const_iterator iter(
		std::search(const_begin() + pos, const_end(), s,
			    s + traits_type::length(s))
	);
	size_type iter_pos(iter.index());

	if (iter_pos == size())
		iter_pos = npos;

	return iter_pos;
}

template<typename char_type, typename traits_type_, typename alloc_type>
std::basic_ostream<char_type, traits_type_> &operator<<(
	std::basic_ostream<char_type, traits_type_> &os,
	rope<char_type, traits_type_, alloc_type> const &r
)
{
	typedef rope<char_type, traits_type_, alloc_type> rope_type;
	std::ostream_iterator<char_type, char_type, traits_type_> out_iter(os);
	auto rope_len(r.size());
	decltype(rope_len) w(os.width());
	auto pad_len((rope_len < w) ? (w - rope_len) : 0);
	bool left(os.flags() & std::ios::left);

	if (!left && pad_len > 0)
		std::fill_n(out_iter, pad_len, os.fill());

	rope_type::apply(
		std::get<0>(r.treeplus),
		[&out_iter](char_type const *in,
			    typename alloc_type::size_type in_sz)
		-> bool {
			std::copy_n(in, in_sz, out_iter);
			return true;
		},
		0, rope_len
	);

	if (left && pad_len > 0)
		std::fill_n(out_iter, pad_len, os.fill());

	return os;
}

}
#endif

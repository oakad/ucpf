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

#if !defined(UCPF_YESOD_DETAIL_ROPE_BASE_OCT_31_2013_1840)
#define UCPF_YESOD_DETAIL_ROPE_BASE_OCT_31_2013_1840

#include <bitset>
#include <stdexcept>
#include <algorithm>
#include <ext/algorithm>

#include <yesod/counted_ptr.hpp>
#include <yesod/mpl/value_cast.hpp>
#include <yesod/mpl/fibonacci_c.hpp>
#include <yesod/iterator/facade.hpp>
#include <yesod/detail/allocator_utils.hpp>

namespace ucpf { namespace yesod {

template <typename ValueType, typename Policy>
struct rope {
	typedef rope      rope_type;
	typedef ValueType value_type;
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	struct            reference;
	struct            pointer;
	struct            const_iterator;
	struct            iterator;
	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	static constexpr size_type npos = ~size_type(0);

protected:
	/* Tag for simplified dynamic-like rope component casts. */
	enum struct rope_tag : uint8_t {
		null = 0,
		leaf,
		concat,
		substr,
		func,
		last_tag
	};

	/* Fibonacci numbers */
	typedef typename mpl::value_cast<
		typename mpl::fibonacci_c<
			typename std::conditional<
				Policy::max_rope_depth < 46,
				uint32_t, uint64_t
			>::type,
			Policy::max_rope_depth + 1
		>::type
	> min_len;

	struct node;
	typedef counted_ptr<node> node_ptr;
	typedef std::array<node_ptr, Policy::max_rope_depth + 1> forest_t;
	typedef std::function<
		bool (value_type const *, size_type)
	> apply_func_t;

	struct node {
		struct leaf {
			static constexpr rope_tag ref_tag = rope_tag::leaf;

			static size_type rounded_up_size(size_type n)
			{
				// Allow slop for in-place expansion.

				return ((
					n >> Policy::alloc_granularity_shift
				) + 1) << Policy::alloc_granularity_shift;
			}

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type first, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type first,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void construct(
				Alloc const &a, node_ptr const &p
			)
			{
				detail::allocator_array_helper<
					value_type, Alloc
				>::make(a, p.get_extra(), p->size);
			}

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p)
			{
				detail::allocator_array_helper<
					value_type, Alloc
				>::destroy(
					a, reinterpret_cast<value_type *>(
						p->self->get_extra()
					), p->size, false
				);
			}

			static value_type *extra(node_ptr const &p)
			{
				return reinterpret_cast<value_type *>(
					p.get_extra()
				);
			}
		};

		struct concat {
			static constexpr rope_tag ref_tag = rope_tag::concat;
			static size_t const extra_size;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type first, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type first,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void construct(Alloc const &a, node_ptr const &p)
			{
				detail::allocator_array_helper<
					concat, Alloc
				>::make(a, p.get_extra(), 1);
			}

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p)
			{
				detail::allocator_array_helper<
					concat, Alloc
				>::destroy(
					a, reinterpret_cast<concat *>(
						p->self->get_extra()
					), 1, false);
			}

			static concat *extra(node_ptr const &p)
			{
				return reinterpret_cast<concat *>(
					p.get_extra()
				);
			}

			node_ptr left;
			node_ptr right;
		};

		struct substr {
			static constexpr rope_tag ref_tag = rope_tag::substr;
			static size_t const extra_size;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type first, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type first,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void construct(Alloc const &a, node_ptr const &p)
			{
				detail::allocator_array_helper<
					substr, Alloc
				>::make(a, p.get_extra(), 1);
			}

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p)
			{
				detail::allocator_array_helper<
					substr, Alloc
				>::destroy(
					a, reinterpret_cast<substr *>(
						p->self->get_extra()
					), 1, false
				);
			}

			static substr *extra(node_ptr const &p)
			{
				return reinterpret_cast<substr *>(
					p.get_extra()
				);
			}

			node_ptr base;
			size_type offset;
		};

		struct func {
			static constexpr rope_tag ref_tag = rope_tag::func;
			static size_t const extra_size;

			/* Same argument order as pread (2) */
			typedef std::function<
				void (value_type *, size_type, size_type)
			> func_type;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type first, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type first,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void construct(Alloc const &a, node_ptr const &p)
			{
				detail::allocator_array_helper<
					func, Alloc
				>::make(a, p.get_extra(), 1);
			}

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p)
			{
				detail::allocator_array_helper<
					func, Alloc
				>::destroy(
					a, reinterpret_cast<func *>(
						p->self->get_extra()
					), 1, false
				);
			}

			static func *extra(node_ptr const &p)
			{
				return reinterpret_cast<func *>(
					p.get_extra()
				);
			}

			func_type fn;
		};

		template <typename Alloc>
		static node_ptr make_leaf(
			Alloc const &a, size_type n
		)
		{
			auto rv(allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(
					leaf::rounded_up_size(n)
				),
				n, leaf::ref_tag, 0, true
			));
			rv->self = rv.get_value_container();
			leaf::construct(a, rv);
			return rv;
		}

		template <typename Alloc>
		static node_ptr make_leaf(
			Alloc const &a, size_type n, value_type const &v
		)
		{
			auto rv(make_leaf(a, n));
			std::fill_n(leaf::extra(rv), n, v);
			return rv;
		}

		template <typename Alloc, typename Iterator>
		static node_ptr make_leaf(
			Alloc const &a, size_type n, Iterator first
		)
		{
			auto rv(make_leaf(a, n));
			std::copy_n(first, n, leaf::extra(rv));
			return rv;
		}

		static node_ptr make_leaf(node_ptr const &a, size_type n)
		{
			alloc_wrapper w(
				leaf::rounded_up_size(n),
				n, leaf::ref_tag, 0, true
			);
			a.access_allocator(&w);
			return w.p;
		}

		template <typename Iterator>
		static node_ptr make_leaf(
			node_ptr const &a, size_type n, Iterator first
		)
		{
			auto rv(make_leaf(a, n));
			std::copy_n(first, n, leaf::extra(rv));
			return rv;
		}

		template <typename Alloc>
		static node_ptr make_concat(
			Alloc const &a, node_ptr const &l, node_ptr const &r
		)
		{
			auto rv(allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(
					concat::extra_size
				),
				l->size + r->size, concat::ref_tag,
				std::max(l->depth, r->depth) + 1,
				false
			));
			rv->self = rv.get_value_container();
			concat::construct(a, rv);
			auto c(concat::extra(rv));
			c->left = l;
			c->right = r;
			return rv;
		}

		static node_ptr make_concat(
			node_ptr const &l, node_ptr const &r
		)
		{
			alloc_wrapper w(
				concat::extra_size, l->size + r->size,
				concat::ref_tag,
				std::max(l->depth, r->depth) + 1,
				false
			);
			l.access_allocator(&w);
			auto c(concat::extra(w.p));
			c->left = l;
			c->right = r;
			return w.p;
		}

		template <typename Alloc>
		static node_ptr make_substr(
			Alloc const &a, node_ptr const &base, size_type offset,
			size_type n
		)
		{
			auto rv(allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(
					substr::extra_size
				),
				n, substr::ref_tag, 0, true
			));
			rv->self = rv.get_value_container();
			substr::construct(a, rv);
			auto c(substr::extra(rv));
			c->base = base;
			c->offset = offset;
			return rv;
		}

		static node_ptr make_substr(
			node_ptr const &base, size_type offset, size_type n
		)
		{
			alloc_wrapper w(
				substr::extra_size, n, substr::ref_tag,
				0, true
			);
			base.access_allocator(&w);
			auto c(substr::extra(w.p));
			c->base = base;
			c->offset = offset;
			return w.p;
		}

		template <typename Alloc>
		static node_ptr make_func(
			Alloc const &a, typename func::func_type const &f,
			size_type n
		)
		{
			auto rv(allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(
					func::extra_size
				),
				n, func::ref_tag, 0, true
			));
			rv->self = rv.get_value_container();
			func::construct(a, rv);
			func::extra(rv)->fn = f;
			return rv;
		}

		static bool apply(
			node_ptr const &r, apply_func_t &&f,
			size_type first, size_type end
		)
		{
			static constexpr bool (*disp[])(
				node_ptr const &, apply_func_t &&, size_type,
				size_type
			) = {
				&node::apply,
				&leaf::apply,
				&concat::apply,
				&substr::apply,
				&func::apply
			};
			return (*disp[int(r->tag)])(
				r, std::forward<apply_func_t>(f), first, end
			);
		}

		static node_ptr substring(
			node_ptr const &r, size_type first,
			size_type end, size_type adj_end
		)
		{
			static constexpr node_ptr (*disp[])(
				node_ptr const &, size_type, size_type,
				size_type
			) = {
				&node::substring,
				&leaf::substring,
				&concat::substring,
				&substr::substring,
				&func::substring
			};
			return (*disp[int(r->tag)])(r, first, end, adj_end);
		}

		template <typename Alloc>
		static void construct_extra(Alloc const &a, node_ptr const &p)
		{
			static constexpr void (*disp[])(
				Alloc const &a, node_ptr const &p
			) = {
				nullptr,
				&leaf::template construct<Alloc>,
				&concat::template construct<Alloc>,
				&substr::template construct<Alloc>,
				&func::template construct<Alloc>
			};

			(*disp[int(p->tag)])(a, p);
		}

		template <typename Alloc>
		static void destroy(Alloc const &a, node *p)
		{
			static constexpr void (*disp[])(
				Alloc const &a, node *p
			) = {
				nullptr,
				&leaf::template destroy<Alloc>,
				&concat::template destroy<Alloc>,
				&substr::template destroy<Alloc>,
				&func::template destroy<Alloc>
			};

			(*disp[int(p->tag)])(a, p);
			detail::allocator_array_helper<
				node, Alloc
			>::destroy(a, p, 1, false);
		}

		struct alloc_wrapper {
			alloc_wrapper(
				size_type extra_size_,
				size_type size_,
				rope_tag tag_,
				uint8_t depth_,
				bool is_balanced_
			) : extra_size(extra_size_), size(size_), tag(tag_),
			    depth(depth_), is_balanced(is_balanced_)
			{}

			node_ptr p;
			size_type extra_size;
			size_type size;
			rope_tag tag;
			uint8_t depth;
			bool is_balanced;
		};

		template <typename Alloc>
		static void access_allocator(
			Alloc &a, node const *p, void *data
		)
		{
			auto w(reinterpret_cast<alloc_wrapper *>(data));

			w->p = allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(w->extra_size),
				*w
			);
			w->p->self = w->p.get_value_container();
			construct_extra(a, w->p);
		}

		node(
			size_type size_, rope_tag tag_, uint8_t depth_,
			bool is_balanced_
    		) : size(size_), tag(tag_), depth(depth_),
		    is_balanced(is_balanced_)
		{}

		node(alloc_wrapper const &w)
		: size(w.size), tag(w.tag), depth(w.depth),
		  is_balanced(w.is_balanced)
		{}

		detail::counted_ptr_val<node> *self;
		size_type size;
		rope_tag tag;
		uint8_t depth;
		bool is_balanced;
	};

	struct iterator_base
	{
		/* Iterator value */
		size_type    current_pos;
		/* The whole referenced rope */
		node_ptr root;
		/* Starting position for the current leaf */
		size_type    leaf_pos;

		/* path_end contains the bottom section of the path from the
		 * root to the current leaf, representing partial path cache.
		 * The path is truncated to keep iterators copying penalty to
		 * the possible minimum.
		 */
		std::array<node_ptr, Policy::path_cache_len> path_end;

		/* Last valid position in path_end.
		 * path_end[0] ... path_end[path_index - 1] point to
		 * concatenation nodes.
		 */
		int path_index;

		/* (path_directions >> i) & 1 is 1 iff we got from
		 * path_end[path_index - i - 1] to path_end[path_index - i] by
		 * going to the right.
		 */
		std::bitset<Policy::max_rope_depth + 1> path_directions;

		/* Buffer possibly containing current char. */
		value_type const *buf_begin;

		/* One past last valid value in buffer. */
		value_type const *buf_end;

		/* Pointer to current char in buffer. Iff buf_cur != 0 buffer
		 * is valid.
		 */
		value_type const *buf_cur;

		/* Short buffer for surrounding values. This is useful primarily
		 * for function nodes. We put the buffer here to avoid locking
		 * in the multithreaded case. The cached path is generally
		 * assumed to be valid only if the buffer is valid.
		 */
		std::array<value_type, Policy::iterator_buf_len> tmp_buf;

		static void set_buf(iterator_base &iter);

		static void set_cache(iterator_base &iter);

		static void set_cache_for_incr(iterator_base &iter);

		void incr(size_type n);
		void decr(size_type n);

		size_type index() const
		{
			return current_pos;
		}

		iterator_base()
		{}

		iterator_base(node_ptr const &root_, size_type pos)
		: current_pos(pos), root(root_), buf_cur(nullptr)
		{}

		iterator_base(iterator_base const &iter)
		{
			if (iter.buf_cur)
				*this = iter;
			else {
				current_pos = iter.current_pos;
				root = iter.root;
				buf_cur = nullptr;
			}
		}
	};

	static bool is_balanced(node_ptr const &r)
	{
		return (r->size >= min_len::value[r->depth]);
	}

	static node_ptr concat_and_set_balanced(
		node_ptr const &l, node_ptr const &r
	)
	{
		node_ptr result(concat(l, r));

		if (is_balanced(result))
			result->is_balanced = true;

		return result;
	}

	static void add_leaf_to_forest(forest_t &forest, node_ptr const &r);

	static void add_to_forest(forest_t &forest, node_ptr const &r);

	static node_ptr balance(node_ptr const &r);

	static node_ptr concat(node_ptr const &l, node_ptr const &r);

	static node_ptr tree_concat(node_ptr const &l, node_ptr const &r);

	template <typename Iterator>
	static node_ptr leaf_concat_value_iter(
		node_ptr const &l, Iterator iter, size_type n
	);

	template <typename Iterator>
	static node_ptr concat_value_iter(
		node_ptr const &l, Iterator iter, size_type n
	);

	template <typename Iterator>
	static Iterator flatten(
		node_ptr const &r, size_type offset, size_type n, Iterator iter
	);

	template <typename Iterator>
	static Iterator flatten(node_ptr const &r, Iterator first)
	{
		return flatten(r, 0, r->size, first);
	}

	static value_type fetch(node_ptr const &r, size_type pos);

	static node_ptr substring(
		node_ptr const &r, size_type first, size_type last
	)
	{
		if (!r)
			return r;

		size_type len(r->size);
		size_type adj_end;

		if (last >= len) {
			if (0 == first)
				return r;
			else
				adj_end = len;
		} else
			adj_end = last;

		return node::substring(r, first, last, adj_end);
	}

	static bool apply(
		node_ptr const &r, apply_func_t &&f,
		size_type first, size_type last
	)
	{
		if (r)
			return node::apply(
				r, std::forward<apply_func_t>(f), first, last
			);
		else
			return true;
	}

	static node_ptr replace(
		node_ptr const &old, size_type first, size_type last,
		node_ptr const &r
	)
	{
		if (!old)
			return r;

		node_ptr left(substring(old, 0, first));
		node_ptr right(substring(old, last, old->size));

		if (!r)
			return concat(left, right);
		else
			return concat(concat(left, r), right);
	}

	int compare(node_ptr const &left, node_ptr const &right);

	template <typename CharType>
	static std::basic_ostream<CharType> &dump(
		node_ptr const &r, std::basic_ostream<CharType> &os,
		int indent = 0
	);

	node_ptr root_node;

	rope(node_ptr const &other)
	: root_node(other)
	{}

public:
	struct reference {
		reference(rope_type &r, size_type pos_)
		: root(r), pos(pos_), current_valid(false)
		{}

		reference(reference const &ref)
		: root(ref.root), pos(ref.pos), current_valid(false),
		  current(ref.current)
		{}

		reference(rope_type &r, size_type pos_, value_type const &c)
		: root(r), pos(pos_), current_valid(true), current(c)
		{}

		operator value_type() const
		{
			if (current_valid)
				return current;
			else
				return rope_type::fetch(
					std::get<0>(root.root_node), pos
				);
		}

		reference &operator=(value_type const &c)
		{
			node_ptr old(std::get<0>(root.root_node));

			node_ptr left(rope_type::substring(old, 0, pos));
			node_ptr right(
				rope_type::substring(old, pos + 1, old->size)
			);

			std::get<0>(root.root_node) = rope_type::concat(
				rope_type::concat_value_iter(left, &c, 1),
				right
			);

			return *this;
		}

		pointer operator&() const
		{
			return pointer(*this);
		}

		reference &operator=(reference const &c)
		{
			return operator=(value_type(c));
		}

	private:
		friend struct pointer;

		value_type current;
		rope_type  &root;
		size_type  pos;
		bool       current_valid;
	};

	struct pointer {
		pointer(reference const &ref)
		: root(&ref.root), pos(ref.pos)
		{}

		pointer(pointer const &ptr)
		: root(&ptr.root), pos(ptr.pos)
		{}

		pointer()
		: root(0), pos(0)
		{}

		pointer &operator=(pointer const &ptr)
		{
			root = ptr.root;
			pos = ptr.pos;
			return *this;
		}

		reference operator*() const
		{
			return reference(root, pos);
		}

		template <typename ValueType1, typename Policy1>
		friend bool operator==(
			typename rope<ValueType1, Policy1>::pointer const &x,
			typename rope<ValueType1, Policy1>::pointer const &y
		)
		{
			return (x.pos == y.pos) && (x.root == y.root);
		}

		template <typename ValueType1, typename Policy1>
		friend bool operator!=(
			typename rope<ValueType1, Policy1>::pointer const &x,
			typename rope<ValueType1, Policy1>::pointer const &y
		)
		{
			return (x.pos != y.pos) || (x.root != y.root);
		}

	private:
		rope_type *root;
		size_type pos;
	};

	struct const_iterator : yesod::iterator::facade<
		const_iterator, value_type const,
		yesod::iterator::random_access_traversal_tag
	>, iterator_base {
		const_iterator() = default;

		const_iterator(const_iterator const &iter)
		: iterator_base(iter)
		{}

		const_iterator(iterator const &iter)
		: iterator_base(iter)
		{}

		const_iterator(rope_type const &r, size_type pos)
		: iterator_base(std::get<0>(r.treeplus), pos)
		{}

	private:
		friend struct rope;
		friend struct yesod::iterator::core_access;

		const_iterator(node_ptr const &root_, size_type pos_)
		: iterator_base(root_, pos_)
		{}

		bool equal(const_iterator const &other) const
		{
			return (this->current_pos == other.current_pos)
				&& (this->root == other.root);
		}

		void increment()
		{
			this->incr(1);
		}

		void decrement()
		{
			this->decr(1);
		}

		void advance(difference_type n)
		{
			if (n >= 0)
				this->incr(n);
			else
				this->decr(-n);
		}

		difference_type distance_to(const_iterator const &other) const
		{
			auto x(static_cast<difference_type>(other.current_pos));
			auto y(static_cast<difference_type>(this->current_pos));

			return x - y;
		}

		value_type const &dereference() const
		{
			if (!this->buf_cur)
				this->set_cache(
					*const_cast<const_iterator *>(this)
				);

			return *this->buf_cur;
		}
	};

	struct iterator : yesod::iterator::facade<
		iterator, reference,
		yesod::iterator::random_access_traversal_tag
	>, iterator_base {
		iterator()
		: root_rope(nullptr)
		{}

		iterator(iterator const &iter)
		: iterator_base(iter), root_rope(iter.root_rope)
		{}

		iterator(rope_type &r, size_type pos_)
		: iterator_base(std::get<0>(r.treeplus), pos_), root_rope(&r)
		{}

	private:
		friend struct rope;
		friend struct yesod::iterator::core_access;

		iterator(rope_type *r, size_type pos_)
		: iterator_base(std::get<0>(r->treeplus), pos_), root_rope(r)
		{
			if (!r->empty())
				set_cache(*this);
		}

		void check()
		{
			if (root_rope->root_node != this->root) {
				this->root = root_rope->root_node;
				this->buf_cur = nullptr;
			}
		}

		void increment()
		{
			this->incr(1);
		}

		void decrement()
		{
			this->decr(1);
		}

		void advance(difference_type n)
		{
			if (n >= 0)
				this->incr(n);
			else
				this->decr(-n);
		}

		difference_type distance_to(const_iterator const &other) const
		{
			auto x(static_cast<difference_type>(other.current_pos));
			auto y(static_cast<difference_type>(this->current_pos));
			return x - y;
		}

		bool equal(const_iterator const &other) const
		{
			return (this->current_pos == other.current_pos)
				&& (this->root_rope == other.root_rope);
		}

		reference dereference()
		{
			check();

			if (!this->buf_cur)
				return reference(*root_rope, this->current_pos);
			else
				return reference(
					*root_rope, this->current_pos,
					*this->buf_cur
				);
		}

		rope_type *root_rope;
	};

	template <typename Alloc>
	Alloc const &get_allocator() const
	{
		return root_node.template get_allocator<Alloc>();
	}

	bool empty() const
	{
		return bool(root_node);
	}

	int compare(rope const &r) const
	{
		return compare(root_node, r.root_node);
	}

	template <typename Iterator, typename Alloc = std::allocator<void>>
	rope(Iterator first, Iterator last, Alloc const &a = Alloc())
	: root_node(node::make_leaf(a, std::distance(first, last), first))
	{}

	rope(const_iterator const &first, const_iterator const &last)
	: root_node(substring(first.root, first.current_pos, last.current_pos))
	{}

	rope(iterator const &first, iterator const &last)
	: root_node(substring(first.root, first.current_pos, last.current_pos))
	{}

	template <typename Alloc = std::allocator<void>>
	rope(value_type const &v, Alloc const &a = Alloc())
	: root_node(node::make_leaf(a, 1, v))
	{}

	template <typename Alloc = std::allocator<void>>
	rope(size_type n, value_type const &v, Alloc const &a = Alloc());

	typedef typename node::func::func_type generator_type;

	template <typename Alloc = std::allocator<void>>
	rope(generator_type const &fn, size_type n, Alloc const &a = Alloc())
	: root_node(node::make_func(a, fn, n))
	{}

	template <typename Range, typename Alloc = std::allocator<void>>
	rope(Range const &r, Alloc const &a = Alloc())
	: root_node(node::make_leaf(
		a, std::distance(std::begin(r), std::end(r)), std::begin(r)
	))
	{}

	rope() = default;

	rope(rope const &r)
	: root_node(r.root_node)
	{}

	rope(rope &&r)
	{
		r.root_node.swap(root_node);
	}

	~rope()
	{}

	rope &operator=(rope const &r)
	{
		root_node = r.root_node;
		return *this;
	}

	rope &operator=(rope &&r)
	{
		root_node = std::move(r.root_node);
		return *this;
	}

	void clear()
	{
		root_node.reset();
	}

	template <typename Alloc = std::allocator<void>>
	void push_back(value_type const &c, Alloc const &a = Alloc())
	{
		append(c, a);
	}

	void pop_back()
	{
		node_ptr old(root_node);

		root_node = substring(old, 0, old->size - 1);
	}

	value_type back() const
	{
		node_ptr r(root_node);

		return fetch(r, r->size - 1);
	}

	template <typename Alloc = std::allocator<void>>
	void push_front(value_type const &c, Alloc const &a = Alloc())
	{
		node_ptr old(root_node);
		node_ptr l;

		if (!old)
			l = node::make_leaf(a, 1, c);
		else
			l = node::make_leaf(old, 1, c);

		root_node = concat(l, old);
	}

	void pop_front()
	{
		node_ptr old(root_node);

		root_node = substring(old, 1, old->size);
	}

	value_type front() const
	{
		return fetch(root_node, 0);
	}

	void balance()
	{
		node_ptr old(root_node);

		root_node = balance(old);
	}

	template <typename Iterator>
	void copy(Iterator first) const
	{
		flatten(root_node, first);
	}

	template <typename Iterator>
	size_type copy(Iterator first, size_type n, size_type pos = 0) const
	{
		auto len(std::min<difference_type>(size() - pos, n));

		flatten(root_node, pos, len, first);
		return len;
	}

	template <typename Iterator>
	size_type copy(Iterator first, size_type n, const_iterator pos) const
	{
		auto len(std::min<difference_type>(n, cend() - pos));

		flatten(root_node, pos.current_pos, len, first);
		return len;
	}

	template <typename CharType>
	struct rope_dumper {
		friend struct rope;
		node_ptr const &r;

		rope_dumper(node_ptr const &r_)
		: r(r_)
		{}

		std::basic_ostream<CharType> &dump(
			std::basic_ostream<CharType> &os
		) const
		{
			return rope_type::dump(r, os);
		}

		friend std::basic_ostream<CharType> &operator<<(
			std::basic_ostream<CharType> &os,
			rope_dumper const &d
		)
		{
			return d.dump(os);
		}
	};

	template <typename CharType>
	rope_dumper<CharType> dump()
	{
		 return rope_dumper<CharType>(root_node);
	}

	template <typename CharType>
	std::basic_string<CharType> str() const
	{
		std::basic_string<CharType> s;
		s.reserve(size());
		copy(s.begin());
		return s;
	}

	void flatten()
	{
		if (root_node && (root_node->tag != rope_tag::leaf)) {
			auto l(node::make_leaf(
				root_node, size(), value_type()
			));
			copy(l->data);
			root_node = l;
		}
	}

	value_type operator[](size_type pos) const
	{
		return fetch(root_node, pos);
	}

	iterator begin() const
	{
		return iterator(this, 0);
	}

	iterator end() const
	{
		return iterator(this, size());
	}

	const_iterator cbegin() const
	{
		return const_iterator(root_node, 0);
	}

	const_iterator cend() const
	{
		return const_iterator(root_node, size());
	}

	reverse_iterator rbegin() const
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend() const
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}

	const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}

	size_type size() const
	{
		if (root_node)
			return root_node->size;
		else
			return 0;
	}

	size_type length() const
	{
		return size();
	}

	size_type capacity() const
	{
		return size();
	}

	size_type max_size() const
	{
		/* Guarantees that the result can be sufficiently balanced.
		 * Longer ropes will probably still work, but it's harder to
		 * make guarantees.
		 */
		 return min_len::value[Policy::max_rope_depth - 1] - 1;
	}

	rope &operator+=(rope const &r)
	{
		return append(r);
	}

	template <typename Range>
	rope &operator+=(Range const &r)
	{
		return append(std::begin(r), std::end(r));
	}

	rope &operator+=(value_type const &v)
	{
		return append(v);
	}

	template <typename Iterator, typename Alloc = std::allocator<void>>
	rope &append(Iterator first, size_type n, Alloc const &a = Alloc())
	{
		if (root_node)
			root_node = concat_value_iter(root_node, first, n);
		else
			root_node = node::make_leaf(
				std::allocator<void>(), first, n
			);

		return *this;

	}

	template <typename Iterator, typename Alloc = std::allocator<void>>
	rope &append(Iterator first, Iterator last, Alloc const &a = Alloc())
	{
		return append(first, std::distance(first, last), a);
	}

	rope &append(const_iterator first, const_iterator last)
	{
		root_node = concat(root_node, substring(
			first.root, first.current_pos, last.current_pos
		));

		return *this;
	}

	template <typename Alloc = std::allocator<void>>
	rope &append(value_type const &v, Alloc const &a = Alloc())
	{
		return append(&v, 1, a);
	}

	template <typename Range, typename Alloc = std::allocator<void>>
	rope &append(Range const &r, Alloc const &a = Alloc())
	{
		return append(std::begin(r), std::end(r), a);
	}
	
	rope &append(rope const &r)
	{
		root_node = concat(root_node, r.root_node);
		return *this;
	}

	void swap(rope &r)
	{
		root_node.swap(r.root_node);
	}

	void insert(size_type pos, rope const &r)
	{
		root_node = replace(root_node, pos, pos, r.root_node);
	}

	template <typename Iterator>
	void insert(size_type pos, Iterator first, size_type n)
	{
		node_ptr left(root_node, 0, pos);
		node_ptr right(root_node, pos, size());

		root_node = concat(concat_value_iter(left, first, n), right);
	}

	template <typename Range>
	void insert(size_type pos, Range const &r)
	{
		insert(pos, std::begin(r), std::end(r));
	}

	void insert(size_type pos, value_type const &v)
	{
		std::array<value_type, 1> x_v = {v};
		return insert(pos, x_v.begin(), x_v.end());
	}

	void replace(size_type pos, size_type n, rope const &r)
	{
		root_node = replace(root_node, pos, pos + n, r.root_node);
	}

	template <typename Iterator, typename Alloc = std::allocator<void>>
	void replace(
		size_type pos, size_type n, Iterator first, size_type count,
		Alloc const &a = Alloc()
	)
	{
		rope_type r(first, count, a);
		replace(pos, n, r);
	}

	template <typename Alloc = std::allocator<void>>
	void replace(
		size_type pos, size_type n, value_type const &v,
		Alloc const &a = Alloc()
	)
	{
		rope_type r(&v, 1, a);
		replace(pos, n, r);
	}

	template <typename Range, typename Alloc = std::allocator<void>>
	void replace(
		size_type pos, size_type n, Range const &r,
		Alloc const &a = Alloc()
	)
	{
		rope_type x_r(std::begin(r), std::end(r), a);
		replace(pos, n, r);
	}

	void replace(size_type pos, value_type const &v)
	{
		iterator iter(this, pos);
		*iter = v;
	}

	void replace(size_type pos, rope const &r)
	{
		replace(pos, 1, r);
	}

	void erase(size_type pos, size_type n)
	{
		root_node = replace(
			root_node, pos, pos + n, node_ptr()
		);
	}

	void erase(size_type pos)
	{
		erase(pos, pos + 1);
	}

	iterator insert(iterator const &pos, rope const &r)
	{
		insert(pos.index(), r);
		return pos;
	}

	iterator insert(iterator const &pos, size_type n, value_type const &v)
	{
		insert(pos.index(), n, v);
		return pos;
	}

	iterator insert(iterator const &pos, value_type const &v)
	{
		insert(pos.index(), v);
		return pos;
	}

	template <typename Iterator>
	iterator insert(iterator const &pos, Iterator first, size_type n)
	{
		insert(pos.index(), first, n);
		return pos;
	}

	void replace(iterator const &pos, rope const &r)
	{
		replace(pos.index(), r);
	}

	void replace(iterator const &pos, value_type const &v)
	{
		replace(pos.index(), v);
	}

	iterator erase(iterator const &first, iterator const &last)
	{
		size_type first_index(first.index());
		erase(first_index, last.index() - first_index);
		return iterator(this, first_index);
	}

	iterator erase(iterator const &pos)
	{
		size_type pos_index(pos.index());
		erase(pos_index, 1);
		return iterator(this, pos_index);
	}

	rope substr(size_type pos, size_type n = 1) const
	{
		return rope_type(substring(root_node, pos, pos + n));
	}

	rope substr(iterator const &first, iterator const &last) const
	{
		return rope_type(substring(
			root_node, first.index(), last.index()
		));
	}

	rope substr(iterator const &pos) const
	{
		size_type pos_index(pos.index());

		return rope_type(substring(
			root_node, pos_index, pos_index + 1
		));
	}

	rope substr(
		const_iterator const &first, const_iterator const &last
	) const
	{
		return rope_type(substring(
			root_node, first.index(), last.index()
		));
	}

	rope substr(const_iterator const &pos) const
	{
		size_type pos_index(pos.index());

		return rope_type(substring(
			root_node, pos_index, pos_index + 1
		));
	}

	size_type find(value_type const &v, size_type pos = 0) const;

	template <typename Iterator>
	size_type find(Iterator first, Iterator last, size_type pos = 0) const;

	reference mutable_reference_at(size_type pos)
	{
		return reference(this, pos);
	}

	reference operator[] (size_type pos)
	{
		return reference(this, pos);
	}

	template <typename ValueType1, typename Policy1>
	friend rope<ValueType1, Policy1>
	operator+(
		rope<ValueType1, Policy1> const &l,
		rope<ValueType1, Policy1> const &r
	);

	template <typename ValueType1, typename Policy1, typename Range>
	friend rope<ValueType1, Policy1>
	operator+(rope<ValueType1, Policy1> const &l, Range const &r);

	template <typename ValueType1, typename Policy1, typename Range>
	friend rope<ValueType1, Policy1>
	operator+(
		rope<ValueType1, Policy1> const &l,
		ValueType1 const &v
	);

	template <typename CharType, typename ValueType1, typename Policy1>
	friend std::basic_ostream<CharType> &operator<<(
		std::basic_ostream<CharType> &os,
		rope<ValueType1, Policy1> const &r
	);
};

template <typename ValueType, typename Policy>
rope<ValueType, Policy> operator+(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	typedef rope<ValueType, Policy> rope_type;

	return rope_type(rope_type::concat(l.root_node, r.root_node));
}

template <typename ValueType, typename Policy>
rope<ValueType, Policy> &operator+=(
	rope<ValueType, Policy> &l, rope<ValueType, Policy> const &r
)
{
	l.append(r);
	return l;
}
/*
template <typename ValueType, typename Policy, typename Range>
rope<ValueType, Policy> operator+(
	rope<ValueType, Policy>  const &l, Range const &r
)
{
	typedef rope<ValueType, Policy> rope_type;
	typedef typename rope_type::size_type size_type;

	size_type s_len(rope_type::traits_type::length(s));

	if (std::get<0>(l.treeplus))
		return rope_type(
			l.concat_value_iter(std::get<0>(l.treeplus), s, s_len),
			std::get<1>(l.treeplus)
		);
	else
		return rope_type(s, std::get<1>(l.treeplus));
}
*/
template <typename ValueType, typename Policy, typename Range>
rope<ValueType, Policy> &operator+=(
	rope<ValueType, Policy> &l, Range const &r
)
{
	l.append(r);
	return l;
}

template <typename ValueType, typename Policy>
rope<ValueType, Policy> operator+(
	rope<ValueType, Policy> &l,
	typename rope<ValueType, Policy>::value_type const &v
)
{
	typedef rope<ValueType, Policy> rope_type;

	if (l.root_node)
		return rope_type(
			rope_type::concat_value_iter(l.root_node, &v, 1)
		);
	else
		return rope_type(1, v);
}

template <typename ValueType, typename Policy>
rope<ValueType, Policy> &operator+=(
	rope<ValueType, Policy> &l,
	typename rope<ValueType, Policy>::value_type const &v
)
{
	l.append(v);
	return l;
}

template <typename ValueType, typename Policy>
bool operator<(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) < 0;
}

template <typename ValueType, typename Policy>
bool operator>(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) > 0;
}

template <typename ValueType, typename Policy>
bool operator<=(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) <= 0;
}

template <typename ValueType, typename Policy>
bool operator>=(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) >= 0;
}

template <typename ValueType, typename Policy>
bool operator==(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) == 0;
}

template <typename ValueType, typename Policy>
bool operator!=(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	return l.compare(r) != 0;
}

template <typename CharType, typename ValueType, typename Policy>
std::basic_ostream<CharType> &operator<<(
	std::basic_ostream<CharType> &os, rope<ValueType, Policy> const &r
);

}}
#endif

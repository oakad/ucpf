/*
 * Copyright (c) 2010 - 2014 Alex Dubov <oakad@yahoo.com>
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

#if !defined(UCPF_YESOD_DETAIL_ROPE_BASE_OCT_31_2013_1840)
#define UCPF_YESOD_DETAIL_ROPE_BASE_OCT_31_2013_1840

#include <algorithm>
#include <functional>
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
	typedef typedef value_cast<
		typename fibonacci_c<
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
				size_type begin, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type begin,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p);

			static value_type *extra(node_ptr const &p)
			{
				return reinterpret_cast<value_type *>(
					p.get_extra()
				);
			}
		};

		struct concat {
			static constexpr rope_tag ref_tag = rope_tag::concat;
			static size_type const extra_size;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type begin, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type begin,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p);

			static concat *extra(node_ptr const &p)
			{
				return reinterpret_cast<concat *>(
					p.get_extra()
				);
			}

			concat(node_ptr const &l, node_ptr const &r)
			: left(l), right(r)
			{}

			node_ptr left;
			node_ptr right;
		};

		struct substr {
			static constexpr rope_tag ref_tag = rope_tag::substr;
			static size_type const extra_size;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type begin, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type begin,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p);

			static substr *extra(node_ptr const &p)
			{
				return reinterpret_cast<substr *>(
					p.get_extra()
				);
			}

			substr(node_ptr const &base_, size_type offset_)
			: base(base_), offset(offset_)
			{}

			node_ptr base;
			size_type offset;
		};

		struct func {
			static rope_tag const ref_tag = rope_tag::func;
			static size_type const extra_size;

			/* Same argument order as pread (2) */
			typedef std::function<
				void (value_type *, size_type, size_type)
			> func_type;

			static bool apply(
				node_ptr const &r, apply_func_t &&f,
				size_type begin, size_type end
			);

			static node_ptr substring(
				node_ptr const &r, size_type begin,
				size_type end, size_type adj_end
			);

			template <typename Alloc>
			static void destroy(Alloc const &a, node *p);

			static func *extra(node_ptr const &p)
			{
				return reinterpret_cast<func *>(
					p.get_extra()
				);
			}

			func(func_type &&fn_)
			: fn(std::forward<func_type>(fn_))
			{}

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
				leaf::ref_tag, 0, true, n
			));
			rv->self = rv.get_value_container();
		}

		template <typename Alloc>
		static node_ptr make_leaf(
			Alloc const &a, size_type n, value_type c
		)
		{
			auto rv(make_leaf(a, n));
			detail::allocator_array_helper<
				value_type, Alloc
			>::make(a, rv.get_extra(), n, c);
			return rv;
		}

		template <typename Alloc, typename Iterator>
		static node_ptr make_leaf(
			Alloc const &a, Iterator first, Iterator last
		)
		{
			auto rv(make_leaf(a, last - first));
			detail::allocator_array_helper<
				value_type, Alloc
			>::make(a, rv.get_extra(), first, last);
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
				concat::ref_tag,
				std::max(l->depth, r->depth) + 1,
				false, l->size + r->size
			));
			rv->self = rv.get_value_container();
			detail::allocator_array_helper<
				concat, Alloc
			>::make(a, rv.get_extra(), 1, l, r);
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
				substr::ref_tag, 0, true, n
			));
			rv->self = rv.get_value_container();
			detail::allocator_array_helper<
				substr, Alloc
			>::make(a, rv.get_extra(), 1, base, offset);
		}

		template <typename Alloc>
		static node_ptr make_func(
			Alloc const &a, typename func::func_type &&f,
			size_type n
		)
		{
			auto rv(allocate_counted<node>(
				a,
				typename node_ptr::extra_size_t(
					func::extra_size
				),
				func::ref_tag, 0, true, n
			));
			rv->self = rv.get_value_container();
			detail::allocator_array_helper<
				func, Alloc
			>::make(
				a, rv.get_extra(), 1,
				std::forward<func::func_type>(f)
			);
		}

		static bool apply(
			node_ptr const &r, apply_func_t &&f,
			size_type begin, size_type end
		)
		{
			static constexpr bool (*ad[])(
				node_ptr const &, apply_func_t &&, size_type,
				size_type
			) = {
				&node::apply,
				&leaf::apply,
				&concat::apply,
				&substr::apply,
				&func::apply
			};
			return ad[r->tag](
				r, std::forward<apply_func_t>(f), begin, end
			);
		}

		static node_ptr substring(
			node_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		)
		{
			static constexpr bool (*sd[])(
				node_ptr const &, size_type, size_type,
				size_type
			) = {
				&node::substring,
				&leaf::substring,
				&concat::substring,
				&substr::substring,
				&func::substring
			};
			return sd[r->tag](r, begin, end, adj_end);
		}

		template <typename Alloc>
		static void destroy(Alloc const &a, node *p)
		{
			static constexpr void (*dd[])(
				Alloc const &a, node *p
			) = {
				nullptr,
				&leaf::destroy<Alloc>,
				&concat::destroy<Alloc>,
				&substr::destroy<Alloc>,
				&func::destroy<Alloc>
			};

			dd[p->tag](a, p);
			detail::allocator_array_helper<
				node, Alloc
			>::destroy(a, p, 1, false);
		}

		iterator::range<value_type *> leaf_range()
		{
			if (tag == rope_tag::leaf)
				return iterator::make_range(
					reinterpret_cast<value_type *>(
						self->get_extra()
					), size
				);
			else
				return iterator::make_range<value_type *>(
					nullptr, 0
				);
		}

		node(rope_tag tag_, uint8_t d, bool b, size_type size_)
		: size(size_), tag(tag_), depth(d), is_balanced(b ? 1 : 0)
		{}

		detail::counted_ptr_val<node> *self;
		size_type size;
		rope_tag  tag;
		uint8_t   depth;
		bool   is_balanced;
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
		node_ptr path_end[Policy::path_cache_len];

		/* Last valid position in path_end.
		 * path_end[0] ... path_end[path_index - 1] point to
		 * concatenation nodes.
		 */
		int path_index;

		/* (path_directions >> i) & 1 is 1 iff we got from
		 * path_end[path_index - i - 1] to path_end[path_index - i] by
		 * going to the right.
		 */
		unsigned long long path_directions;

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

		static void setbuf(iterator_base &iter);

		static void setcache(iterator_base &iter);

		static void setcache_for_incr(iterator_base &iter);

		void incr(size_type n);
		void decr(size_type n);

		size_type index() const
		{
			return current_pos;
		}

		iterator_base()
		{}

		iterator_base(node_ptr const &root_, size_type pos)
		: current_pos(pos), root(root_), buf_cur(0)
		{}

		iterator_base(iterator_base const &iter)
		{
			if (iter.buf_cur)
				*this = iter;
			else {
				current_pos = iter.current_pos;
				root = iter.root;
				buf_cur = 0;
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
		node_ptr const &r, Iterator iter, size_type n
	);

	template <typename Iterator>
	static node_ptr concat_value_iter(
		node_ptr const &r, Iterator iter, size_type n
	);

	static value_type *flatten(
		node_ptr const &r, size_type begin, size_type n,
		value_type *s
	);

	static value_type *flatten(node_ptr const &r, value_type *s)
	{
		return flatten(r, 0, r->size, s);
	}

	static value_type fetch(node_ptr const &r, size_type pos);

	static node_ptr substring(
		node_ptr const &r, size_type begin, size_type end
	)
	{
		if (!r)
			return r;

		size_type len(r->size);
		size_type adj_end;

		if (end >= len) {
			if (0 == begin)
				return r;
			else
				adj_end = len;
		} else
			adj_end = end;

		return node::substring(r, begin, end, adj_end);
	}

	static bool apply(
		node_ptr const &r, apply_func_t &&f,
		size_type begin, size_type end
	)
	{
		if (r)
			return node::apply(r, f, begin, end);
		else
			return true;
	}

	static node_ptr replace(
		node_ptr const &old, size_type begin, size_type end,
		node_ptr const &r
	)
	{
		if (!old)
			return r;

		node_ptr left(substring(old, 0, begin));
		node_ptr right(substring(old, end, old->size));

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
				this->setcache(
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
				setcache(*this);
		}

		void check()
		{
			if (std::get<0>(root_rope->treeplus) != this->root) {
				this->root = std::get<0>(root_rope->treeplus);
				this->buf_cur = 0;
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
				return reference(*root_rope, this->current_pos,
						 *this->buf_cur);
		}

		rope_type *root_rope;
	};

	template <typename Alloc>
	Alloc const &get_allocator() const
	{
		return root_node.get_allocator<Alloc>();
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
	rope(Iterator first, Iterator last, Alloc a = Alloc())
	: root_node(node::make_leaf(a, first, last))
	{}

	rope(const_iterator const &first, const_iterator const &last)
	: root_node(substring(first.root, first.current_pos, last.current_pos))
	{}

	rope(iterator const &first, iterator const &last)
	: root_node(substring(first.root, first.current_pos, last.current_pos))
	{}

	template <typename Alloc = std::allocator<void>>
	rope(value_type c, Alloc a = Alloc())
	: root_node(node::make_leaf(a, 1, c))
	{}

	template <typename Alloc = std::allocator<void>>
	rope(size_type n, value_type c, Alloc a = Alloc());

	rope() = default;

	template <typename Alloc = std::allocator<void>>
	rope(
		typename node::func::func_type &&fn, size_type n,
		Alloc a = Alloc()
	) : root_node(node::make_func(
		std::forward<typename node::func::func_type>(fn), n, a
	))
	{}

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

	void push_back(value_type const &c)
	{
		append(c);
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

	void push_front(value_type const &c)
	{
		node_ptr old(root_node);
		node_ptr l(node::make_leaf(
			std::allocator<void>(), 1, c
		));

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
		node_ptr r;

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

	template <typename Alloc = std::allocator<void>>
	void flatten(Alloc const &a = Alloc())
	{
		if (root_node && (root_node->tag != rope_tag::leaf)) {
			auto l(node::make_leaf(a, size(), value_type()));
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

	template <typename Container>
	rope &operator+=(Container const &s)
	{
		return append(s.begin(), s.end());
	}

	rope &operator+=(value_type const &c)
	{
		return append(c);
	}

	template <typename Iterator>
	rope &append(Iterator first, Iterator last)
	{
		if (root_node)
			root_node = concat_value_iter(
				root_node, first, last - first
			);
		else
			root_node = node::make_leaf(
				std::allocator<void>(), first, last
			);

		return *this;
	}

	rope &append(const_iterator first, const_iterator last)
	{
		root_node = concat(root_node, substring(
			first.root, first.current_pos, last.current_pos
		));

		return *this;
	}

	rope &append(value_type const &c)
	{
		std::array<value_type, 1> x_c = {c};
		return append(x_c.begin(), x_c.end());
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
#if 0
	template <typename input_iter_t>
	void insert(size_type pos, input_iter_t iter, size_type n)
	{
		node_ptr left(root_node, 0, pos);
		node_ptr right(root_node, pos, size());

		root_node = concat(
			concat_value_iter(left, iter, n), right
		);
	}

	void insert(size_type pos, CharType const *s)
	{
		insert(pos, s, traits_type::length(s));
	}

	void insert(size_type pos, CharType c)
	{
		insert(pos, &c, 1);
	}

	template <typename input_iter_t>
	void insert(size_type pos, input_iter_t begin, input_iter_t end)
	{
		rope_type r(begin, end, std::get<1>(treeplus));
		insert(pos, r);
	}

	void insert(size_type pos, const_iterator const &begin,
		    const_iterator const &end)
	{
		rope_type r(begin, end, std::get<1>(treeplus));
		insert(pos, r);
	}

	void insert(size_type pos, iterator const &begin, iterator const &end)
	{
		rope_type r(begin, end, std::get<1>(treeplus));
		insert(pos, r);
	}

	void replace(size_type pos, size_type n, rope const &r)
	{
		root_node = replace(
			root_node, pos, pos + n,
			std::get<0>(r.treeplus)
		);
	}

	template <typename input_iter_t>
	void replace(size_type pos, size_type n, input_iter_t iter,
		     size_type len)
	{
		rope_type r(iter, len, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	void replace(size_type pos, size_type n, CharType c)
	{
		rope_type r(c, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	void replace(size_type pos, size_type n, CharType const *s)
	{
		rope_type r(s, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	template <typename input_iter_t>
	void replace(size_type pos, size_type n, input_iter_t begin,
		     input_iter_t end)
	{
		rope_type r(begin, end, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	void replace(size_type pos, size_type n, const_iterator const &begin,
		     const_iterator const &end)
	{
		rope_type r(begin, end, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	void replace(size_type pos, size_type n, iterator const &begin,
		     iterator const &end)
	{
		rope_type r(begin, end, std::get<1>(r.treeplus));
		replace(pos, n, r);
	}

	void replace(size_type pos, CharType c)
	{
		iterator iter(this, pos);
		*iter = c;
	}

	void replace(size_type pos, rope const &r)
	{
		replace(pos, 1, r);
	}

	template <typename input_iter_t>
	void replace(size_type pos, input_iter_t iter, size_type len)
	{
		replace(pos, 1, iter, len);
	}

	void replace(size_type pos, CharType const *s)
	{
		replace(pos, 1, s);
	}

	template <typename input_iter_t>
	void replace(size_type pos, input_iter_t begin, input_iter_t end)
	{
		replace(pos, 1, begin, end);
	}

	void replace(size_type pos, const_iterator const &begin,
		     const_iterator const &end)
	{
		replace(pos, 1, begin, end);
	}

	void replace(size_type pos, iterator const &begin, iterator const &end)
	{
		replace(pos, 1, begin, end);
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

	iterator insert(iterator const &pos, const rope& r)
	{
		insert(pos.index(), r);
		return pos;
	}

	iterator insert(iterator const &pos, size_type n, CharType c)
	{
		insert(pos.index(), n, c);
		return pos;
	}

	iterator insert(iterator const &pos, CharType c)
	{
		insert(pos.index(), c);
		return pos;
	}

	iterator insert(iterator const &pos, CharType const *s)
	{
		insert(pos.index(), s);
		return pos;
	}

	template <typename input_iter_t>
	iterator insert(iterator const &pos, input_iter_t begin, size_t n)
	{
		insert(pos.index(), begin, n);
		return pos;
	}

	template <typename input_iter_t>
	iterator insert(iterator const &pos, input_iter_t begin,
			input_iter_t end)
	{
		insert(pos.index(), begin, end);
		return pos;
	}

	iterator insert(iterator const &pos, const_iterator const &begin,
			const_iterator const &end)
	{
		insert(pos.index(), begin, end);
		return pos;
	}

	iterator insert(iterator const &pos, iterator const &begin,
			iterator const &end)
	{
		insert(pos.index(), begin, end);
		return pos;
	}

	void replace(iterator const &begin, iterator const &end, rope const &r)
	{
		replace(begin.index(), end.index() - begin.index(), r);
	}

	void replace(iterator const &begin, iterator const &end, CharType c)
	{
		replace(begin.index(), end.index() - begin.index(), c);
	}

	void replace(iterator const &begin, iterator const &end,
		     CharType const *s)
	{
		replace(begin.index(), end.index() - begin.index(), s);
	}

	template <typename input_iter_t>
	void replace(iterator const &begin, iterator const &end,
		     input_iter_t iter, size_type n)
	{
		replace(begin.index(), end.index() - begin.index(), iter, n);
	}

	template <typename input_iter_t>
	void replace(iterator const &begin, iterator const &end,
		     input_iter_t i_begin, input_iter_t i_end)
	{
		replace(begin.index(), end.index() - begin.index(), i_begin,
			i_end);
	}

	void replace(iterator const &begin, iterator const &end,
		     const_iterator const &i_begin, const_iterator const &i_end)
	{
		replace(begin.index(), end.index() - begin.index(), i_begin,
			i_end);
	}

	void replace(iterator const &begin, iterator const &end,
		     iterator const &i_begin, iterator const &i_end)
	{
		replace(begin.index(), end.index() - begin.index(), i_begin,
			i_end);
	}

	void replace(iterator const &pos, rope const &r)
	{
		replace(pos.index(), r);
	}

	void replace(iterator const &pos, CharType c)
	{
		replace(pos.index(), c);
	}

	void replace(iterator const &pos, CharType const *s)
	{
		replace(pos.index(), s);
	}

	template <typename input_iter_t>
	void replace(iterator const &pos, input_iter_t iter, size_type n)
	{
		replace(pos.index(), iter, n);
	}

	template <typename input_iter_t>
	void replace(iterator const &pos, input_iter_t begin, input_iter_t end)
	{
		replace(pos.index(), begin, end);
	}

	void replace(iterator const &pos, const_iterator begin,
		     const_iterator end)
	{
		replace(pos.index(), begin, end);
	}

	void replace(iterator const &pos, iterator begin, iterator end)
	{
		replace(pos.index(), begin, end);
	}

	iterator erase(iterator const &begin, iterator const &end)
	{
		size_type begin_index(begin.index());
		erase(begin_index, end.index() - begin_index);
		return iterator(this, begin_index);
	}

	iterator erase(iterator const &pos)
	{
		size_type pos_index(pos.index());
		erase(pos_index, 1);
		return iterator(this, pos_index);
	}

	rope substr(size_type pos, size_type n = 1) const
	{
		return rope_type(
			substring(
				root_node, pos, pos + n
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(iterator const &begin, iterator const &end) const
	{
		return rope_type(
			substring(
				root_node, begin.index(),
				end.index()
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(iterator const &pos) const
	{
		size_type pos_index(pos.index());

		return rope_type(
			substring(
				root_node, pos_index, pos_index + 1
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(const_iterator const &begin,
		    const_iterator const &end) const
	{
		return rope_type(
			substring(
				root_node, begin.index(),
				end.index()
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(const_iterator const &pos) const
	{
		size_type pos_index(pos.index());

		return rope_type(
			substring(
				root_node, pos_index,
				pos_index + 1
			),
			std::get<1>(treeplus)
		);
	}
#endif
	size_type find(value_type const &c, size_type pos = 0) const;

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
#if 0
	template <typename CharType1, typename TraitsType1,
		  typename AllocType1, typename Policy1>
	friend rope<CharType1, TraitsType1, AllocType1, Policy1>
	operator+(
		rope<CharType1, TraitsType1, AllocType1, Policy1> const
		&l,
		CharType1 const *s
	);

	template <typename CharType1, typename TraitsType1,
		  typename AllocType1, typename Policy1>
	friend rope<CharType1, TraitsType1, AllocType1, Policy1>
	operator+(
		rope<CharType1, TraitsType1, AllocType1, Policy1> const
		&l,
		CharType1 c
	);

	template <typename CharType1, typename TraitsType1,
		  typename AllocType1, typename Policy1>
	friend std::basic_ostream<CharType1, TraitsType1> &operator<<(
		std::basic_ostream<CharType1, TraitsType1> &os,
		rope<CharType1, TraitsType1, AllocType1, Policy1> const
		&r
	);
#endif
};

template <typename ValueType, typename Policy>
rope<ValueType, Policy> operator+(
	rope<ValueType, Policy> const &l, rope<ValueType, Policy> const &r
)
{
	typedef rope<ValueType, Policy> rope_type;

	return rope_type(rope_type::concat(l.root_node, r.root_node));
}
#if 0
template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> &operator+=(
	rope<CharType, TraitsType, AllocType, Policy> &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	l.append(r);
	return l;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> operator+(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	CharType const *s
)
{
	typedef rope<CharType, TraitsType, AllocType, Policy> rope_type;
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

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> &operator+=(
	rope<CharType, TraitsType, AllocType, Policy> &l,
	CharType const *s
)
{
	l.append(s);
	return l;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> operator+(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	CharType c
)
{
	typedef rope<CharType, TraitsType, AllocType, Policy> rope_type;

	if (std::get<0>(l.treeplus))
		return rope_type(
			l._S_concat_value_iter(std::get<0>(l.treeplus), &c, 1),
			std::get<1>(l.treeplus)
		);
	else
		return rope_type(1, c, std::get<1>(l.treeplus));
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> &operator+=(
	rope<CharType, TraitsType, AllocType, Policy> &l,
	CharType c
)
{
	l.append(c);
	return l;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator<(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) < 0;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator>(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) > 0;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator<=(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) <= 0;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator>=(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) >= 0;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator==(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) == 0;
}

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
bool operator!=(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	return l.compare(r) != 0;
}
#endif

template <typename CharType, typename ValueType, typename Policy>
std::basic_ostream<CharType> &operator<<(
	std::basic_ostream<CharType> &os, rope<ValueType, Policy> const &r
);

}}
#endif

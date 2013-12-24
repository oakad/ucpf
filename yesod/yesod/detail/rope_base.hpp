/*
 * Copyright (c) 2010 - 2013 Alex Dubov <oakad@yahoo.com>
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

#include <functional>
#include <algorithm>
#include <yesod/counted_ptr.hpp>
#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod {

template <
	typename CharType, typename TraitsType, typename AllocType,
	typename Policy
> struct rope {
protected:
	typedef rope<CharType, TraitsType, AllocType, Policy> rope_type;
public:
	typedef TraitsType                            traits_type;
	typedef typename TraitsType::char_type        value_type;
	typedef AllocType                             allocator_type;
	typedef typename AllocType::size_type         size_type;
	typedef typename AllocType::difference_type   difference_type;
	struct                                        reference;
	struct                                        pointer;
	struct                                        const_iterator;
	struct                                        iterator;
	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	static size_type const npos;

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

	static unsigned long const min_len[Policy::max_rope_depth + 1];

	struct rope_rep;
	struct rope_leaf;
	struct rope_concat;
	struct rope_substr;
	struct rope_func;

	typedef counted_ptr<rope_rep>    rope_rep_ptr;
	typedef counted_ptr<rope_leaf>   rope_leaf_ptr;
	typedef counted_ptr<rope_concat> rope_concat_ptr;
	typedef counted_ptr<rope_substr> rope_substr_ptr;
	typedef counted_ptr<rope_func>   rope_func_ptr;

	/*
	 * Piece-wise application of functions to rope nodes may require
	 * access to allocator, which is stored externally to the actual rope
	 * node. This effectively precludes the use of virtual dispatch,
	 * as reference to the enclosing counted_ptr may be required.
	 */
	struct rope_rep_ops {
		bool (*apply)(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		);

		rope_rep_ptr (*substring)(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		);
	};

	static rope_rep_ops const rep_ops[int(rope_tag::last_tag)];

	template <typename rep_type1, typename rep_type2>
	static counted_ptr<rep_type1> rep_cast(counted_ptr<rep_type2> const &p)
	{
		if ((rep_type1::ref_tag == p->tag)
		    || (rep_type1::ref_tag == rope_tag::null))
			return static_pointer_cast<rep_type1>(p);
		else
			return counted_ptr<rep_type1>();
	}

	struct rope_rep {
		static rope_tag const ref_tag = rope_tag::null;

		size_type size;
		rope_tag  tag;
		uint8_t   depth;
		uint8_t   is_balanced;

		static bool apply(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		)
		{
			return false;
		}

		static rope_rep_ptr substring(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		)
		{
			return rope_rep_ptr();
		}

	protected:
		rope_rep(rope_tag tag_, uint8_t d, bool b, size_type size_)
		: size(size_), tag(tag_), depth(d), is_balanced(b ? 1 : 0)
		{}
	};

	static CharType *leaf_data(rope_leaf_ptr const &l)
	{
		return l.template get_extra<CharType>();
	}

	struct rope_leaf : public rope_rep {
		static const rope_tag ref_tag = rope_tag::leaf;

		static size_type rounded_up_size(size_type n)
		{
			// Allow slop for in-place expansion.

			return ((n >> Policy::alloc_granularity_shift) + 1)
			       << Policy::alloc_granularity_shift;
		}

		rope_leaf(size_type n)
		: rope_rep(ref_tag, 0, true, n)
		{}

		static rope_leaf_ptr make(size_type n, AllocType a)
		{
			return allocate_counted<rope_leaf>(
				a,
				typename rope_leaf_ptr::extra_size_t(
					rounded_up_size(n)
				),
				n
			);
		}

		static rope_leaf_ptr make(size_type n, CharType c,
					  AllocType a)
		{
			auto rv(allocate_counted<rope_leaf>(
				a,
				typename rope_leaf_ptr::extra_size_t(
					rounded_up_size(n)
				),
				n
			));
			traits_type::assign(leaf_data(rv), n, c);
			return rv;
		}

		static rope_leaf_ptr make(CharType const *s, size_type n,
					  AllocType a)
		{
			auto rv(allocate_counted<rope_leaf>(
				a,
				typename rope_leaf_ptr::extra_size_t(
					rounded_up_size(n)
				),
				n
			));

			if (n == 1)
				traits_type::assign(*leaf_data(rv), *s);
			else
				traits_type::copy(leaf_data(rv), s, n);

			return rv;
		}

		static bool apply(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		)
		{
			rope_leaf_ptr l(static_pointer_cast<rope_leaf>(r));
			return f(leaf_data(l) + begin, end - begin);
		}

		static rope_rep_ptr substring(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		);
	};

	struct rope_concat : public rope_rep {
		static rope_tag const ref_tag = rope_tag::concat;

		rope_rep_ptr left;
		rope_rep_ptr right;

		rope_concat(rope_rep_ptr const &l, rope_rep_ptr const &r)
		: rope_rep(
			ref_tag,
			std::max(l->depth, r->depth) + 1,
			false, l->size + r->size
		 ), left(l), right(r)
		{}

		static rope_concat_ptr make(
			rope_rep_ptr const &l, rope_rep_ptr const &r,
			AllocType a
		)
		{
			return allocate_counted<rope_concat>(a, l, r);
		}

		static rope_concat_ptr make(
			rope_rep_ptr const &l, rope_rep_ptr const &r
		)
		{
			return make(
				l, r, *l.template get_allocator<AllocType>()
			);
		}

		static bool apply(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		);

		static rope_rep_ptr substring(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		);
	};

	struct rope_substr : public rope_rep {
		static rope_tag const ref_tag = rope_tag::substr;

		rope_rep_ptr base;
		size_type    start;

		rope_substr(rope_rep_ptr const &base_, size_type start_,
			    size_type n)
		: rope_rep(ref_tag, 0, true, n), base(base_), start(start_)
		{}

		static rope_substr_ptr make(
			rope_rep_ptr const &base_, size_type start_,
			size_type n, AllocType a
		)
		{
			return allocate_counted<rope_substr>(a, base_, start_,
							     n);
		}

		static bool apply(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		)
		{
			rope_substr_ptr s(static_pointer_cast<rope_substr>(r));
			return rope_type::apply(
				s->base, f, begin + s->start,
				std::min(s->size, end)
			);
		}

		static rope_rep_ptr substring(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		);
	};

	struct rope_func : public rope_rep {
		static rope_tag const ref_tag = rope_tag::func;
		/* Same argument order as pread (2) */
		typedef std::function<void (CharType *, size_type, size_type)>
		func_type;

		func_type fn;

		rope_func(func_type f, size_type n)
		: rope_rep(ref_tag, 0, true, n), fn(f)
		{}

		static rope_func_ptr make(func_type f, size_type n,
					  AllocType a)
		{
			return allocate_counted<rope_func>(a, f, n);
		}

		/* fn will be evaluated into a temporary leaf node, before
		 * application of f.
		 */
		static bool apply(
			rope_rep_ptr const &r,
			std::function<bool (CharType const *, size_type)> f,
			size_type begin, size_type end
		)
		{
			size_type len(end - begin);
			rope_leaf_ptr l(rope_leaf::make(
				len, *r.template get_allocator<AllocType>()
			));

			static_pointer_cast<rope_func>(r)->fn(
				leaf_data(l), len, begin
			);

			return f(leaf_data(l), len);
		}


		static rope_rep_ptr substring(
			rope_rep_ptr const &r, size_type begin,
			size_type end, size_type adj_end
		);
	};

	struct iterator_base
	{
		/* Iterator value */
		size_type    current_pos;
		/* The whole referenced rope */
		rope_rep_ptr root;
		/* Starting position for the current leaf */
		size_type    leaf_pos;

		/* path_end contains the bottom section of the path from the
		 * root to the current leaf, representing partial path cache.
		 * The path is truncated to keep iterators copying penalty to
		 * the possible minimum.
		 */
		rope_rep_ptr path_end[Policy::path_cache_len];

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
		CharType tmp_buf[Policy::iterator_buf_len];

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

		iterator_base(rope_rep_ptr const &root_, size_type pos)
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

	static bool is_balanced(rope_rep_ptr const &r)
	{
		return (r->size >= min_len[r->depth]);
	}

	static rope_rep_ptr concat_and_set_balanced(rope_rep_ptr const &l,
						    rope_rep_ptr const &r)
	{
		rope_rep_ptr result(concat(l, r));

		if (is_balanced(result))
			result->is_balanced = true;

		return result;
	}

	static void add_leaf_to_forest(rope_rep_ptr const &r,
				       rope_rep_ptr *forest);

	static void add_to_forest(rope_rep_ptr const &r,
				  rope_rep_ptr *forest);

	static rope_rep_ptr balance(rope_rep_ptr const &r);

	static rope_rep_ptr concat(rope_rep_ptr const &l,
				   rope_rep_ptr const &r);

	static rope_rep_ptr tree_concat(rope_rep_ptr const &l,
					rope_rep_ptr const &r);

	template <typename input_iter_t>
	static rope_leaf_ptr leaf_concat_char_iter(
		rope_leaf_ptr const &r, input_iter_t iter, size_type n
	);

	template <typename input_iter_t>
	static rope_rep_ptr concat_char_iter(
		rope_rep_ptr const &r, input_iter_t iter, size_type len
	);

	static CharType *flatten(
		rope_rep_ptr const &r, size_type begin, size_type n,
		CharType *s
	);

	static CharType *flatten(rope_rep_ptr const &r, CharType *s)
	{
		return flatten(r, 0, r->size, s);
	}

	static CharType fetch(rope_rep_ptr const &r, size_type pos);

	static rope_rep_ptr substring(
		rope_rep_ptr const &r, size_type begin, size_type end
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

		return rep_ops[static_cast<size_t>(r->tag)]
		       .substring(r, begin, end, adj_end);
	}

	static bool apply(
		rope_rep_ptr const &r,
		std::function<bool (CharType const *, size_type)> f,
		size_type begin, size_type end
	)
	{
		if (r)
			return rep_ops[static_cast<size_t>(r->tag)]
			       .apply(r, f, begin, end);
		else
			return true;
	}

	static rope_rep_ptr replace(
		rope_rep_ptr const &old, size_type begin, size_type end,
		rope_rep_ptr const &r
	)
	{
		if (!old)
			return r;

		rope_rep_ptr left(substring(old, 0, begin));
		rope_rep_ptr right(substring(old, end, old->size));

		if (!r)
			return concat(left, right);
		else
			return concat(concat(left, r), right);
	}

	int compare(rope_rep_ptr const &left, rope_rep_ptr const &right);

	static std::basic_ostream<CharType, TraitsType> &dump(
		rope_rep_ptr const &r,
		std::basic_ostream<CharType, TraitsType> &os,
		int indent = 0
	);

	std::tuple<rope_rep_ptr, allocator_type> treeplus;

private:
	rope(rope_rep_ptr const &t, AllocType a = AllocType())
	: treeplus(t, a)
	{}
/*
	rope(rope_rep_ptr const &__t, AllocType const &__a)
	: treeplus(__t, __a)
	{}
*/
public:
	struct reference {
		reference(rope_type &r, size_type pos_)
		: root(r), pos(pos_), current_valid(false)
		{}

		reference(reference const &ref)
		: root(ref.root), pos(ref.pos), current_valid(false),
		  current(ref.current)
		{}

		reference(rope_type &r, size_type pos_, CharType c)
		: root(r), pos(pos_), current_valid(true), current(c)
		{}

		operator CharType () const
		{
			if (current_valid)
				return current;
			else
				return rope_type::fetch(
					std::get<0>(root.treeplus), pos
				);
		}

		reference &operator=(CharType c)
		{
			rope_rep_ptr old(std::get<0>(root.treeplus));

			rope_rep_ptr left(rope_type::substring(old, 0, pos));
			rope_rep_ptr right(
				rope_type::substring(old, pos + 1, old->size)
			);

			std::get<0>(root.treeplus) = rope_type::concat(
				rope_type::concat_char_iter(left, &c, 1), right
			);

			return *this;
		}

		pointer operator&() const
		{
			return pointer(*this);
		}

		reference &operator=(reference const &c)
		{
			return operator=(CharType(c));
		}

	private:
		friend struct pointer;

		rope_type &root;
		size_type pos;
		bool      current_valid;
		CharType current;
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

		template <
			typename CharType1, typename TraitsType1,
			typename AllocType1, typename Policy1
		> friend bool operator==(
			typename rope<
				CharType1, TraitsType1, AllocType1, Policy1
			>::pointer const &x,
			typename rope<
				CharType1, TraitsType1, AllocType1, Policy1
			>::pointer const &y
		)
		{
			return (x.pos == y.pos) && (x.root == y.root);
		}

		template <
			typename CharType1, typename TraitsType1,
			typename AllocType1, typename Policy1
		> friend bool operator!=(
			typename rope<
				CharType1, TraitsType1, AllocType1, Policy1
			>::pointer const &x,
			typename rope<
				CharType1, TraitsType1, AllocType1, Policy1
			>::pointer const &y
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

		const_iterator(rope_rep_ptr const &root_, size_type pos_)
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

	allocator_type get_allocator() const
	{
		return std::get<1>(treeplus);
	}

	bool empty() const
	{
		return !(std::get<0>(treeplus));
	}

	int compare(rope const &r) const
	{
		return compare(std::get<0>(treeplus), std::get<0>(r.treeplus));
	}

	rope(CharType const *s, AllocType a = AllocType())
	: treeplus(rope_rep_ptr(), a)
	{
		if (s && traits_type::length(s)) {
			rope_leaf_ptr l(rope_leaf::make(
				s, traits_type::length(s), a
			));
			std::get<0>(treeplus) = l;
		}
	}

	rope(CharType const *s, size_type n, AllocType a = AllocType())
	: treeplus(rope_rep_ptr(), a)
	{
		if (s && n) {
			rope_leaf_ptr l(rope_leaf::make(s, n, a));
			std::get<0>(treeplus) = l;
		}
	}

	template <typename input_iter_t>
	rope(input_iter_t begin, input_iter_t end, AllocType a = AllocType())
	: treeplus(rope_rep_ptr(), a)
	{
		size_type len(end - begin);

		if (len) {
			rope_leaf_ptr l(rope_leaf::make(len, a));
			std::copy(begin, end, l->data);
			std::get<0>(treeplus) = l;
		}
	}

	rope(const_iterator const &begin, const_iterator const &end,
	     AllocType a = AllocType())
	: treeplus(substring(begin.root, begin.current_pos, end.current_pos), a)
	{}

	rope(iterator const &begin, iterator const &end,
	     AllocType a = AllocType())
	: treeplus(substring(begin.root, begin.current_pos, end.current_pos), a)
	{}

	rope(CharType c, AllocType a = AllocType())
	: treeplus(rope_leaf::make(1, c, a), a)
	{}

	rope(size_type n, CharType c, AllocType a = AllocType());

	rope(AllocType a = AllocType())
	: treeplus(rope_rep_ptr(), a)
	{}

	rope(typename rope_func::func_type fn, size_type n,
	     AllocType a = AllocType())
	: treeplus(rope_func::make(fn, n, a), a)
	{}

	rope(rope const &r)
	: treeplus(r.treeplus)
	{}

	rope(rope &&r)
	: treeplus(r.treeplus)
	{
		std::get<0>(r.treeplus).reset();
	}

	~rope()
	{}

	rope &operator=(rope const &r)
	{
		std::get<0>(treeplus) = std::get<0>(r.treeplus);

		return *this;
	}

	rope &operator=(rope &&r)
	{
		std::get<0>(treeplus) = std::get<0>(r.treeplus);
		std::get<0>(r.treeplus).reset();

		return *this;
	}

	void clear()
	{
		std::get<0>(treeplus).reset();
	}

	void push_back(CharType c)
	{
		append(c);
	}

	void pop_back()
	{
		rope_rep_ptr old(std::get<0>(treeplus));

		std::get<0>(treeplus) = substring(old, 0, old->size - 1);
	}

	CharType back() const
	{
		rope_rep_ptr r(std::get<0>(treeplus));

		return fetch(r, r->size - 1);
	}

	void push_front(CharType c)
	{
		rope_rep_ptr old(std::get<0>(treeplus));
		rope_leaf_ptr l(rope_leaf::make(&c, 1, std::get<1>(treeplus)));

		std::get<0>(treeplus) = concat(l, old);
	}

	void pop_front()
	{
		rope_rep_ptr old(std::get<0>(treeplus));

		std::get<0>(treeplus) = substring(old, 1, old->size);
	}

	CharType front() const
	{
		return fetch(std::get<0>(treeplus), 0);
	}

	void balance()
	{
		rope_rep_ptr old(std::get<0>(treeplus));

		std::get<0>(treeplus) = balance(old);
	}

	void copy(CharType *s) const
	{
		flatten(std::get<0>(treeplus), s);
	}

	size_type copy(CharType *s, size_type n, size_type pos = 0) const
	{
		auto len(std::min<difference_type>(size() - pos, n));

		flatten(std::get<0>(treeplus), pos, len, s);
		return len;
	}

	size_type copy(CharType *s, size_type n, const_iterator pos) const
	{
		auto len(std::min<difference_type>(n, cend() - pos));

		flatten(std::get<0>(treeplus), pos.current_pos, len, s);
		return len;
	}

	struct rope_dumper {
		friend struct rope;
		rope_rep_ptr r;

		rope_dumper(rope_rep_ptr const &r_)
		: r(r_)
		{}

		std::basic_ostream<CharType, TraitsType> &dump(
			std::basic_ostream<CharType, TraitsType> &os
		) const
		{
			return rope_type::dump(r, os);
		}

		friend std::basic_ostream<CharType, TraitsType> &operator<<(
			std::basic_ostream<CharType, TraitsType> &os,
			rope_dumper const &d
		)
		{
			return d.dump(os);
		}
	};

	rope_dumper dump()
	{
		 return rope_dumper(std::get<0>(treeplus));
	}

	std::basic_string<CharType, TraitsType, AllocType> str() const
	{
		std::basic_string<CharType, TraitsType, AllocType> s;
		s.reserve(size());
		copy(s.data());
		return s;
	}

	void flatten()
	{
		rope_leaf_ptr l(rep_cast<rope_leaf>(std::get<0>(treeplus)));

		if (!l && std::get<0>(treeplus)) {
			l = rope_leaf::make(size(), std::get<1>(treeplus));
			copy(l->data);
			std::get<0>(treeplus) = l;
		}
	}

	CharType operator[](size_type pos) const
	{
		return fetch(std::get<0>(treeplus), pos);
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
		return const_iterator(std::get<0>(treeplus), 0);
	}

	const_iterator cend() const
	{
		return const_iterator(std::get<0>(treeplus), size());
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
		if (std::get<0>(treeplus))
			return std::get<0>(treeplus)->size;
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
		 return min_len[Policy::max_rope_depth - 1] - 1;
	}

	rope &operator+=(rope const &r)
	{
		return append(r);
	}

	rope &operator+=(CharType const *s)
	{
		return append(s);
	}

	rope &operator+=(CharType c)
	{
		return append(c);
	}

	template <typename input_iter_t>
	rope &append(input_iter_t iter, size_type n)
	{
		if (std::get<0>(treeplus))
			std::get<0>(treeplus) = concat_char_iter(
				std::get<0>(treeplus), iter, n
			);
		else
			std::get<0>(treeplus) = rope_leaf::make(
				iter, n, std::get<1>(treeplus)
			);

		return *this;
	}

	rope &append(CharType const *s)
	{
		return append(s, traits_type::length(s));
	}

	template <typename input_iter_t>
	rope &append(input_iter_t begin, input_iter_t end)
	{
		return append(begin, end - begin);
	}

	rope &append(const_iterator first, const_iterator last)
	{
		std::get<0>(treeplus) = concat(
			std::get<0>(treeplus),
			substring(
				first.root, first.current_pos, last.current_pos
			)
		);

		return *this;
	}

	rope &append(CharType c)
	{
		return append(&c, 1);
	}

	rope &append(rope const &r)
	{
		std::get<0>(treeplus) = concat(
			std::get<0>(treeplus), std::get<0>(r.treeplus)
		);
		return *this;
	}

	rope &append(size_type n, CharType c)
	{
		return append(rope_type(n, c, std::get<1>(treeplus)));
	}

	void swap(rope &r)
	{
		treeplus.swap(r.treeplus);
	}

	void insert(size_type pos, rope const &r)
	{
		std::get<0>(treeplus) = replace(
			std::get<0>(treeplus), pos, pos, std::get<0>(r.treeplus)
		);
	}

	void insert(size_type pos, size_type n, CharType c)
	{
		rope_type r(n, c, std::get<1>(treeplus));
		insert(pos, r);
	}

	template <typename input_iter_t>
	void insert(size_type pos, input_iter_t iter, size_type n)
	{
		rope_rep_ptr left(std::get<0>(treeplus), 0, pos);
		rope_rep_ptr right(std::get<0>(treeplus), pos, size());

		std::get<0>(treeplus) = concat(
			concat_char_iter(left, iter, n), right
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
		std::get<0>(treeplus) = replace(
			std::get<0>(treeplus), pos, pos + n,
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
		std::get<0>(treeplus) = replace(
			std::get<0>(treeplus), pos, pos + n, rope_rep_ptr()
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
				std::get<0>(treeplus), pos, pos + n
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(iterator const &begin, iterator const &end) const
	{
		return rope_type(
			substring(
				std::get<0>(treeplus), begin.index(),
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
				std::get<0>(treeplus), pos_index, pos_index + 1
			),
			std::get<1>(treeplus)
		);
	}

	rope substr(const_iterator const &begin,
		    const_iterator const &end) const
	{
		return rope_type(
			substring(
				std::get<0>(treeplus), begin.index(),
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
				std::get<0>(treeplus), pos_index,
				pos_index + 1
			),
			std::get<1>(treeplus)
		);
	}

	size_type find(CharType c, size_type pos = 0) const;

	size_type find(CharType const *s, size_type pos = 0) const;

	reference mutable_reference_at(size_type pos)
	{
		return reference(this, pos);
	}

	reference operator[] (size_type pos)
	{
		return reference(this, pos);
	}

	template <typename CharType1, typename TraitsType1,
		  typename AllocType1, typename Policy1>
	friend rope<CharType1, TraitsType1, AllocType1, Policy1>
	operator+(
		rope<CharType1, TraitsType1, AllocType1, Policy1> const
		&l,
		rope<CharType1, TraitsType1, AllocType1, Policy1> const
		&r
	);

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
};

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
rope<CharType, TraitsType, AllocType, Policy> operator+(
	rope<CharType, TraitsType, AllocType, Policy> const &l,
	rope<CharType, TraitsType, AllocType, Policy> const &r
)
{
	typedef rope<CharType, TraitsType, AllocType, Policy> rope_type;

	return rope_type(
		rope_type::concat(std::get<0>(l.treeplus),
				  std::get<0>(r.treeplus)),
		std::get<1>(l.treeplus)
	);
}

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
			l.concat_char_iter(std::get<0>(l.treeplus), s, s_len),
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
			l._S_concat_char_iter(std::get<0>(l.treeplus), &c, 1),
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

template <typename CharType, typename TraitsType, typename AllocType,
	  typename Policy>
std::basic_ostream<CharType, TraitsType> &operator<<(
	std::basic_ostream<CharType, TraitsType> &os,
	rope<CharType, TraitsType, AllocType, Policy> const &r
);

}}
#endif

/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_PARDES_INTERNAL_BASIC_BTREE_BASE_HPP)
#define _PARDES_INTERNAL_BASIC_BTREE_BASE_HPP

#include <pardes/internal/restricted_array.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/set.hpp>
#include <ext/functional>

namespace pardes
{

template <typename key_type, typename value_type>
struct basic_btree_default_traits {
	typedef std::less<key_type>             key_compare_t;
	typedef __gnu_cxx::identity<value_type> key_of_val_t;
	typedef std::allocator<value_type>      allocator_type;

	static std::size_t const arity = 32;
};

template <
	typename key_type_, typename value_type_,
	typename traits_type = basic_btree_default_traits<
		key_type_, value_type_
	>
> struct basic_btree {
	typedef key_type_                                key_type;
	typedef value_type_                              value_type;
	typedef typename traits_type::key_compare_t      key_compare_t;
	typedef typename traits_type::key_of_val_t       key_of_val_t;
	typedef typename traits_type::allocator_type     allocator_type;
	typedef typename allocator_type::size_type       size_type;
	typedef typename allocator_type::difference_type difference_type;

	typedef boost::fusion::set<key_compare_t, key_of_val_t, allocator_type>
	support_type;

private:
	struct branch_node;
	struct leaf_node;

public:
	template <typename iter_type>
	struct iterator_base : public boost::iterator_facade <
		iterator_base<iter_type>, iter_type,
		boost::bidirectional_traversal_tag
	> {
	private:
		friend struct basic_btree;
		friend struct boost::iterator_core_access;
		struct enabler {};
		typedef typename restricted_array<
			value_type, traits_type::arity
		>::template iterator_base<iter_type> base_iterator;

	public:
		iterator_base()
		: n(nullptr)
		{}

		template <typename iter_type1>
		iterator_base(
			iterator_base<iter_type1> const &other,
			typename std::enable_if<
				std::is_convertible<
					iter_type1 *, iter_type *
				>::value, enabler
			>::type = enabler()
		) : n(other.n), pos(other.pos)
		{}

	private:
		iterator_base(leaf_node const *n_, base_iterator pos_)
		: n(n_), pos(pos_)
		{}

		template <typename iter_type1>
		bool equal(iterator_base<iter_type1> const &other) const
		{
			return (n == other.n) && (pos == other.pos);
		}

		void increment();

		void decrement();

		iter_type &dereference() const
		{
			return *pos;
		}

		leaf_node const *n;
		base_iterator pos;
	};

	typedef iterator_base<value_type>             iterator;
	typedef iterator_base<value_type const>       const_iterator;
	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	basic_btree()
	: height(0), val_count(0), node_count(
		boost::fusion::make_pair<branch_node>(0),
		boost::fusion::make_pair<leaf_node>(0)
	)
	{}

	~basic_btree()
	{
		clear();
	}

	iterator begin();

	iterator end();

	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	const_iterator begin() const
	{
		return cbegin();
	}

	const_iterator end() const
	{
		return cend();
	}

	const_iterator cbegin() const;

	const_iterator cend() const;

	const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}

	const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}

	iterator insert_equal(value_type const &v);

	std::pair<iterator, bool>
	insert_unique(value_type const &v);

	const_iterator lower_bound(key_type const &k) const;

	void clear();

	template <
		typename char_type,
		typename char_traits_t = std::char_traits<char_type>
	> static std::basic_ostream<char_type, char_traits_t> &dump(
		basic_btree t, std::basic_ostream<char_type, char_traits_t> &os,
		std::function<decltype(os) & (key_type const &, decltype(os) &)>
		key_dumper,
		std::function<decltype(os) & (key_type const &, decltype(os) &)>
		value_dumper
	);
private:
	union node_ptr {
		node_ptr()
		: b(nullptr)
		{}

		node_ptr(branch_node *b_)
		: b(b_)
		{}

		node_ptr(leaf_node *l_)
		: l(l_)
		{}

		struct branch_node *b;
		struct leaf_node   *l;
	};

	struct branch_node {
		typedef std::pair<key_type, node_ptr> node_value_type;
		typedef typename allocator_type::template rebind<branch_node>
		::other node_alloc_type;

		restricted_array<key_type, traits_type::arity> k;
		restricted_array<node_ptr, traits_type::arity + 1> c;

		typedef typename decltype(c)::const_iterator const_iterator;

		branch_node const *parent;
		const_iterator parent_pos;

		const_iterator lower_bound(
			key_type const &key, support_type const &sup
		) const;

		branch_node *left_stem(size_type &h) const;
		branch_node *right_stem(size_type &h) const;
	};

	struct leaf_node {
		typedef value_type node_value_type;
		typedef typename allocator_type::template rebind<leaf_node>
		::other node_alloc_type;

		restricted_array<node_value_type, traits_type::arity> c;

		typedef typename decltype(c)::const_iterator const_iterator;

		branch_node const *parent;
		typename branch_node::const_iterator parent_pos;

		const_iterator lower_bound(
			key_type const &key, support_type const &sup
		) const;
	};

	template <typename node_type>
	node_type *create_node()
	{
		typename node_type::node_alloc_type a(
			boost::fusion::at_key<allocator_type>(sup)
		);
		auto p(a.allocate(1));
		a.construct(p);
		++boost::fusion::at_key<node_type>(node_count);
		return p;
	}

	template <typename node_type>
	void destroy_node(node_type *p)
	{
		typename node_type::node_alloc_type a(
			boost::fusion::at_key<allocator_type>(sup)
		);
		a.destroy(p);
		a.deallocate(p, 1);
		--boost::fusion::at_key<node_type>(node_count);
	}

	branch_node *split_1(branch_node *n, key_type const &key);
	leaf_node *split_1(leaf_node *n, key_type const &key);

	leaf_node *select_leaf(key_type const &key);

	void clear_r(branch_node *b, size_type h);

	node_ptr     root;              /**< Root of the tree */
	size_type    height;            /**< Height of the tree */
	size_type    val_count;         /**< Number of valid keys in the tree */
	/** Number of allocated nodes by type */
	boost::fusion::map<
		boost::fusion::pair<branch_node, size_type>,
		boost::fusion::pair<leaf_node, size_type>
	> node_count;
	support_type sup;               /**< Btree customization parameters */
};

}

#endif

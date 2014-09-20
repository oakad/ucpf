/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_SPARSE_VECTOR_BASE_20140106T1320)
#define UCPF_YESOD_DETAIL_SPARSE_VECTOR_BASE_20140106T1320

#include <ostream>
#include <yesod/bitops.hpp>
#include <yesod/detail/placement_array.hpp>

namespace ucpf { namespace yesod {
namespace detail {

template <typename T>
struct has_value_valid_pred {
	template <typename U>
	struct type_wrapper {
		typedef U type;
	};

	template <typename U>
	static std::true_type test(
		type_wrapper<U> const volatile *,
		type_wrapper<typename U::value_valid_pred> * = 0
	);

	static std::false_type test(...);

	typedef decltype(
		test(static_cast<type_wrapper<T> *>(nullptr))
	) type;

	static const bool value = type::value;
};

template <typename Policy, bool HasPredicate = false>
struct sparse_vector_value_predicate {
	typedef void type;
};

template <typename Policy>
struct sparse_vector_value_predicate<Policy, true> {
	typedef typename Policy::value_valid_pred type;
};

}

template <
	typename ValueType, typename Policy
> struct sparse_vector {
	typedef ValueType value_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::pointer pointer;
	typedef typename allocator_traits::const_pointer const_pointer;
	typedef typename allocator_traits::size_type size_type;

	sparse_vector()
	: root(nullptr), aux(0, allocator_type())
	{}

	~sparse_vector()
	{
		clear();
	}

	void swap(sparse_vector &other)
	{
		std::swap(root, other.root);

		if (allocator_traits::propagate_on_container_swap::value)
			std::swap(aux, other.aux);
		else
			std::swap(std::get<0>(aux), std::get<0>(aux));
	}

	void clear();

	bool empty() const
	{
		return !std::get<0>(aux);
	}

	template <typename Pred>
	bool for_each_above(size_type pos, Pred &&pred);

	template <typename Pred>
	bool for_each_above(size_type pos, Pred &&pred) const;

	template <typename Pred>
	void remove_if(Pred pred);

	reference operator[](size_type pos)
	{
		auto &node(*data_node_at(pos));
		return node[pos];
	}

	const_reference operator[](size_type pos) const
	{
		auto &node(*data_node_at(pos));
		return node[pos];
	}

	pointer ptr_at(size_type pos)
	{
		auto node(data_node_at(pos));
		return node ? node->ptr_at(pos) : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto node(data_node_at(pos));
		return node ? node->ptr_at(pos) : nullptr;
	}

	template <typename... Args>
	reference emplace_at(size_type pos, Args&&... args)
	{
		auto node(data_node_alloc_at(pos));
		return node->emplace_at(
			std::get<1>(aux), pos, std::forward<Args>(args)...
		);
	}

	size_type find_empty_above(size_type pos) const;

	allocator_type get_allocator() const
	{
		return std::get<1>(aux);
	}

	template <typename CharType, typename Traits>
	auto dump(
		std::basic_ostream<CharType, Traits> &os
	) const -> std::basic_ostream<CharType, Traits> &;

private:
	struct node_type {
		virtual ~node_type()
		{}
	};

	template <std::size_type...>
	struct data_node;

	template <std::size_type OrdId>
	struct data_node<OrdId, OrdId> : node_type {
	};

	template <std::size_type OrdId, std::size_type MaxOrdId>
	struct data_node<OrdId, MaxOrdId> : node_type {
	};

	template <std::size_type...>
	struct ptr_node;

	template <std::size_type OrdId>
	struct ptr_node<OrdId, OrdId> : node_type {
	};

	template <std::size_type OrdId, std::size_type MaxOrdId>
	struct ptr_node<Order, MaxOrdId> : node_type {
	};

	struct loc_pair {
		node_type *ptr;
		size_type off;
	};

	constexpr static size_type data_node_order
	= Policy::data_node_order.back();

	constexpr static size_type data_node_size = 1 << data_node_order;

	constexpr static size_type ptr_node_order
	= Policy::ptr_node_order.back();

	constexpr static size_type ptr_node_size = 1 << ptr_node_order;

	node_type *root;
	std::tuple<size_type, allocator_type> aux;
};

}}
#endif

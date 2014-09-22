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
#include <yesod/detail/compressed_array.hpp>

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
	: root(nullptr), tup_height_alloc(0, allocator_type())
	{}

	~sparse_vector()
	{
		//clear();
	}

	void swap(sparse_vector &other)
	{
		std::swap(root, other.root);

		if (allocator_traits::propagate_on_container_swap::value)
			std::swap(tup_height_alloc, other.tup_height_alloc);
		else
			std::swap(
				std::get<0>(tup_height_alloc),
				std::get<0>(other.tup_height_alloc)
			);
	}

	void clear();

	bool empty() const
	{
		return !root;
	}

	allocator_type get_allocator() const
	{
		return std::get<1>(tup_height_alloc);
	}

	template <typename CharType, typename Traits>
	auto dump(
		std::basic_ostream<CharType, Traits> &os
	) const -> std::basic_ostream<CharType, Traits> &;

private:
	struct node_base {
		virtual ~node_base()
		{}
	};

	typedef node_base *node_ptr;

	struct loc_pair {
		node_ptr ptr;
		size_type off;
	};

	template <
		typename ValueType0, typename ValueValidPred,
		std::size_t OrdId, std::size_t MaxOrdId,
		std::array<std::size_t, MaxOrdId> const *arr
	> struct node : node_base {
		typedef ValueType0 value_type;

		typedef node<
			value_type, ValueValidPred, OrdId, MaxOrdId, arr
		> this_node_type;

		constexpr static std::size_t prev_ord_id
		= OrdId ? OrdId - 1 : OrdId;

		constexpr static std::size_t next_ord_id
		= OrdId < (MaxOrdId - 1) ? OrdId + 1 : OrdId;

		typedef node<
			value_type, ValueValidPred, prev_ord_id, MaxOrdId, arr
		> prev_node_type;

		typedef node<
			value_type, ValueValidPred, next_ord_id, MaxOrdId, arr
		> next_node_type;

		constexpr static std::size_t apparent_order = (*arr)[0];
		constexpr static std::size_t real_order = (*arr)[OrdId];

		static_assert(
			std::is_same<prev_node_type, this_node_type>::value
			|| (prev_node_type::real_order > real_order),
			"prev_node_type::real_order > real_order"
		);

		static_assert(
			std::is_same<next_node_type, this_node_type>::value
			|| (next_node_type::real_order < real_order),
			"next_node_type::real_order < real_order"
		);

		detail::compressed_array<
			value_type, apparent_order, real_order, ValueValidPred
		> items;
	};

	struct ptr_valid_pred {
		bool test(node_ptr v)
		{
			return v != nullptr;
		}
	};

	typedef node<
		node_ptr, ptr_valid_pred, 0, Policy::ptr_node_order.size(),
		&Policy::ptr_node_order
	> ptr_node_type;

	
	typedef node<
		value_type, 
		typename detail::sparse_vector_value_predicate<
			Policy, detail::has_value_valid_pred<Policy>::value
		>::type, 0, Policy::data_node_order.size(),
		&Policy::data_node_order
	> data_node_type;

	node_ptr root;
	std::tuple<size_type, allocator_type> tup_height_alloc;
};

}}
#endif

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
#include <yesod/detail/static_bit_field_map.hpp>

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

	pointer ptr_at(size_type pos)
	{
		auto h(std::get<0>(tup_height_alloc));
		node_ptr *p(&root);

		while (h > 1) {
			p = static_cast<ptr_node_base *>(*p)->ptr_at(
				node_offset(pos, h)
			);

			if (!p)
				return nullptr;

			--h;
		}

		if (*p) {
			 return static_cast<data_node_base *>(*p)->ptr_at(
				node_offset(pos, h)
			);
		}
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto h(std::get<0>(tup_height_alloc));
		node_ptr *p(&root);

		while (h > 1) {
			p = static_cast<ptr_node_base *>(*p)->ptr_at(
				node_offset(pos, h)
			);

			if (!p)
				return nullptr;

			--h;
		}

		if (*p) {
			 return static_cast<data_node_base *>(*p)->ptr_at(
				node_offset(pos, h)
			);
		}
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

	struct ptr_node_base : node_base {
		typedef node_ptr node_value_type;
		struct value_valid_pred {
			bool test(node_ptr v)
			{
				return v != nullptr;
			}
		};

		virtual node_value_type *ptr_at(size_type pos) = 0;
	};

	struct data_node_base : node_base {
		typedef value_type node_value_type;
		typedef typename detail::sparse_vector_value_predicate<
			Policy, detail::has_value_valid_pred<Policy>::value
		>::type value_valid_pred;

		virtual node_value_type *ptr_at(size_type pos) = 0;
	};

	template <
		typename NodeBase, size_type OrdId, size_type MaxOrdId,
		std::array<std::size_t, MaxOrdId> const &arr
	> struct node : NodeBase {
		typedef NodeBase base_type;
		typedef node<base_type, OrdId, MaxOrdId, arr> self_type;

		constexpr static size_type prev_ord_id
		= OrdId ? OrdId - 1 : OrdId;

		constexpr static size_type next_ord_id
		= OrdId < (MaxOrdId - 1) ? OrdId + 1 : OrdId;

		typedef node<
			base_type, prev_ord_id, MaxOrdId, arr
		> prev_node_type;

		typedef node<
			base_type, next_ord_id, MaxOrdId, arr
		> next_node_type;

		constexpr static size_type apparent_order = arr[MaxOrdId - 1];
		constexpr static size_type real_order = arr[OrdId];

		static_assert(
			std::is_same<prev_node_type, self_type>::value
			|| (prev_node_type::real_order < real_order),
			"prev_node_type::real_order < real_order"
		);

		static_assert(
			std::is_same<next_node_type, self_type>::value
			|| (next_node_type::real_order > real_order),
			"next_node_type::real_order > real_order"
		);

		virtual typename base_type::node_value_type *ptr_at(
			size_type pos
		)
		{
		}

		detail::compressed_array<
			typename base_type::node_value_type, apparent_order,
			real_order, typename base_type::value_valid_pred
		> items;
	};

	typedef node<
		ptr_node_base, 0, Policy::ptr_node_order.size(),
		Policy::ptr_node_order
	> ptr_node_type;

	typedef node<
		data_node_base, 0, Policy::data_node_order.size(),
		Policy::data_node_order
	> data_node_type;

	typedef typename detail::static_bit_field_map<
		sizeof(size_type) * 8, data_node_type::apparent_order,
		ptr_node_type::apparent_order
	>::repeat_last::value_type pos_field_map;

	constexpr static size_type node_offset(size_type pos, size_type h)
	{
		return (pos >> pos_field_map::value[h - 1].second) & ((
			size_type(1) << pos_field_map::value[h - 1].first
		) - 1);
	}

	node_ptr root;
	std::tuple<size_type, allocator_type> tup_height_alloc;
};

}}
#endif

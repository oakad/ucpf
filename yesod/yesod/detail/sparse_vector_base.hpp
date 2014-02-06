/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_SPARSE_VECTOR_BASE_JAN_06_2014_1320)
#define UCPF_YESOD_DETAIL_SPARSE_VECTOR_BASE_JAN_06_2014_1320

#include <yesod/detail/placement_array.hpp>
#include <yesod/mpl/detail/type_wrapper.hpp>

namespace ucpf { namespace yesod {
namespace detail {

using ucpf::yesod::mpl::detail::type_wrapper;

template <typename T>
struct has_value_valid_pred {
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
	: height(0), root_node(nullptr, allocator_type())
	{}

	~sparse_vector()
	{
		clear();
	}

	void swap(sparse_vector &other)
	{
		std::swap(height, other.height);

		if (allocator_traits::propagate_on_container_swap::value)
			std::swap(root_node, other.root_node);
		else
			std::swap(
				std::get<0>(root_node),
				std::get<0>(other.root_node)
			);
	}

	void clear()
	{
		destroy_node_r(std::get<0>(root_node), height);
		std::get<0>(root_node) = nullptr;
		height = 0;
	}

	bool empty() const
	{
		return !height;
	}

	bool for_each(
		std::function<bool (size_type, const_reference)> &&f
	) const;

	template <typename Pred>
	void remove_if(Pred pred);

	reference operator[](size_type pos)
	{
		auto &node(*data_node_at(pos));
		return node[node_offset(pos, 1)];
	}

	const_reference operator[](size_type pos) const
	{
		auto &node(*data_node_at(pos));
		return node[node_offset(pos, 1)];
	}

	pointer ptr_at(size_type pos)
	{
		auto node(data_node_at(pos));
		return node ? node->ptr_at(node_offset(pos, 1)) : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto node(data_node_at(pos));
		return node ? node->ptr_at(node_offset(pos, 1)) : nullptr;
	}

	reference at(size_type pos)
	{
		auto node(data_node_alloc_at(pos));
		printf("at %p\n", node);
		return node->at(
			node_offset(pos, 1), std::get<1>(root_node)
		);
	}

	template <typename... Args>
	reference emplace_at(size_type pos, Args&&... args)
	{
		auto node(data_node_alloc_at(pos));
		return node->emplace_at(
			node_offset(pos, 1), std::get<1>(root_node),
			std::forward<Args>(args)...
		);
	}

	allocator_type get_allocator() const
	{
		return std::get<1>(root_node);
	}

private:
	typedef typename allocator_traits::void_pointer node_pointer;

	typedef detail::placement_array<
		node_pointer, (size_t(1) << Policy::ptr_node_order),
		typename Policy::allocator_type, void
	> ptr_node;

	typedef detail::placement_array<
		ValueType, (size_t(1) << Policy::data_node_order),
		typename Policy::allocator_type,
		typename detail::sparse_vector_value_predicate<
			Policy, detail::has_value_valid_pred<Policy>::value
		>::type
	> data_node;

	size_type height;
	std::tuple<node_pointer, allocator_type> root_node;

	static size_type node_offset(size_type pos, size_type h)
	{
		auto l_pos(pos & ((1UL << Policy::data_node_order) - 1));
		if (h == 1)
			return l_pos;

		pos >>= Policy::data_node_order;
		return (pos >> (Policy::ptr_node_order * (h - 2)))
		       & ((1UL << Policy::ptr_node_order) - 2);
	}

	bool for_each_impl(
		ptr_node const *p, size_type h, size_type &offset,
		std::function<bool (size_type, const_reference)> &&f
	) const;

	void destroy_node_r(node_pointer p_, size_type h);
	data_node *data_node_at(size_type pos);
	data_node const *data_node_at(size_type pos) const;
	data_node *data_node_alloc_at(size_type pos);
};

}}
#endif

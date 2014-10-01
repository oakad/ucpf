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

	sparse_vector(allocator_type const &a = allocator_type())
	: root(a), tup_height_alloc(0, a)
	{}

	~sparse_vector()
	{
		clear();
	}

	void swap(sparse_vector &other)
	{
		std::swap(root.item, other.root.item);

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
		return !root.item;
	}

	pointer ptr_at(size_type pos)
	{
		auto h(std::get<0>(tup_height_alloc));
		ptr_node_base *p(&root);
		size_type d_pos(0);

		while (h > 1) {
			auto q(p->ptr_at(d_pos));
			if (!q)
				return nullptr;

			p = static_cast<ptr_node_base *>(*q);
			d_pos = node_offset(pos, h);
			--h;
		}

		auto q(p->ptr_at(d_pos));
		if (!q)
			return nullptr;

		return static_cast<data_node_base *>(*q)->ptr_at(
			node_offset(pos, 1)
		);
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto h(std::get<0>(tup_height_alloc));
		ptr_node_base *p(&root);
		size_type d_pos(0);

		while (h > 1) {
			auto q(p->ptr_at(d_pos));
			if (!q)
				return nullptr;

			p = static_cast<ptr_node_base *>(*q);
			d_pos = node_offset(pos, h);
			--h;
		}

		auto q(p->ptr_at(d_pos));
		if (!q)
			return nullptr;

		return static_cast<data_node_base *>(*q)->ptr_at(
			node_offset(pos, 1)
		);
	}

	template <typename... Args>
	pointer emplace_at(
		size_type pos, Args&&... args
	)
	{
		auto d_pos(node_offset(pos, 1));
		auto pp(alloc_data_node_at(pos));
		printf("--1- %zd: %p - %p (%p)\n", d_pos, pp.first, pp.second, *pp.second);
		auto qp(pp.first->reserve_at(d_pos));
		printf("-a1- %p, %d\n", qp.first, qp.second);
		if (!qp.first) {
			pp.first = static_cast<data_node_base *>(
				pp.first->grow_node(
					std::get<1>(tup_height_alloc),
					pp.second
				)
			);
			qp = pp.first->reserve_at(d_pos);
		}

		if (qp.second) {
			*qp.first = std::move(
				value_type(std::forward<Args>(args)...)
			);
			return qp.first;
		}

		auto release = [pp, d_pos](pointer v) -> void {
			pp.first->release_at(d_pos);
		};

		std::unique_ptr<
			value_type, decltype(release)
		> q(qp.first, release);
		allocator::array_helper<value_type, allocator_type>::make_n(
			std::get<1>(tup_height_alloc), qp.first, 1,
			std::forward<Args>(args)...
		);
		printf("-b1-\n");
		q.release();
		return qp.first;
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred);

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred) const;

	size_type find_vacant(size_type first) const;

	allocator_type get_allocator() const
	{
		return std::get<1>(tup_height_alloc);
	}

	std::ostream &dump(std::ostream &os) const;

private:
	struct node_base {
		virtual ~node_base()
		{}

		virtual void destroy(allocator_type const &a) = 0;
		virtual void release_at(size_type pos) = 0;
		virtual size_type find_vacant(size_type first) const = 0;
		virtual node_base *grow_node(
			allocator_type const &a, node_base **parent
		) = 0;
	};

	struct ptr_node_base : node_base {
		typedef node_base *node_value_type;
		struct value_valid_pred {
			static bool test(node_base *v)
			{
				return v != nullptr;
			}
		};

		virtual node_value_type *ptr_at(size_type pos) = 0;
		virtual std::pair<
			node_value_type *, size_type
		> find_occupied(size_type first) = 0;
		virtual std::pair<
			node_value_type const *, size_type
		> find_occupied(size_type first) const = 0;
		virtual std::pair<
			node_value_type *, bool
		> reserve_at(size_type pos) = 0;
	};

	struct loc_pair {
		ptr_node_base *ptr;
		size_type pos;
	};

	struct c_loc_pair {
		ptr_node_base const *ptr;
		size_type pos;
	};

	struct data_node_base : node_base {
		typedef value_type node_value_type;
		typedef typename detail::sparse_vector_value_predicate<
			Policy, detail::has_value_valid_pred<Policy>::value
		>::type value_valid_pred;

		virtual node_value_type *ptr_at(size_type pos) = 0;
		virtual std::pair<
			node_value_type *, size_type
		> find_occupied(size_type first) = 0;
		virtual std::pair<
			node_value_type const *, size_type
		> find_occupied(size_type first) const = 0;
		virtual std::pair<
			node_value_type *, bool
		> reserve_at(size_type pos) = 0;
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
		constexpr static size_type apparent_size
		= size_type(1) << apparent_order;

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

		node(allocator_type const &a)
		{
			items.init(a);
		}

		virtual void destroy(allocator_type const &a)
		{
			typedef allocator::array_helper<
				self_type, allocator_type
			> a_h;

			items.destroy(a);
			a_h::destroy(a, this, 1, true);
		}

		virtual typename base_type::node_value_type *ptr_at(
			size_type pos
		)
		{
			return items.ptr_at(pos);
		}

		virtual std::pair<
			typename base_type::node_value_type *, size_type
		> find_occupied(size_type first)
		{
			std::pair<
				typename base_type::node_value_type *,
				size_type
			> rv(nullptr, items.find_occupied(first));

			if (rv.second != items.size())
				rv.first = items.ptr_at(rv.second);

			return rv;
		}

		virtual std::pair<
			typename base_type::node_value_type const *,
			size_type
		> find_occupied(size_type first) const
		{
			std::pair<
				typename base_type::node_value_type const *,
				size_type
			> rv(nullptr, items.find_occupied(first));

			if (rv.second != items.size())
				rv.first = items.ptr_at(rv.second);

			return rv;
		}

		virtual size_type find_vacant(size_type first) const
		{
			return items.find_vacant(first);
		}

		virtual std::pair<
			typename base_type::node_value_type *, bool
		> reserve_at(size_type pos)
		{
			return items.reserve_at(pos);
		}

				
		virtual void release_at(size_type pos)
		{
			return items.release_at(pos);
		}

		virtual node_base *grow_node(
			allocator_type const &a, node_base **parent
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

	constexpr static size_type node_size(size_type h)
	{
		return size_type(1) << pos_field_map::value[h - 1].first;
	}

	constexpr static size_type node_offset(size_type pos, size_type h)
	{
		return (pos >> pos_field_map::value[h - 1].second) & (
			node_size(h) - 1
		);
	}

	constexpr static size_type height_at_pos(size_type pos)
	{
		return (pos >= data_node_type::apparent_size) ? (
			(yesod::fls(pos) - data_node_type::apparent_order)
			/ ptr_node_type::apparent_order + 2
		) : 1;
	}

	std::pair<
		data_node_base *, node_base **
	> alloc_data_node_at(size_type pos);

	void tree_loc_at(c_loc_pair *tree_loc, size_type pos) const;
	size_type tree_loc_pos(c_loc_pair *tree_loc) const;
	void tree_loc_next(c_loc_pair *tree_loc) const;
	bool tree_loc_next_valid(c_loc_pair *tree_loc) const;

	constexpr static std::array<std::size_t, 1> root_node_order = {{0}};

	node<
		ptr_node_base, 0, 1, root_node_order
	> root;
	std::tuple<size_type, allocator_type> tup_height_alloc;
};

}}
#endif

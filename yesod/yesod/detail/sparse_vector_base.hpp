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

template <typename...>
struct sparse_vector;

template <
	typename ValueType, typename Policy
> struct sparse_vector<ValueType, Policy> {
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
	: root(allocator_type()), tup_height_alloc(0, allocator_type())
	{}

	template <typename Alloc>
	sparse_vector(Alloc const &a = Alloc())
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

	bool erase(size_type pos)
	{
		auto h(std::get<0>(tup_height_alloc));
		ptr_node_base *p(&root);
		size_type d_pos(0);

		while (h > 1) {
			auto q(p->ptr_at(d_pos));
			if (!q)
				return false;

			p = static_cast<ptr_node_base *>(*q);
			d_pos = node_offset(pos, h);
			--h;
		}

		auto q(p->ptr_at(d_pos));
		if (!q)
			return false;

		return static_cast<data_node_base *>(*q)->erase(
			std::get<1>(tup_height_alloc), node_offset(pos, 1)
		);
	}

	reference operator[](size_type pos)
	{
		return *ptr_at(pos);
	}

	const_reference operator[](size_type pos) const
	{
		return *ptr_at(pos);
	}

	template <typename... Args>
	pointer emplace(size_type pos, Args&&... args)
	{
		auto p(alloc_data_node_at(pos));
		auto q(p.data_ref());
		auto d_pos(node_offset(pos, 1));
		auto qp(q->reserve_at(d_pos));

		if (!qp.first) {
			q = static_cast<data_node_base *>(q->grow_node(
				std::get<1>(tup_height_alloc), p, false
			));
			qp = q->reserve_at(d_pos);
		}

		if (qp.second)
			return qp.first;

		auto release = [d_pos](data_node_base *q) -> void {
			q->release_at(d_pos);
		};

		std::unique_ptr<
			data_node_base, decltype(release)
		> qq(q, release);

		allocator::array_helper<value_type, allocator_type>::make_n(
			std::get<1>(tup_height_alloc), qp.first, 1,
			std::forward<Args>(args)...
		);

		qq.release();
		return qp.first;
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred) const;

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred);

	template <typename Pred>
	void for_each_pos(size_type first, size_type last, Pred &&pred);

	size_type find_vacant(size_type first) const;

	allocator_type get_allocator() const
	{
		return std::get<1>(tup_height_alloc);
	}

	std::ostream &dump(std::ostream &os) const;

	void shrink_to_fit();

	std::pair<size_type, size_type> utilization() const;

private:
	struct loc_pair;

	struct node_base {
		virtual ~node_base()
		{}

		virtual void destroy(allocator_type const &a) = 0;
		virtual void release_at(size_type pos) = 0;
		virtual size_type find_vacant(size_type first) const = 0;
		virtual bool erase(allocator_type const &a, size_type pos) = 0;
		virtual node_base *grow_node(
			allocator_type const &a, loc_pair parent, bool maximize
		) = 0;
		virtual node_base *shrink_node(
			allocator_type const &a, loc_pair parent
		) = 0;
		virtual std::pair<size_type, size_type> utilization() const = 0;
	};

	struct ptr_node_base : node_base {
		typedef node_base *node_value_type;
		struct value_valid_pred {
			static bool test(node_base *v)
			{
				return v != nullptr;
			}
		};

		static ptr_node_base *make(
			allocator_type const &a, bool maximize
		);

		virtual node_value_type *ptr_at(size_type pos) = 0;
		virtual node_value_type const *ptr_at(size_type pos) const = 0;
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

	struct data_node_base : node_base {
		typedef value_type node_value_type;
		typedef typename detail::sparse_vector_value_predicate<
			Policy, detail::has_value_valid_pred<Policy>::value
		>::type value_valid_pred;

		static data_node_base *make(
			allocator_type const &a, bool maximize
		);

		virtual node_value_type *ptr_at(size_type pos) = 0;
		virtual node_value_type const *ptr_at(size_type pos) const = 0;
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
		loc_pair(node_base *ptr_, size_type pos_)
		: ptr(static_cast<ptr_node_base *>(ptr_)), pos(pos_)
		{}

		loc_pair() = default;

		std::pair<node_base **, bool> reserve()
		{
			return ptr->reserve_at(pos);
		}

		void release()
		{
			ptr->release_at(pos);
		}

		data_node_base *data_ref()
		{
			return static_cast<data_node_base *>(
				*ptr->ptr_at(pos)
			);
		}

		ptr_node_base *ptr;
		size_type pos;
	};

	struct c_loc_pair {
		ptr_node_base const *ptr;
		size_type pos;
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

		virtual typename base_type::node_value_type const *ptr_at(
			size_type pos
		) const
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
				rv.first = items.unsafe_ptr_at(rv.second);

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
				rv.first = items.unsafe_ptr_at(rv.second);

			return rv;
		}

		virtual size_type find_vacant(size_type first) const
		{
			return items.find_vacant(first);
		}

		virtual bool erase(allocator_type const &a, size_type pos)
		{
			return items.erase_at(a, pos);
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
			allocator_type const &a, loc_pair parent, bool maximize
		)
		{
			if (std::is_same<next_node_type, self_type>::value)
				return *parent.ptr->ptr_at(parent.pos);

			if (maximize)
				return resize_node<node<
					base_type, MaxOrdId - 1, MaxOrdId, arr
				>>(a, parent, this);
			else
				return resize_node<next_node_type>(
					a, parent, this
				);
		}

		template <typename OtherNodeType>
		static node_base *resize_node(
			allocator_type const &a, loc_pair parent, node *self
		)
		{
			typedef allocator::array_helper<
				OtherNodeType, allocator_type
			> a_h;

			auto deleter = [a](OtherNodeType *p) -> void {
				p->destroy(a);
			};

			std::unique_ptr<
				OtherNodeType, decltype(deleter)
			> p(a_h::alloc_n(a, 1, a), deleter);

			p->items.init_move(a, self->items);
			*(parent.ptr->ptr_at(parent.pos)) = p.get();
			auto rv(p.release());
			self->destroy(a);
			return rv;
		}

		virtual node_base *shrink_node(
			allocator_type const &a, loc_pair parent
		)
		{
			auto sz(items.count());
			if (!sz) {
				parent.ptr->erase(a, parent.pos);
				destroy(a);
				return nullptr;
			}

			if (std::is_same<prev_node_type, self_type>::value)
				return this;

			if (sz > prev_node_type::storage_size())
				return this;

			return shrink_node_impl<prev_node_type>(
				a, parent, this, sz
			);
		}

		template <typename OtherNodeType>
		static node_base *shrink_node_impl(
			allocator_type const &a, loc_pair parent,
			node *self, size_type sz
		)
		{
			typedef typename OtherNodeType::prev_node_type
			pp_node_type;

			if (
				!std::is_same<
					OtherNodeType, pp_node_type
				>::value
				&& (sz <= pp_node_type::storage_size())
			)
				return shrink_node_impl<
					pp_node_type
				>(a, parent, self, sz);

			return resize_node<OtherNodeType>(a, parent, self);
		}

		virtual std::pair<size_type, size_type> utilization() const
		{
			return std::make_pair(
				sizeof(*this),
				items.count() * sizeof(value_type)
			);
		}

		detail::compressed_array<
			typename base_type::node_value_type, apparent_order,
			real_order, typename base_type::value_valid_pred
		> items;

		constexpr static size_type storage_size()
		{
			return decltype(items)::storage_size();
		}
	};

	typedef node<
		ptr_node_base, 0, Policy::ptr_node_order.size(),
		Policy::ptr_node_order
	> min_ptr_node_type;

	typedef node<
		ptr_node_base, Policy::ptr_node_order.size() - 1,
		Policy::ptr_node_order.size(), Policy::ptr_node_order
	> max_ptr_node_type;

	typedef node<
		data_node_base, 0, Policy::data_node_order.size(),
		Policy::data_node_order
	> min_data_node_type;

	typedef node<
		data_node_base, Policy::data_node_order.size() - 1,
		Policy::data_node_order.size(), Policy::data_node_order
	> max_data_node_type;

	typedef typename detail::static_bit_field_map<
		sizeof(size_type) * 8, max_data_node_type::apparent_order,
		max_ptr_node_type::apparent_order
	>::repeat_last::value_type pos_field_map;

	constexpr static size_type node_size_shift(size_type h)
	{
		return pos_field_map::value[h - 1].first;
	}

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
		return (pos >= max_data_node_type::apparent_size) ? (
			(yesod::fls(pos) - max_data_node_type::apparent_order)
			/ max_ptr_node_type::apparent_order + 2
		) : 1;
	}

	loc_pair alloc_data_node_at(size_type pos);

	constexpr static std::array<std::size_t, 1> root_node_order = {{0}};

	node<
		ptr_node_base, 0, 1, root_node_order
	> root;
	std::tuple<size_type, allocator_type> tup_height_alloc;
};

}}
#endif

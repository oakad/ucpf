/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_POD_SPARSE_VECTOR_BASE_20141031T1700)
#define UCPF_YESOD_POD_SPARSE_VECTOR_BASE_20141031T1700

#include <memory>
#include <yesod/bitops.hpp>
#include <yesod/detail/static_bit_field_map.hpp>

namespace ucpf { namespace yesod {
namespace detail {

template <typename T>
struct has_value_init_func {
	template <typename U>
	struct type_wrapper {
		typedef U type;
	};

	template <typename U>
	static std::true_type test(
		type_wrapper<U> const volatile *,
		type_wrapper<typename U::value_init_func> * = 0
	);

	static std::false_type test(...);

	typedef decltype(
		test(static_cast<type_wrapper<T> *>(nullptr))
	) type;

	static const bool value = type::value;
};

template <typename Policy, typename DefaultInit, bool HasPredicate = false>
struct pod_sparse_vector_value_init {
	typedef DefaultInit type;
};

template <typename Policy, typename DefaultInit>
struct pod_sparse_vector_value_init<Policy, DefaultInit, true> {
	typedef typename Policy::value_init_func type;
};

}

template <typename...>
struct pod_sparse_vector;

template <typename ValueType, typename Policy>
struct pod_sparse_vector<ValueType, Policy> {
	typedef ValueType value_type;
	static_assert(
		std::is_pod<value_type>::value,
		"std::is_pod<value_type>::value"
	);

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

	pod_sparse_vector()
	: root(nullptr), data(nullptr, 0),
	  tup_height_alloc_init(0, allocator_type(), init_type())
	{}

	template <typename Alloc>
	pod_sparse_vector(Alloc const &a)
	: root(nullptr), data(nullptr, 0),
	  tup_height_alloc_init(0, a, init_type())
	{}

	~pod_sparse_vector()
	{
		clear();
	}

	void clear();

	pointer ptr_at(uintptr_t pos)
	{
		return const_cast<pointer>(
			const_cast<
				pod_sparse_vector const *
			>(this)->ptr_at(pos)
		);
	}

	const_pointer ptr_at(uintptr_t pos) const;

	template <typename Pred>
	bool for_each(uintptr_t first, Pred &&pred) const
	{
		return for_each_impl<const_reference>(
			first, std::forward<Pred>(pred)
		);
	}

	template <typename Pred>
	bool for_each(uintptr_t first, Pred &&pred)
	{
		return for_each_impl<reference>(
			first, std::forward<Pred>(pred)
		);
	}

	template <typename Pred>
	void for_each_pos(uintptr_t first, uintptr_t last, Pred &&pred);

	allocator_type get_allocator() const
	{
		return std::get<1>(tup_height_alloc_init);
	}

private:
	struct node_base {};

	struct ptr_node : node_base {
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<ptr_node> node_allocator_type;

		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<ptr_node> node_allocator_traits;

		constexpr static size_type size()
		{
			return size_type(1) << Policy::ptr_node_order;
		}

		static ptr_node *construct(allocator_type const &a_)
		{
			node_allocator_type a(a_);

			auto rv(node_allocator_traits::allocate(a, 1));
			node_allocator_traits::construct(a, rv);
			return rv;
		}

		static void destroy(allocator_type const &a_, node_base *p)
		{
			if (!p)
				return;

			node_allocator_type a(a_);
			auto pp(static_cast<ptr_node *>(p));

			node_allocator_traits::destroy(a, pp);
			node_allocator_traits::deallocate(a, pp, 1);
		}

		node_base *at(size_type pos)
		{
			return items[pos];
		}

		node_base const *at(size_type pos) const
		{
			return items[pos];
		}

		void set(size_type pos, node_base *p)
		{
			items[pos] = p;
		}

		node_base *items[size()];
	};

	struct default_init {
		void operator()(allocator_type &a, value_type *v)
		{
			allocator_traits::construct(a, v);
		}
	};

	typedef typename detail::pod_sparse_vector_value_init<
		Policy, default_init,
		detail::has_value_init_func<Policy>::value
	>::type init_type;

	struct data_node : node_base {
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<data_node> node_allocator_type;

		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<data_node> node_allocator_traits;

		constexpr static size_type size()
		{
			return size_type(1) << Policy::data_node_order;
		}

		static data_node *construct(
			allocator_type &a_, init_type &init_func
		)
		{
			node_allocator_type a(a_);

			auto rv(node_allocator_traits::allocate(a, 1));
			node_allocator_traits::construct(a, rv);
			
			for (size_type c(0); c < size(); ++c)
				init_func(a_, rv->ptr_at(c));

			return rv;
		}

		static void destroy(allocator_type const &a_, node_base *p)
		{
			if (!p)
				return;

			node_allocator_type a(a_);
			auto dp(static_cast<data_node *>(p));

			node_allocator_traits::destroy(a, dp);
			node_allocator_traits::deallocate(a, dp, 1);
		}

		pointer ptr_at(size_type pos)
		{
			return &items[pos];
		}

		const_pointer ptr_at(size_type pos) const
		{
			return &items[pos];
		}

		value_type items[size()];
	};

	typedef std::pair<ptr_node *, size_type> loc_pair;
	typedef std::pair<ptr_node const *, size_type> c_loc_pair;

	typedef typename detail::static_bit_field_map<
		sizeof(uintptr_t) * 8, Policy::data_node_order,
		Policy::ptr_node_order
	>::repeat_last::value_type pos_field_map;

	constexpr static size_type node_size_shift(size_type h)
	{
		return pos_field_map::value[h].first;
	}

	constexpr static size_type node_size(size_type h)
	{
		return size_type(1) << pos_field_map::value[h].first;
	}

	constexpr static size_type node_offset(uintptr_t pos, size_type h)
	{
		return (pos >> pos_field_map::value[h].second) & (
			node_size(h) - 1
		);
	}

	constexpr static size_type height_at_pos(uintptr_t pos)
	{
		return (pos >= data_node::size()) ? (
			(yesod::fls(pos) - Policy::data_node_order)
			/ Policy::ptr_node_order + 1
		) : 0;
	}

	template <typename ValueRefType, typename Pred>
	bool for_each_impl(uintptr_t first, Pred &&pred) const;

	data_node *alloc_data_node_at(uintptr_t pos);

	ptr_node *root;
	mutable std::pair<data_node *, uintptr_t> data;
	std::tuple<size_type, allocator_type, init_type> tup_height_alloc_init;
};

template <typename ValueType, typename Policy>
void pod_sparse_vector<ValueType, Policy>::clear(void)
{
	auto &a(std::get<1>(tup_height_alloc_init));
	if (!root) {
		data_node::destroy(a, data.first);
		data = std::make_pair(nullptr, 0);
		return;
	}

	data = std::make_pair(nullptr, 0);
	auto const height(std::get<0>(tup_height_alloc_init));
	size_type h(0);

	loc_pair tree_loc[height];
	tree_loc[0] = std::make_pair(root, 0);
	while (true) {
		if (tree_loc[h].second >= ptr_node::size()) {
			ptr_node::destroy(a, tree_loc[h].first);
			if (!h)
				return;

			++tree_loc[--h].second;
			continue;
		}

		if ((h + 1) == height) {
			while (tree_loc[h].second < ptr_node::size()) {
				auto d_ptr(static_cast<data_node *>(
					tree_loc[h].first->at(
						tree_loc[h].second
					)
				));
				data_node::destroy(a, d_ptr);
				++tree_loc[h].second;
			}
			continue;
		}

		auto p(static_cast<ptr_node *>(
			tree_loc[h].first->at(tree_loc[h].second)
		));
		if (p) {
			tree_loc[++h] = std::make_pair(p, 0);
		} else
			++tree_loc[h].second;
	}
}

template <typename ValueType, typename Policy>
auto pod_sparse_vector<ValueType, Policy>::ptr_at(uintptr_t pos) const
-> const_pointer
{
	if (data.first && (data.second == (pos >> node_size_shift(0))))
		return data.first->ptr_at(node_offset(pos, 0));

	auto h(std::get<0>(tup_height_alloc_init));
	node_base *p(root);

	while (p && h) {
		p = static_cast<ptr_node *>(p)->at(node_offset(pos, h));
		--h;
	}

	if (p) {
		data = std::make_pair(
			static_cast<data_node *>(p), pos >> node_size_shift(0)
		);
		return data.first->ptr_at(node_offset(pos, 0));
	} else
		return nullptr;
}

template <typename ValueType, typename Policy>
template <typename Pred>
void pod_sparse_vector<ValueType, Policy>::for_each_pos(
	uintptr_t first, uintptr_t last, Pred &&pred
)
{
	auto last_tree_pos((last - 1) >> node_size_shift(0));
	bool same_node(last_tree_pos == (first >> node_size_shift(0)));
	alloc_data_node_at(last_tree_pos << node_size_shift(0));
	auto d_pos(node_offset(first, 0));
	auto &a(std::get<1>(tup_height_alloc_init));
	auto &init(std::get<2>(tup_height_alloc_init));

	if (same_node) {
		while (first < last) {
			pred(first, *data.first->ptr_at(d_pos++));
			++first;
		}
		return;
	}

	auto const height(std::get<0>(tup_height_alloc_init));
	size_type h;

	loc_pair tree_loc[height];
	tree_loc[0] = std::make_pair(root, node_offset(first, height - h));
	for (h = 1; h < height; ++h)
		tree_loc[h] = std::make_pair(
			nullptr, node_offset(first, height - h)
		);
	h = 0;

	while (true) {
		if (tree_loc[h].second >= ptr_node::size()) {
			if (!h)
				return;

			tree_loc[h].second = 0;
			++tree_loc[--h].second;
			continue;
		}

		if ((h + 1) == height) {
			auto q(static_cast<data_node *>(
				tree_loc[h].first->at(tree_loc[h].second)
			));
			if (!q) {
				q = data_node::construct(a, init);
				tree_loc[h].first->set(tree_loc[h].second, q);
			}

			while (d_pos < data_node::size()) {
				pred(first, *q->ptr_at(d_pos++));
				++first;
				if (first == last)
					return;
			}
			d_pos = 0;
			++tree_loc[h].second;
			continue;
		}

		auto p(static_cast<ptr_node *>(
			tree_loc[h].first->at(tree_loc[h].second)
		));
		if (!p) {
			p = ptr_node::construct(a);
			tree_loc[h].first->set(tree_loc[h].second, p);
		}

		tree_loc[++h].first = p;
	}
}

template <typename ValueType, typename Policy>
template <typename ValueRefType, typename Pred>
bool pod_sparse_vector<ValueType, Policy>::for_each_impl(
	uintptr_t first, Pred &&pred
) const
{
	auto d_pos(node_offset(first, 0));
	if (!root) {
		if (data.first && (
			data.second == (first >> node_size_shift(0))
		)) {
			while (d_pos < data_node::size()) {
				if (pred(first, *data.first->ptr_at(d_pos++)))
					return true;
				++first;
			}
		}
		return false;
	}

	auto const height(std::get<0>(tup_height_alloc_init));
	size_type h;
	uintptr_t n_pos(0);

	loc_pair tree_loc[height];
	tree_loc[0] = std::make_pair(root, node_offset(first, height - h));
	for (h = 1; h < height; ++h)
		tree_loc[h] = std::make_pair(
			nullptr, node_offset(first, height - h)
		);
	h = 0;

	while (true) {
		if (tree_loc[h].second >= ptr_node::size()) {
			if (!h)
				return false;

			tree_loc[h].second = 0;
			n_pos >>= node_size_shift(height - h);
			++tree_loc[--h].second;
			auto mask(node_size(height - h) - 1);
			n_pos &= ~mask;
			n_pos |= tree_loc[h].second & mask;
			continue;
		}

		if ((h + 1) == height) {
			auto q(static_cast<data_node *>(
				tree_loc[h].first->at(tree_loc[h].second)
			));
			auto base_pos(n_pos << node_size_shift(0));
			while (d_pos < data_node::size()) {
				if (pred(
					base_pos + d_pos, *q->ptr_at(d_pos++)
				))
					return true;
			}
			d_pos = 0;
			++tree_loc[h].second;
			auto mask(node_size(height - h) - 1);
			n_pos &= ~mask;
			n_pos |= tree_loc[h].second & mask;
			continue;
		}

		auto p(static_cast<ptr_node *>(
			tree_loc[h].first->at(tree_loc[h].second)
		));
		if (p) {
			tree_loc[++h].first = p;
			n_pos <<= node_size_shift(height - h);
		} else {
			++tree_loc[h].second;
			auto mask(node_size(height - h) - 1);
			n_pos &= ~mask;
			n_pos |= tree_loc[h].second & mask;
		}
	}
}

template <typename ValueType, typename Policy>
auto pod_sparse_vector<ValueType, Policy>::alloc_data_node_at(
	uintptr_t pos
) -> data_node *
{
	auto &init(std::get<2>(tup_height_alloc_init));
	auto &a(std::get<1>(tup_height_alloc_init));
	auto &h(std::get<0>(tup_height_alloc_init));
	auto p_h(height_at_pos(pos));

	if (!p_h && !root) {
		if (!data.first)
			data = std::make_pair(
				data_node::construct(a, init), 0
			);

		return data.first;
	}

	if (!root) {
		if (!data.first) {
			data = std::make_pair(
				data_node::construct(a, init), 0
			);

			root = ptr_node::construct(a);
			root->set(node_offset(pos, 1), data.first);
			auto d_ptr(data.first);
			data.first = nullptr;
			h = 1;

			while (h < p_h) {
				auto p_ptr(ptr_node::construct(a));
				p_ptr->set(node_offset(pos, ++h), root);
				root = p_ptr;
			}
			data = std::make_pair(
				d_ptr, pos >> node_size_shift(0)
			);

			return d_ptr;
		} else {
			root = ptr_node::construct(a);
			root->set(0, data.first);
			h = 1;
		}
	}

	while (h < p_h) {
		auto p_ptr(ptr_node::construct(a));
		p_ptr->set(0, root);
		root = p_ptr;
		++h;
	}

	p_h = 0;
	auto p(root);
	while (true) {
		if ((p_h + 1) == h) {
			auto q(static_cast<data_node *>(
				p->at(node_offset(pos, h - p_h))
			));
			if (!q) {
				q = data_node::construct(a, init);
				p->set(node_offset(pos, h - p_h), q);
			}
				data = std::make_pair(
					q, pos >> node_size_shift(0)
				);
				return q;
		} else {
			auto q(static_cast<ptr_node *>(
				p->at(node_offset(pos, h - p_h))
			));
			if (!q) {
				q = ptr_node::construct(a);
				p->set(node_offset(pos, h - p_h), q);
			}
			p = q;
		}
	}
}

}}
#endif

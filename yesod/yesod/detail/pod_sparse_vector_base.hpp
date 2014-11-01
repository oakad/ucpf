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
#include <ostream>
#include <yesod/bitops.hpp>
#include <yesod/detail/static_bit_field_map.hpp>
#include <yesod/detail/tree_print_decorator.hpp>

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

	std::ostream &dump(std::ostream &os) const;

private:
	struct node_base {};
	struct ptr_node;

	struct loc_pair {
		node_base *ptr;
		size_type pos;
	};

	struct c_loc_pair {
		node_base const *ptr;
		size_type pos;
	};

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

		static void destroy(allocator_type const &a_, ptr_node *p)
		{
			node_allocator_type a(a_);

			node_allocator_traits::destroy(a, p);
			node_allocator_traits::deallocate(a, p, 1);
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

		c_loc_pair find_occupied(size_type first) const
		{
			while (first < size()) {
				if (items[first])
					return c_loc_pair{
						items[first], first
					};
				++first;
			}
			return c_loc_pair{nullptr, first};
		}

		loc_pair find_occupied(size_type first)
		{
			while (first < size()) {
				if (items[first])
					return loc_pair{
						items[first], first
					};
				++first;
			}
			return loc_pair{nullptr, first};
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
	tree_loc[0] = loc_pair{root, 0};
	while (true) {
		auto p(static_cast<ptr_node *>(tree_loc[h].ptr));
		auto pp(p->find_occupied(tree_loc[h].pos));

		if (!pp.ptr) {
			ptr_node::destroy(a, p);
			if (!h)
				break;

			++tree_loc[--h].pos;
			continue;
		}

		tree_loc[h].pos = pp.pos;

		if ((h + 1) == height) {
			data_node::destroy(
				a, static_cast<data_node *>(pp.ptr)
			);
			++tree_loc[h].pos;
		} else {
			++tree_loc[h].pos;
			tree_loc[++h] = loc_pair{pp.ptr, 0};
		}
	}

	std::get<0>(tup_height_alloc_init) = 0;
	root = nullptr;
	data = std::make_pair(nullptr, 0);
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
	tree_loc[0] = loc_pair{root, node_offset(first, height)};
	for (h = 1; h < height; ++h)
		tree_loc[h] = loc_pair{
			nullptr, node_offset(first, height - h)
		};
	h = 0;

	while (true) {
		auto p(static_cast<ptr_node *>(tree_loc[h].ptr));
		auto pos(tree_loc[h].pos);

		if (pos >= ptr_node::size()) {
			if (!h)
				return;

			tree_loc[h].pos = 0;
			++tree_loc[--h].pos;
			continue;
		}

		if ((h + 1) == height) {
			auto d_ptr(static_cast<data_node *>(p->at(pos)));
			if (!d_ptr) {
				d_ptr = data_node::construct(a, init);
				p->set(pos, d_ptr);
			}

			while (d_pos < data_node::size()) {
				pred(first, *d_ptr->ptr_at(d_pos++));
				++first;
				if (first == last)
					return;
			}
			d_pos = 0;
			++tree_loc[h].pos;
			continue;
		}

		auto q(static_cast<ptr_node *>(p->at(pos)));
		if (!q) {
			q = ptr_node::construct(a);
			p->set(pos, q);
		}

		tree_loc[++h].ptr = q;
	}
}

template <typename ValueType, typename Policy>
std::ostream &pod_sparse_vector<ValueType, Policy>::dump(
	std::ostream &os
) const
{
	struct printer_type {
		printer_type(std::ostream &os_)
		: os(os_)
		{}

		void operator()(char const *str)
		{
			os << str;
		}

		std::ostream &os;
	} printer(os);

	detail::tree_print_decorator<
		printer_type, allocator_type
	> decorator(printer, std::get<1>(tup_height_alloc_init));

	auto print_data = [&os, &decorator](data_node const *p) -> void {
		for (size_type c(0); c < (data_node::size() - 1); ++c) {
			decorator.next_child();
			os << '[' << c << "] ";
			os << *p->ptr_at(c) << '\n';
		}
		decorator.last_child();
		os << '[' << (data_node::size() - 1) << "] ";
		os << *p->ptr_at(data_node::size() - 1) << '\n';
	};

	os << '<' << this << ">\n";
	auto const height(std::get<0>(tup_height_alloc_init));
	if (!height) {
		if (data.first) {
			decorator.push_level();
			os << "[^" << data.second << "] <" << data.first
			   << ">\n";
			print_data(data.first);
		}
		return os;
	}

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{root, 0};
	size_type h(0);
	decorator.last_child();
	os << '[' << 0 << "] <" << root << ">\n";
	decorator.push_level();

	while (true) {
		auto p(static_cast<ptr_node const *>(tree_loc[h].ptr));
		auto pp(p->find_occupied(tree_loc[h].pos));
		if (!pp.ptr) {
			if (!h)
				return os;

			--h;
			decorator.pop_level();
			continue;
		}

		auto pq(p->find_occupied(pp.pos + 1));
		tree_loc[h].pos = pq.pos;
		if (pq.ptr)
			decorator.next_child();
		else
			decorator.last_child();

		os << '[' << pp.pos << "] <" << pp.ptr << ">\n";

		if ((h + 1) == height) {
			decorator.push_level();
			print_data(static_cast<data_node const*>(pp.ptr));
			decorator.pop_level();
		} else {
			decorator.push_level();
			tree_loc[++h] = c_loc_pair{pp.ptr, 0};
		}
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

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{root, node_offset(first, height)};
	for (h = 1; h < height; ++h)
		tree_loc[h] = c_loc_pair{
			nullptr, node_offset(first, height - h)
		};
	h = 0;

	while (true) {
		auto p(static_cast<ptr_node const *>(tree_loc[h].ptr));
		auto pp(p->find_occupied(tree_loc[h].pos));

		if (!pp.ptr) {
			if (!h)
				return false;

			tree_loc[h].pos = 0;
			n_pos >>= node_size_shift(height - h);
			++tree_loc[--h].pos;
			continue;
		}

		tree_loc[h].pos = pp.pos;
		auto mask(node_size(height - h) - 1);
		n_pos &= ~mask;
		n_pos |= tree_loc[h].pos;

		if ((h + 1) == height) {
			auto d_ptr(static_cast<data_node const *>(pp.ptr));
			auto base_pos(n_pos << node_size_shift(0));
			while (d_pos < data_node::size()) {
				if (pred(
					base_pos + d_pos,
					const_cast<ValueRefType>(
						*d_ptr->ptr_at(d_pos)
					)
				))
					return true;

				++d_pos;
			}
			d_pos = 0;
			++tree_loc[h].pos;
			continue;
		} else {
			tree_loc[++h].ptr = pp.ptr;
			n_pos <<= node_size_shift(height - h);
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
			++p_h;
		}
	}
}

}}
#endif

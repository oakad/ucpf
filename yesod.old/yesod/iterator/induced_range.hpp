/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_7020A0422B6980AA796A76325F1035E0)
#define HPP_7020A0422B6980AA796A76325F1035E0

#include <yesod/iterator/range.hpp>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

struct induced_range_base {
	struct node {
		static void get_next(node *&node_ptr, std::size_t &offset)
		{
			++offset;
			if (offset < node_ptr->data_size)
				return;

			auto n_prev = node_ptr;

			if (node_ptr->next) {
				offset = 0;
				node_ptr = node_ptr->next;
				n_prev->parent->release(n_prev);
			} else {
				node_ptr = nullptr;
				node_ptr = n_prev->parent->fetch_next(
					n_prev, offset, true
				);
			}
		}

		node *prev = nullptr;
		node *next = nullptr;
		induced_range_base *parent = nullptr;
		long ref_count = 0;
		std::size_t alloc_size = 0;
		std::size_t data_size = 0;
		void *data = nullptr;
	};

	virtual void release(node *) = 0;
	virtual node *fetch_next(
		node *prev_ptr, std::size_t &offset, bool release_prev
	) = 0;

	long ref_count = 0;
};

template <typename T>
struct has_acquire {
	template <typename U, U>
	struct wrapper {};

	template <typename U>
	static std::true_type test(
		U *, wrapper<
			std::pair<void *, std::size_t> (U::*)(), &U::acquire
		> * = nullptr
	);
	static std::false_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;

	static constexpr bool value = type::value;
};

template <typename ValueType, typename Producer, typename Alloc>
struct induced_range_store : induced_range_base, Alloc {
	typedef ValueType value_type;
	typedef induced_range_base::node node;
	typedef allocator::array_helper<node, Alloc> node_alloc_helper_t;
	typedef allocator::array_helper<
		induced_range_store, Alloc
	> self_alloc_helper_t;

	static induced_range_base::node *make_head(
		Producer &p_, Alloc const &a
	)
	{
		auto deleter = [a](induced_range_store *store_ptr) {
			self_alloc_helper_t::destroy(a, store_ptr, 1, true);
		};

		auto node_deleter = [](node *node_ptr) {
			node_ptr->parent->release(node_ptr);
		};

		std::unique_ptr<
			induced_range_store, decltype(deleter)
		> s(self_alloc_helper_t::alloc(a, p_, a), deleter);

		std::unique_ptr<
			node, decltype(node_deleter)
		> node_ptr(s->alloc_node(), node_deleter);

		auto s_ptr = s.release();

		if (s_ptr->p.attach_data(*s_ptr, node_ptr.get()))
			return node_ptr.release();
		else
			return nullptr;
	}

	induced_range_store(Producer &p_, Alloc const &a)
	: Alloc(a), p(p_), finished(false)
	{}

	virtual void release(node *node_ptr)
	{
		if (node_ptr->ref_count)
			--node_ptr->ref_count;

		if (node_ptr->ref_count)
			return;

		if (node_ptr->prev)
			return;

		do {
			auto p_next(node_ptr->next);
			if (p_next)
				p_next->prev = nullptr;

			p.detach_data(*this, node_ptr);

			node_alloc_helper_t::destroy(
				static_cast<Alloc>(*this), node_ptr,
				1, true
			);

			--ref_count;

			if (!ref_count) {
				self_alloc_helper_t::destroy(
					static_cast<Alloc &>(*this), this, 1,
					true
				);
				return;
			}

			node_ptr = p_next;
		} while (node_ptr && !node_ptr->ref_count);
	}

	virtual node *fetch_next(
		node *prev_ptr, std::size_t &offset, bool release_prev
	)
	{
		auto prev_deleter = [release_prev](node *p) {
			if (release_prev)
				p->parent->release(p);
		};

		auto node_deleter = [](node *p) {
			p->parent->release(p);
		};

		std::unique_ptr<node, decltype(prev_deleter)> prev_ptr_guard(
			prev_ptr, prev_deleter
		);

		auto l_offset(prev_ptr->data_size);
		offset = 0;

		if (finished)
			return nullptr;

		finished = true;
		if (prev_ptr->data_size < prev_ptr->alloc_size) {
			if (p.append_data(*this, prev_ptr)) {
				offset = l_offset;
				finished = false;
				return prev_ptr;
			} else
				return nullptr;
		}

		std::unique_ptr<node, decltype(node_deleter)> next_ptr(
			alloc_node(), node_deleter
		);

		if (p.attach_data(*this, next_ptr.get())) {
			prev_ptr->next = next_ptr.get();
			next_ptr->prev = prev_ptr;
			finished = false;
			return next_ptr.release();
		} else
			return nullptr;
	}

	node *alloc_node()
	{
		auto node_ptr = node_alloc_helper_t::alloc(
			static_cast<Alloc const &>(*this)
		);
		node_ptr->parent = this;
		++ref_count;
		return node_ptr;
	}

	template <typename Tp, bool HasAcquire = false>
	struct producer_access {
		typedef allocator::array_helper<
			value_type, Alloc
		> data_alloc_helper_t;

		static void * alloc_data_buf(induced_range_store &parent)
		{
			return data_alloc_helper_t::alloc_n(
				static_cast<Alloc const &>(parent),
				Tp::preferred_block_size
			);
		}

		producer_access(Tp &p_)
		: p(p_)
		{}

		std::size_t append_data(
			induced_range_store &parent, node *node_ptr
		)
		{
			auto r_sz = node_ptr->alloc_size - node_ptr->data_size;
			if (!r_sz)
				return r_sz;

			auto c_sz = p.copy(
				reinterpret_cast<value_type *>(
					node_ptr->data
				) + node_ptr->data_size, r_sz
			);
			node_ptr->data_size += c_sz;
			return c_sz;
		}

		std::size_t attach_data(
			induced_range_store &parent, node *node_ptr
		)
		{
			node_ptr->data = alloc_data_buf(parent);
			node_ptr->alloc_size = Tp::preferred_block_size;
			return append_data(parent, node_ptr);
		}

		void detach_data(induced_range_store &parent, node *node_ptr)
		{
			data_alloc_helper_t::destroy(
				static_cast<Alloc const &>(parent),
				node_ptr->data, Tp::preferred_block_size, true
			);

			node_ptr->data = nullptr;
			node_ptr->alloc_size = 0;
			node_ptr->data_size = 0;
		}

		Tp &p;
	};

	template <typename Tp>
	struct producer_access<Tp, true> {
		producer_access(Tp &p_)
		: p(p_)
		{}

		std::size_t append_data(
			induced_range_store &parent, node *node_ptr
		) {
			return 0;
		}

		std::size_t attach_data(
			induced_range_store &parent, node *node_ptr
		)
		{
			auto bp(p.acquire());
			node_ptr->data = bp.first;
			node_ptr->alloc_size = bp.second;
			node_ptr->data_size = bp.second;
			return bp.second;
		}

		void detach_data(induced_range_store &parent, node *node_ptr)
		{
			p.release(node_ptr->data, node_ptr->alloc_size);
			node_ptr->data = nullptr;
			node_ptr->alloc_size = 0;
			node_ptr->data_size = 0;
		}

		Tp &p;
	};

	producer_access<Producer, has_acquire<Producer>::value> p;
	bool finished;
};

}

template <typename ValueType>
struct induced_range_iterator : facade<
	induced_range_iterator<ValueType const>, ValueType const,
	std::forward_iterator_tag
> {
	induced_range_iterator()
	: node_ptr(nullptr), offset(0)
	{}

	induced_range_iterator(induced_range_iterator const &other)
	: node_ptr(other.node_ptr), offset(other.offset)
	{
		if (node_ptr)
			++node_ptr->ref_count;
	}

	induced_range_iterator(induced_range_iterator &&other)
	: node_ptr(other.node_ptr), offset(other.offset)
	{
		other.node_ptr = nullptr;
		other.offset = 0;
	}

	~induced_range_iterator()
	{
		if (node_ptr)
			node_ptr->parent->release(node_ptr);
	}

	induced_range_iterator &operator=(induced_range_iterator const &other)
	{
		offset = other.offset;

		if (node_ptr == other.node_ptr)
			return *this;

		if (node_ptr)
			node_ptr->parent->release(node_ptr);

		node_ptr = other.node_ptr;

		if (node_ptr)
			++node_ptr->ref_count;

		return *this;
	}

	induced_range_iterator &operator=(induced_range_iterator &&other)
	{
		offset = other.offset;
		if (node_ptr)
			node_ptr->parent->release(node_ptr);

		node_ptr = other.node_ptr;
		other.node_ptr = nullptr;
		other.offset = 0;
	}

private:
	friend struct core_access;
	template <typename ValueType1, typename Producer, typename Alloc>
	friend range<induced_range_iterator<ValueType1>> make_induced_range(
		Producer &p, Alloc const &a
	);

	induced_range_iterator(
		detail::induced_range_base::node *node_ptr_,
		std::size_t offset_
	) : node_ptr(node_ptr_), offset(offset_)
	{
		if (node_ptr)
			++node_ptr->ref_count;
	}

	bool equal(induced_range_iterator const &other) const
	{
		if (node_ptr == other.node_ptr)
			return !node_ptr || (offset == other.offset);
		else
			return false;
	}

	void increment()
	{
		detail::induced_range_base::node::get_next(node_ptr, offset);
	}

	typename induced_range_iterator::reference dereference() const
	{
		return *(reinterpret_cast<ValueType const *>(
			node_ptr->data
		) + offset);
	}

	detail::induced_range_base::node *node_ptr;
	std::size_t offset;
};

template <
	typename ValueType, typename Producer,
	typename Alloc = std::allocator<void>
>
range<induced_range_iterator<ValueType>> make_induced_range(
	Producer &p, Alloc const &a = Alloc()
)
{
	return make_range(
		induced_range_iterator<ValueType>(
			detail::induced_range_store<
				ValueType, Producer, Alloc
			>::make_head(p, a), 0
		), induced_range_iterator<ValueType>()
	);
}

}}}

#endif

/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_7020A0422B6980AA796A76325F1035E0)
#define HPP_7020A0422B6980AA796A76325F1035E0

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

struct induced_range_base {
	struct node {
		void release()
		{
			parent->release(this);
		}

		template <typename Reference>
		Reference at(std::size_t offset) const
		{
			return reinterpret_cast<Reference>(data[
				offset * sizeof(ValueType)
			]);
		}

		node *prev;
		node *next;
		induced_range_base *parent;
		long ref_count;
		std::size_t alloc_size;
		std::size_t data_size;
		uint8_t *data;
	};

	virtual void release(node *) = 0;

	long ref_count;
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
	typedef allocator::array_helper<value_type, Alloc> data_alloc_helper_t;
	typedef typename data_alloc_helper_t::storage_type storage_type;
	typedef induced_range_base::node node;
	typedef allocator::array_helper<node, Alloc> node_alloc_helper_t;
	typedef allocator::array_helper<
		induced_range_store, Alloc
	> self_alloc_helper_t;

	induced_range_store(Producer &p_)
	: p(p_)
	{}

	virtual void release(node *node_ptr)
	{

		--node_ptr->ref_count;
		if (node_ptr->ref_count || node_ptr->prev)
			return;

		if (node_ptr->next)
			node_ptr->next->prev = nullptr;

		p.detach_data(*this, node_ptr);

		node_alloc_helper_t::destroy(
			static_cast<Alloc>(*this), node_ptr,
			1, true
		);

		--ref_count;

		if (!ref_count)
			self_alloc_helper_t::destroy(
				static_cast<Alloc>(*this), this, 1, true
			);
	}

	uint8_t *alloc_data_buf(std::size_t size) const
	{

	}

	node *alloc_node() const
	{

	}

	template <typename Tp, bool HasAcquire = false>
	struct producer_access : Tp {
		producer_access(Tp &p_)
		: p(p_)
		{}

		void attach_data(induced_range_store &parent, node *node_ptr)
		{
			if (!node_ptr->data) {
				node_ptr->data = parent.alloc_data_buf(
					Tp::preferred_block_size
					* sizeof(storage_type)
				);
				node_ptr->alloc_size = Tp::preferred_block_size;
			}

			auto r_sz = node_ptr->alloc_size - node_ptr->data_size;
			if (!r_sz)
				return;

			auto c_sz = p.copy(
				node_ptr->data + node_ptr->data_size, r_sz
			);
			node_ptr->data_size += c_sz;
		}

		void detach_data(induced_range_store &parent, node *node_ptr)
		{
			data_alloc_helper_t::destroy(
				static_cast<Alloc &>(parent), node_ptr->data,
				node_ptr->data_size, false
			);

			data_alloc_helper_t::free_s(
				static_cast<Alloc &>(parent), node_ptr->data,
				node_ptr->alloc_size
			);

			node_ptr->data = nullptr;
			node_ptr->alloc_size = 0;
			node_ptr->data_size = 0;
		}

		Tp &p;
	};

	template <typename Tp>
	struct producer_access<Tp, true> : Tp {
		producer_access(Tp &p_)
		: p(p_)
		{}

		void attach_data(induced_range_store &parent, node *node_ptr)
		{
			if (node_ptr->data) {
				auto n(parent.alloc_node());
				n->next = node_ptr->next;
				n->prev = node_ptr;
				if (node_ptr->next)
					node_ptr->next->prev = n;

				node_ptr->next = n;
				attach_data(parent, n);
				return;
			}

			auto bp(p.acquire());
			node_ptr->data = bp.first;
			node_ptr->alloc_size = bp.second;
			node_ptr->data_size = bp.second;
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
};

}

template <typename ValueType>
struct induced_range : facade<
	induced_range<ValueType>, ValueType const, std::forward_iterator_tag
> {
	induced_range()
	: node_ptr(nullptr), offset(0)
	{}

	~induced_range()
	{
		if (node_ptr)
			node_ptr->release();
	}

private:
	bool equal(const_iterator const &other)
	{
		if (node_base == other.node_base)
			return !node_base || (offset == other.offset);
		else
			return false;
	}

	void increment()
	{
		detail::induced_range_base::node::get_next(node_ptr, offset);
	}

	auto dereference() const
	{
		return node_ptr->at<typename induced_range::reference>(offset);
	}

	detail::induced_range_base::node *node_ptr;
	std::size_t offset;
};

}}}

#endif

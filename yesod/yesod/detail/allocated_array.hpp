/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_09DB589027E9B1068EA45AC244A0FDE2)
#define HPP_09DB589027E9B1068EA45AC244A0FDE2

#include <memory>
#include <cstddef>

namespace ucpf::yesod::detail {

template <typename ValueType, typename Allocator>
struct allocated_array_container;

template <typename ValueType>
struct allocated_array final {
	typedef ValueType value_type;
	typedef size_t size_type;
	typedef std::aligned_storage_t<
		sizeof(value_type), alignof(value_type)
	> storage_type;

	template <typename Allocator, typename... Args>
	static allocated_array *create(
		Allocator const &alloc, size_type data_size, Args &&...args
	)
	{
		return allocated_array_container<ValueType, Allocator>::create(
			alloc, data_size, std::forward<Args>(args)...
		);
	}

	void destroy()
	{
		op_table->destroy(this);
	}

	size_type size() const
	{
		return data_size;
	}

	value_type &operator[](size_type pos)
	{
		return reinterpret_cast<value_type &>(data[pos]);
	}

	value_type const &operator[](size_type pos) const
	{
		return reinterpret_cast<value_type const &>(data[pos]);
	}

	allocated_array(allocated_array &) = delete;
	allocated_array(allocated_array &&) = delete;
	allocated_array(allocated_array const &) = delete;
	allocated_array(allocated_array const &&) = delete;

private:
	template <typename ValueType_, typename Allocator>
	friend struct allocated_array_container;

	struct op_table_t {
		void (*destroy)(allocated_array *p);
	};

	allocated_array(size_type data_size_, op_table_t const *op_table_)
	: data_size(data_size_), op_table(op_table_)
	{}

	~allocated_array() = default;

	op_table_t const * const op_table;
	size_type const data_size;
	storage_type data[];
};

template <typename ValueType, typename Allocator>
struct allocated_array_container final {
	typedef allocated_array<ValueType> array_type;
	typedef typename array_type::value_type value_type;
	typedef typename array_type::size_type size_type;

	allocated_array_container(size_type data_size)
	: impl(data_size)
	{}

private:
	friend array_type;

	typedef typename std::allocator_traits<
		Allocator
	>::template rebind_traits<uint8_t> storage_traits;
	typedef typename std::allocator_traits<
		Allocator
	>::template rebind_traits<allocated_array_container> container_traits;
	typedef typename std::allocator_traits<
		Allocator
	>::template rebind_traits<value_type> value_traits;

	template <typename... Args>
	static array_type *create(
		Allocator const &alloc, size_type data_size, Args &&...args
	)
	{
		size_t data_pos(0);
		auto deleter([
			&data_pos, data_size, &alloc
		](allocated_array_container *cont) {
			typename storage_traits::allocator_type
			storage_alloc(alloc);
			typename container_traits::allocator_type
			container_alloc(alloc);

			cont->destroy_data(data_pos, alloc);
			container_traits::destroy(container_alloc, cont);
			storage_traits::deallocate(
				storage_alloc,
				reinterpret_cast<uint8_t *>(cont),
				alloc_size(data_size)
			);
		});

		typename storage_traits::allocator_type storage_alloc(alloc);
		allocated_array_container *cont_(
			reinterpret_cast<
				allocated_array_container *
			>(storage_traits::allocate(
				storage_alloc, alloc_size(data_size)
			))
		);
		typename container_traits::allocator_type container_alloc(
			alloc
		);
		container_traits::construct(container_alloc, cont_, data_size);

		std::unique_ptr<
			allocated_array_container, decltype(deleter)
		> cont(cont_, deleter);

		typename value_traits::allocator_type value_alloc(alloc);
		while (data_pos < data_size) {
			value_traits::construct(
				value_alloc,
				reinterpret_cast<value_type *>(
					cont->impl.st.data + data_pos
				), std::forward<Args>(args)...
			);
			++data_pos;
		}

		cont->impl.emplace_alloc(alloc);
		return &cont.release()->impl.st;
	}

	static void destroy(array_type *p)
	{
		auto cont(to_container(p));
		auto alloc(cont->impl.destroy_alloc());

		cont->destroy_data(cont->impl.st.data_size, alloc);

		typename container_traits::allocator_type
		container_alloc(alloc);
		container_traits::destroy(container_alloc, cont);

		typename storage_traits::allocator_type storage_alloc(alloc);
		storage_traits::deallocate(
			storage_alloc, reinterpret_cast<uint8_t *>(cont),
			alloc_size(p->data_size)
		);
	}

	static allocated_array_container *to_container(array_type *p)
	{
		return reinterpret_cast<allocated_array_container *>(
			reinterpret_cast<uint8_t *>(p)
			- offsetof(allocated_array_container, impl)
			- offsetof(impl_type, st)
		);
	}

	constexpr static size_type alloc_size(size_type data_size)
	{
		return sizeof(
			allocated_array_container
		) + data_size * sizeof(typename array_type::storage_type);
	}

	void destroy_data(size_type count, Allocator const &alloc)
	{
		if (!count)
			return;

		typename value_traits::allocator_type value_alloc(alloc);
		do {
			--count;
			value_traits::destroy(
				value_alloc, reinterpret_cast<value_type *>(
					impl.st.data + count
				)
			);
		} while (count);
	}

	constexpr static typename array_type::op_table_t op_table = {
		.destroy = destroy
	};

	template <typename Allocator_, bool EmptyAlloc = false>
	struct allocated_array_impl {
		allocated_array_impl(size_type data_size)
		: st(data_size, &op_table)
		{}

		void emplace_alloc(Allocator_ const &alloc_)
		{
			typedef typename std::allocator_traits<
				Allocator_
			>::template rebind_traits<Allocator_> alloc_traits;

			typename alloc_traits::allocator_type alloc__(alloc_);
			alloc_traits::construct(
				alloc__,
				reinterpret_cast<Allocator_ *>(&alloc),
				alloc_
			);
		}

		Allocator_ destroy_alloc()
		{
			typedef typename std::allocator_traits<
				Allocator_
			>::template rebind_traits<Allocator_> alloc_traits;

			auto alloc_(
				*reinterpret_cast<Allocator_ *>(&alloc)
			);

			typename alloc_traits::allocator_type alloc__(alloc_);
			alloc_traits::destroy(
				alloc__, reinterpret_cast<Allocator_ *>(&alloc)
			);

			return alloc_;
		}

		std::aligned_storage_t<
			sizeof(Allocator_), alignof(Allocator_)
		> alloc;
		array_type st;
	};

	template <typename Allocator_>
	struct allocated_array_impl<Allocator_, true> {
		allocated_array_impl(size_type data_size)
		: st(data_size, &op_table)
		{}

		void emplace_alloc(Allocator_ const &alloc_)
		{
		}

		Allocator_ destroy_alloc()
		{
			return Allocator_();
		}

		array_type st;
	};

	typedef allocated_array_impl<
		Allocator, std::is_empty<Allocator>::value
	> impl_type;

	impl_type impl;
};

template <typename ValueType, typename Allocator>
constexpr typename allocated_array_container<
	ValueType, Allocator
>::array_type::op_table_t allocated_array_container<
	ValueType, Allocator
>::op_table;

}
#endif

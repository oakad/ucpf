/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4ECA0B716AA176D226DC177D0177AC64)
#define HPP_4ECA0B716AA176D226DC177D0177AC64

#include <memory>

namespace ucpf::yesod::detail {

template <typename ValueType, typename Allocator>
struct allocated_object {
	typedef ValueType value_type;

	template <typename... Args>
	static allocated_object *create(Allocator const &alloc, Args &&...args)
	{
		bool value_constructed(false);
		auto deleter([&value_constructed, &alloc](
			allocated_object *s
		) {
			typename storage_traits::allocator_type storage_alloc(
				alloc
			);
			if (value_constructed)
				s->destroy_value(alloc);

			storage_traits::deallocate(storage_alloc, s, 1);
		});

		typename storage_traits::allocator_type storage_alloc(alloc);
		std::unique_ptr<allocated_object, decltype(deleter)> s(
			storage_traits::allocate(storage_alloc, 1), deleter
		);

		typename value_traits::allocator_type value_alloc(alloc);
		value_traits::construct(
			value_alloc,
			reinterpret_cast<value_type *>(&s->impl.value),
			std::forward<Args>(args)...
		);
		value_constructed = true;
		s->impl.emplace_alloc(alloc);
		return s.release();
	}

	void destroy()
	{
		typedef typename std::allocator_traits<
			Allocator
		>::template rebind_traits<allocated_object> storage_traits;

		auto alloc(impl.destroy_allocator());

		destroy_value(alloc);

		typename storage_traits::allocator_type storage_alloc(alloc);
		storage_traits::deallocate(storage_alloc, this, 1);
	}

	value_type *get()
	{
		return reinterpret_cast<value_type *>(&impl.value);
	}

	Allocator get_allocator() const
	{
		return impl.get_allocator();
	}

	static allocated_object *to_storage_ptr(value_type *ptr)
	{
		return reinterpret_cast<allocated_object *>(
			reinterpret_cast<uint8_t *>(ptr)
			- offsetof(allocated_object, impl)
			- offsetof(decltype(impl), value)
		);
	}

	static allocated_object const *to_storage_ptr(value_type const *ptr)
	{
		return reinterpret_cast<allocated_object const *>(
			reinterpret_cast<uint8_t const *>(ptr)
			- offsetof(allocated_object, impl)
			- offsetof(decltype(impl), value)
		);
	}

private:
	typedef typename std::allocator_traits<
		Allocator
	>::template rebind_traits<allocated_object> storage_traits;
	typedef typename std::allocator_traits<
		Allocator
	>::template rebind_traits<value_type> value_traits;

	void destroy_value(Allocator const &alloc)
	{
		typename value_traits::allocator_type value_alloc(alloc);
		value_traits::destroy(
			value_alloc,
			reinterpret_cast<value_type *>(&impl.value)
		);
	}

	template <typename Allocator_, bool EmptyAlloc = false>
	struct allocated_object_impl {
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

		Allocator_ get_allocator() const
		{
			return *reinterpret_cast<Allocator_ *>(&alloc);
		}

		Allocator_ destroy_allocator()
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
		std::aligned_storage_t<
			sizeof(value_type), alignof(value_type)
		> value;
	};

	template <typename Allocator_>
	struct allocated_object_impl<Allocator_, true> {
		void emplace_alloc(Allocator_ const &alloc_)
		{
		}

		Allocator_ get_allocator() const
		{
			return Allocator_();
		}

		Allocator_ destroy_allocator()
		{
			return Allocator_();
		}

		std::aligned_storage_t<
			sizeof(value_type), alignof(value_type)
		> value;
	};

	allocated_object_impl<
		Allocator, std::is_empty<Allocator>::value
	> impl;
};
}
#endif

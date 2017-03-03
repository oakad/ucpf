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
struct allocated_storage final {
	typedef ValueType value_type;

	template <typename... Args>
	static allocated_storage *make(Allocator const &alloc, Args &&...args)
	{
		typedef typename std::allocator_traits<
			Allocator
		>::template rebind_traits<allocated_storage> storage_traits;

		int progress(0);
		auto deleter([&progress, &alloc](allocated_storage *s) {
			typename storage_traits::allocator_type alloc_(alloc);
			switch (progress) {
			case 2:
				s->destroy_value(alloc);
			case 1:
				storage_traits::deallocate(alloc_ , s, 1);
			}
		});

		typename storage_traits::allocator_type alloc_(alloc);
		std::unique_ptr<allocated_storage, decltype(deleter)> s(
			storage_traits::allocate(alloc_, 1), deleter
		);
		progress = 1;
		s->emplace_value(alloc, std::forward<Args>(args)...);
		progress = 2;
		s->impl.emplace_alloc(alloc);
		return s.release();
	}

	void destroy()
	{
		typedef typename std::allocator_traits<
			Allocator
		>::template rebind_traits<allocated_storage> storage_traits;

		auto alloc(impl.destroy_alloc());

		destroy_value(alloc);

		typename storage_traits::allocator_type alloc_(alloc);
		storage_traits::deallocate(alloc_, this, 1);
	}

	value_type *get()
	{
		return reinterpret_cast<value_type *>(&impl.value);
	}

	static allocated_storage *to_storage_ptr(value_type *ptr)
	{
		return reinterpret_cast<allocated_storage *>(
			reinterpret_cast<uint8_t *>(ptr)
			- offsetof(allocated_storage, impl)
			- offsetof(decltype(impl), value)
		);
	}

private:
	template <typename... Args>
	void emplace_value(Allocator const &alloc_, Args &&...args)
	{
		typedef typename std::allocator_traits<
			Allocator
		>::template rebind_traits<value_type> value_traits;

		typename value_traits::allocator_type alloc__(alloc_);
		value_traits::construct(
			alloc__,
			reinterpret_cast<value_type *>(&impl.value),
			std::forward<Args>(args)...
		);
	}

	void destroy_value(Allocator const &alloc_)
	{
		typedef typename std::allocator_traits<
			Allocator
		>::template rebind_traits<value_type> value_traits;

		typename value_traits::allocator_type alloc__(alloc_);
		value_traits::destroy(
			alloc__, reinterpret_cast<value_type *>(&impl.value)
		);
	}

	template <typename Allocator_, bool EmptyAlloc = false>
	struct allocated_storage_impl {
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
			sizeof(value_type), alignof(value_type)
		> value;
		std::aligned_storage_t<
			sizeof(Allocator_), alignof(Allocator_)
		> alloc;
	};

	
	template <typename Allocator_>
	struct allocated_storage_impl<Allocator_, true> {
		void emplace_alloc(Allocator_ const &alloc_)
		{
		}

		Allocator_ destroy_alloc()
		{
			return Allocator_();
		}

		std::aligned_storage_t<
			sizeof(value_type), alignof(value_type)
		> value;
	};

	allocated_storage_impl<
		Allocator, std::is_empty<Allocator>::value
	> impl;
};
}
#endif

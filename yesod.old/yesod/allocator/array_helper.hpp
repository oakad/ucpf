/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_DC1D8775449CDF280895B85D03A48CFB)
#define HPP_DC1D8775449CDF280895B85D03A48CFB

#include <memory>

namespace ucpf { namespace yesod { namespace allocator {

template <typename T, typename Alloc>
struct array_helper {
	typedef T value_type;
	typedef typename std::aligned_storage<
		sizeof(value_type), std::alignment_of<value_type>::value
	>::type storage_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::pointer_type pointer;
	typedef typename allocator_traits::size_type size_type;
	typedef typename allocator_traits::difference_type difference_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<storage_type> storage_allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<storage_type> storage_allocator_traits;

	typedef typename storage_allocator_type::pointer_type storage_pointer;

	template <typename Alloc1>
	static auto alloc_s(Alloc1 const &a, size_type n)
	{
		storage_allocator_type x_alloc(a);
		return storage_allocator_traits::allocate(x_alloc, n);
	}

	template <typename Alloc1, typename Pointer>
	static void free_s(Alloc1 const &a, Pointer up, size_type n)
	{
		storage_allocator_type x_alloc(a);
		std::pointer_traits<Pointer>::rebind<storage_type> p(up);

		storage_allocator_traits::deallocate(x_alloc, up, n);
	}

	template <typename Alloc1, typename Pointer, typename... Args>
	static auto make(Alloc1 const &a, Pointer up, Args&&... args)
	{
		allocator_type x_alloc(a);

		allocator_traits::construct(
			x_alloc, value_pointer(up), std::forward<Args>(args)...
		);
		return p;
	}

	template <typename Alloc1, typename... Args>
	static auto alloc(Alloc1 const &a, Args&&... args)
	{
		allocator_type x_alloc(a);
		allocator_guard a_g(
			allocator_traits::allocate(x_alloc, 1),
			x_alloc, 1
		);

		make(a, a_g.p, std::forward<Args>(args)...);
		return a_g.release();
	}

	template <typename Alloc1, typename Pointer, typename... Args>
	static auto make_n(
		Alloc1 const &a, Pointer up, size_type n, Args&&... args
	)
	{
		allocator_type x_alloc(a);
		size_type init_length(0);
		constructor_guard c_g(value_pointer(up), x_alloc, init_length);

		for (; init_length < n; ++init_length)
			allocator_traits::construct(
				x_alloc, &c_g.p[init_length],
				std::forward<Args>(args)...
			);

		return c_g.release();
	}

	template <typename Alloc1, typename... Args>
	static auto alloc_n(Alloc1 const &a, size_type n, Args&&... args)
	{
		allocator_type x_alloc(a);

		auto deleter([&x_alloc, n](T *p) -> void {
			allocator_traits::deallocate(x_alloc, p, n);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			allocator_traits::allocate(x_alloc, n), deleter
			
		);

		make_n(a, s_ptr.get(), n, std::forward<Args>(args)...);
		return s_ptr.release();
	}

	template <typename Alloc1, typename U, typename Iterator>
	static T *make_r(Alloc1 const &a, U *up, Iterator first, Iterator last)
	{
		allocator_type x_alloc(a);
		size_type init_length(0);

		auto deleter([&x_alloc, &init_length](T *p) -> void {
			for (; init_length > 0; --init_length)
				allocator_traits::destroy(
					x_alloc, &p[init_length - 1]
				);
		});
		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			reinterpret_cast<T *>(up), deleter
			
		);

		for (; first != last; ++first)
			allocator_traits::construct(
				x_alloc, &s_ptr[init_length++], *first
			);

		return s_ptr.release();
	}

	template <typename Alloc1, typename Iterator>
	static T *alloc_r(Alloc1 const &a, Iterator first, Iterator last)
	{
		allocator_type x_alloc(a);
		auto n(std::distance(first, last));

		auto deleter([&x_alloc, n](T *p) -> void {
			allocator_traits::deallocate(x_alloc, p, n);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			allocator_traits::allocate(x_alloc, n), deleter
		);

		make_r(a, s_ptr.get(), first, last);
		return s_ptr.release();
	}

	template <typename Alloc1, typename U>
	static void destroy(Alloc1 const &a, U *up, size_type n, bool d)
	{
		allocator_type x_alloc(a);
		auto p(reinterpret_cast<T *>(up));

		for (size_type c(0); c < n; ++c)
			allocator_traits::destroy(x_alloc, &p[c]);

		if (d)
			allocator_traits::deallocate(x_alloc, p, n);
	}

private:
	template <typename Pointer, int Raw = 3>
	struct as_value_pointer
	{
		static pointer get(Pointer const &p)
		{
			return reinterpret_cast<pointer>(p);
		}
	};

	template <typename Pointer>
	struct as_value_pointer<Pointer, 2>
	{
		static pointer get(Pointer const &p)
		{
			return pointer(reinterpret_cast<value_type *>(p));
		}
	};

	template <typename Pointer>
	struct as_value_pointer<Pointer, 1>
	{
		static pointer get(Pointer const &p)
		{
			return reinterpret_cast<pointer>(std::addressof(*p));
		}
	};

	template <typename Pointer>
	struct as_value_pointer<Pointer, 0>
	{
		static pointer get(Pointer const &p)
		{
			return pointer(p);
		}
	};

	static pointer value_pointer(Pointer const &p)
	{
		return as_value_pointer<
			Pointer,
			(std::is_pointer<Pointer>::value ? 2 : 0)
			+ (std::is_pointer<pointer>::value ? 1 : 0)
		>::get(p);
	}

	struct allocator_guard {
		allocator_guard(
			pointer p_, size_type length_, allocator_type &alloc_
		) : p(p_), alloc(alloc_), length(length_)
		{}

		~allocator_guard()
		{
			if (!p)
				return;

			allocator_traits::deallocate(
				alloc, p, length
			);
		}

		auto release()
		{
			auto rv(p);
			p = {};
			return rv;
		}

		pointer p;
		size_type length;
		allocator_type &alloc;
	};

	struct constructor_guard {
		constructor_guard(
			pointer p_, size_type &init_length_,
			allocator_type &alloc_,
			
		) : p(p_), init_length(init_length_), alloc(alloc_)
		{}

		~constructor_guard()
		{
			if (!p)
				return;

			for (; init_length > 0; --init_length)
				allocator_traits::destroy(
					alloc, &p[init_length - 1]
				);
		}

		auto release()
		{
			auto rv(p);
			p = {};
			return rv;
		}

		pointer p;
		size_type &init_length;
		allocator_type &alloc;
	};
};

}}}
#endif

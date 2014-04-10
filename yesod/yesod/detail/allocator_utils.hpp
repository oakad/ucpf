/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_ALLOCATOR_UTILS_20140324T1330)
#define UCPF_YESOD_DETAIL_ALLOCATOR_UTILS_20140324T1330

#include <memory>

namespace ucpf { namespace yesod { namespace detail {

template <typename T>
using aligned_storage_t = typename std::aligned_storage<
	sizeof(T), std::alignment_of<T>::value
>::type;

template <typename T, typename Alloc>
struct allocator_array_helper {
	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<T> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<T> allocator_traits;

	typedef typename allocator_traits::size_type size_type;

	template <typename U, typename... Args>
	static T *make(Alloc const &a, U *up, size_type n, Args&&... args)
	{
		allocator_type x_alloc(a);
		size_type init_length(0);

		auto deleter([&x_alloc, &init_length, n](T *p) -> void {
			for (; init_length > 0; --init_length)
				allocator_traits::destroy(
					x_alloc, &p[init_length - 1]
				);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			reinterpret_cast<T *>(up), deleter
		);

		for (; init_length < n; ++init_length)
			allocator_traits::construct(
				x_alloc, &s_ptr[init_length],
				std::forward<Args>(args)...
			);

		return s_ptr.release();
	}

	template <typename... Args>
	static T *alloc(Alloc const &a, size_type n, Args&&... args)
	{
		allocator_type x_alloc(a);

		auto deleter([&x_alloc, n](T *p) -> void {
			allocator_traits::deallocate(x_alloc, p, n);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			allocator_traits::allocate(x_alloc, n), deleter
			
		);

		make(a, s_ptr.get(), n, std::forward<Args>(args)...);
		return s_ptr.release();
	}

	template <typename U, typename Iterator>
	static T *make(Alloc const &a, U *up, Iterator first, Iterator last)
	{
		allocator_type x_alloc(a);
		size_type n(last - first);
		size_type init_length(0);

		auto deleter([&x_alloc, &init_length, n](T *p) -> void {
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
				x_alloc, &s_ptr[init_length], *first
			);

		return s_ptr.release();
	}

	template <typename Iterator>
	static T *alloc(Alloc const &a, Iterator first, Iterator last)
	{
		allocator_type x_alloc(a);
		size_type n(last - first);

		auto deleter([&x_alloc, n](T *p) -> void {
			allocator_traits::deallocate(x_alloc, p, n);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			allocator_traits::allocate(x_alloc, n), deleter
		);

		make(a, s_ptr.get(), first, last);
		return s_ptr.release();
	}

	static void destroy(Alloc const &a, T *p, size_type n, bool d)
	{
		allocator_type x_alloc(a);
		for (size_type c(0); c < n; ++c)
			allocator_traits::destroy(x_alloc, &p[c]);

		if (d)
			allocator_traits::deallocate(x_alloc, p, n);
	}
};

}}}
#endif


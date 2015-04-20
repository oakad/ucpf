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

template <typename T>
using aligned_storage_t = typename std::aligned_storage<
	sizeof(T), std::alignment_of<T>::value
>::type;

template <typename T, typename Alloc>
struct array_helper {
	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<T> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<T> allocator_traits;

	typedef typename allocator_traits::size_type size_type;
	typedef typename allocator_traits::difference_type difference_type;

	typedef typename std::aligned_storage<
		sizeof(T), std::alignment_of<T>::value
	>::type storage_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<storage_type> storage_allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<storage_type> storage_allocator_traits;

	template <typename Alloc1>
	static storage_type *alloc_s(Alloc1 const &a, size_type n)
	{
		storage_allocator_type x_alloc(a);
		return storage_allocator_traits::allocate(x_alloc, n);
	}

	template <typename Alloc1, typename U>
	static void free_s(Alloc1 const &a, U *p, size_type n)
	{
		storage_allocator_type x_alloc(a);
		storage_allocator_traits::deallocate(
			x_alloc, reinterpret_cast<storage_type *>(p), n
		);
	}

	template <typename Alloc1, typename U, typename... Args>
	static T *make(Alloc1 const &a, U *up, Args&&... args)
	{
		allocator_type x_alloc(a);
		auto p(reinterpret_cast<T *>(up));
		allocator_traits::construct(
			x_alloc, p, std::forward<Args>(args)...
		);
		return p;
	}

	template <typename Alloc1, typename... Args>
	static T *alloc(Alloc1 const &a, Args&&... args)
	{
		allocator_type x_alloc(a);

		auto deleter([&x_alloc](T *p) -> void {
			allocator_traits::deallocate(x_alloc, p, 1);
		});

		std::unique_ptr<T[], decltype(deleter)> s_ptr(
			allocator_traits::allocate(x_alloc, 1), deleter
		);

		make(a, s_ptr.get(), std::forward<Args>(args)...);
		return s_ptr.release();
	}

	template <typename Alloc1, typename U, typename... Args>
	static T *make_n(Alloc1 const &a, U *up, size_type n, Args&&... args)
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

	template <typename Alloc1, typename... Args>
	static T *alloc_n(Alloc1 const &a, size_type n, Args&&... args)
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
};

}}}
#endif

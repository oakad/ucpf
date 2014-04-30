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
#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <typename T>
using aligned_storage_t = typename std::aligned_storage<
	sizeof(T), std::alignment_of<T>::value
>::type;

struct pow2_alloc_policy {
	static size_t best_size(size_t sz)
	{
		return size_t(1) << order_base_2(sz);
	}
};

struct fibonacci_alloc_policy {
	constexpr static double phi = 0x1.9E3779B97F4A7C15F39;
	constexpr static size_t min_size = 8;
	constexpr static size_t min_index = 4;

	static size_t best_size(size_t sz)
	{
		if (sz > min_size) {
			return (size_t(1) << (order_base_2(sz) + 3)) / 5;
		} else
			return min_size;
	}
};

template <typename T, typename Alloc>
struct allocator_array_helper {
	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<T> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<T> allocator_traits;

	typedef typename allocator_traits::size_type size_type;

	template <typename Alloc1, typename U, typename... Args>
	static T *make(Alloc1 const &a, U *up, size_type n, Args&&... args)
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
	static T *alloc(Alloc1 const &a, size_type n, Args&&... args)
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

	template <typename Alloc1, typename U, typename Iterator>
	static T *make(Alloc1 const &a, U *up, Iterator first, Iterator last)
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

	template <typename Alloc1, typename Iterator>
	static T *alloc(Alloc1 const &a, Iterator first, Iterator last)
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

	template <typename Alloc1>
	static void destroy(Alloc1 const &a, T *p, size_type n, bool d)
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


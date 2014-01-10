/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_UNIQUE_PTR_JAN_10_2014_1530)
#define UCPF_YESOD_UNIQUE_PTR_JAN_10_2014_1530

#include <yesod/array_element_count.hpp>

namespace ucpf { namespace yesod {

template <typename T, typename Alloc = std::allocator<void>>
struct unique_ptr {
	typedef typename std::remove_all_extents<T>::type element_type;
	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		element_type
	> allocator_type;
	typedef typename std::allocator_traits<
		allocator_type
	> allocator_traits_type;
	typedef typename allocator_traits_type::pointer pointer;

	template <typename Alloc1, typename Args... args>
	unique_ptr(Alloc1 const &a_, Args&&... args)
	{
		allocator_type a(a_);

		auto p(allocator_traits_type::allocate(a, alloc_arity));
		size_t cnt(0);

		try {
			for (; c < alloc_arity; ++c)
				allocator_traits_type::construct(
					a, &p[c], std::forward<Args>(args)...
				);
		} catch (...) {
			if (c > 0) {
				--c;
				for (--c; c >= 0; --c)
					allocator_traits_type::destroy(
						a, &p[c]
					);
			}

			allocator_traits::deallocate(a, p, alloc_arity);
			throw;
		}
		value = make_tuple(a, p);
	}

	template <typename Args... args>
	unique_ptr(Args&&... args)
	: unique_ptr(allocator_type(), std::forward<Args>(args)...)
	{}

private:
	constexpr static size_t alloc_arity = array_element_count<T>::value;

	std::tuple<pointer, allocator_type> value;
};

}}

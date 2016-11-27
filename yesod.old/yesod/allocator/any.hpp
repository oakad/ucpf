/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_51606FFFF8155F61CFC78ACA54B459EF)
#define HPP_51606FFFF8155F61CFC78ACA54B459EF

#include <memory>

namespace ucpf { namespace yesod { namespace allocator {

struct any {
	template <typename Alloc>
	any(Alloc const &a)
	: alloc_access_ptr(alloc_access<Alloc>),
	  alloc_ptr(reinterpret_cast<void const *>(&a))
	{}

	void *alloc(std::size_t count)
	{
		void *ptr(nullptr);
		(*alloc_access_ptr)(alloc_ptr, &ptr, count, false);
		return ptr;
	}

	void free(void *ptr, std::size_t count)
	{
		(*alloc_access_ptr)(alloc_ptr, &ptr, count, true);
	}

private:
	template <typename Alloc1>
	static void alloc_access(
		void const *alloc_ptr, void **ptr, std::size_t count,
		bool release
	)
	{
		typedef typename std::allocator_traits<
			Alloc1
		>::template rebind_alloc<char> alloc_type;

		typedef typename std::allocator_traits<
			Alloc1
		>::template rebind_traits<char> alloc_traits;

		typedef typename alloc_traits::pointer ptr_type;

		alloc_type a(*reinterpret_cast<Alloc1 const *>(alloc_ptr));

		if (release)
			alloc_traits::deallocate(
				a, reinterpret_cast<ptr_type>(*ptr), count
			);
		else
			*ptr = alloc_traits::allocate(a, count);
	}

	void (*alloc_access_ptr)(
		void const *alloc_ptr, void **ptr, std::size_t count,
		bool release
	);
	void const *alloc_ptr;
};

}}}
#endif

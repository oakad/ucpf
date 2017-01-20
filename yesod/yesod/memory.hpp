/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_DCB0D4AE9F6AA349BC9F0FA94D18D5C6)
#define HPP_DCB0D4AE9F6AA349BC9F0FA94D18D5C6

#include <experimental/memory_resource>

namespace ucpf::yesod {

namespace pmr = std::experimental::pmr;

template<class ForwardIt, class Size>
ForwardIt uninitialized_default_construct_n(ForwardIt first, Size n)
{
	typedef typename std::iterator_traits<
		ForwardIt
	>::value_type value_type;
	ForwardIt current = first;
	try {
		for (; n > 0; (void)++current, --n)
			::new(static_cast<void*>(
				std::addressof(*current)
			)) value_type;

		return current;
	} catch (...) {
		for (; first != current; ++first)
			first->~value_type();

		throw;
	}
}

template<class T>
void destroy_at(T* p) 
{ 
	p->~T(); 
}

template<class ForwardIt, class Size>
ForwardIt destroy_n(ForwardIt first, Size n)
{
	for (; n > 0; (void)++first, --n)
		destroy_at(std::addressof(*first));

	return first;
}

}
#endif

/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_5C4DD8F84769EE6834EE55981DA64A55)
#define HPP_5C4DD8F84769EE6834EE55981DA64A55

namespace ucpf { namespace yesod { namespace storage {
namespace detail {

template <typename ElementType, typename StorageType>
struct alloc_helper {
	alloc_guard(StorageType &st_)
	: st(st_), p(st.allocate(sizeof(ElementType), alignof(ElementType))),
	  release(true)
	{}

	~alloc_guard()
	{
		if (release)
			st.deallocate(
				p, sizeof(ElementType), alignof(ElementType)
			);
	}

	StorageType &st;
	typename StorageType::storage_pointer p;
	bool release;
};

}

template <typename ElementType, typename StorageType, typename... Args>
auto alloc(StorageType &st, Args&&... args)
{
	detail::alloc_helper<ElementType, StorageType> helper(st);
	auto rv(st.construct(helper.p, std::forward<Args>(args)...));
	helper.release = false;
	return rv;
}

template <typename ElementType, typename StorageType>
void free(StorageType &st, StorageType::template pointer<ElementType> p)
{
	auto pp(st.storage_cast(p));
	st.destroy(p);
	st.deallocate(pp, sizeof(ElementType), alignof(ElementType));
}

}}}
#endif

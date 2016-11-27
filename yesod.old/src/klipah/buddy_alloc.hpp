/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_78B44CB20182A2E160548D02C6A32FCE)
#define HPP_78B44CB20182A2E160548D02C6A32FCE

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {

struct buddy_alloc_base {
	virtual void *allocate(int ord) const = 0;

	virtual void deallocate(void *p, int ord) const = 0;
};

template <std::size_t BaseSize, std::size_t MinOrder, std::size_t MaxOrder>
struct buddy_alloc : buddy_alloc_base {
	virtual void *allocate(int ord) const
	{
		printf(
			"small alloc %zd, ord %d, sz %zd\n",
			BaseSize, ord, BaseSize << ord
		);
		return nullptr;
	}

	virtual void deallocate(void *p, int ord) const
	{
	}
};

}}}}
#endif

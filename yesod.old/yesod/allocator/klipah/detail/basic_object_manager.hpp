/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_9F6110772A4D901B9AD030D87A4DE2CF)
#define HPP_9F6110772A4D901B9AD030D87A4DE2CF

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {

struct basic_object_manager {
	template <typename MutexType>
	void init_mutex(MutexType &m)
	{
		MutexType::init(m);
	}

	template <typename PtrType, typename RawPtrType>
	PtrType &&wrap_pointer(RawPtrType p)
	{
		return PtrType(p);
	}
};

}
}}}}
#endif

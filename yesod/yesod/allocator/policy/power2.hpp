/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ALLOCATOR_POLICY_POWER2_20140502T1820)
#define UCPF_YESOD_ALLOCATOR_POLICY_POWER2_20140502T1820

#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod { namespace allocator { namespace policy {

struct power2 {
	static size_t best_size(size_t sz)
	{
		return size_t(1) << order_base_2(sz);
	}
};

}}}}
#endif

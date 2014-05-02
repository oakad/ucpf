/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ALLOCATOR_POLICY_FIBONACCI_20140502T1820)
#define UCPF_YESOD_ALLOCATOR_POLICY_FIBONACCI_20140502T1820

#include <yesod/mpl/value_cast.hpp>
#include <yesod/mpl/fibonacci_c.hpp>

namespace ucpf { namespace yesod { namespace allocator { namespace policy {

struct fibonacci {
	typedef typename mpl::value_cast<
		typename mpl::fibonacci_c<uint32_t, 46>::type
	> alloc_size;

	static size_t best_size(size_t sz)
	{
		if (sz <= alloc_size::value.back())
			return *std::lower_bound(
				alloc_size::value.begin(),
				alloc_size::value.end(),
				uint32_t(sz)
			);
		else {
			size_t prev(alloc_size::value[
				alloc_size::value.size() - 2
			]);
			size_t rv(prev + alloc_size::value.back());
			prev = alloc_size::value.back();
			while (rv < sz) {
				auto next(rv + prev);
				prev = rv;
				rv = next;
			}
			return rv;
		}
	}
};

}}}}
#endif

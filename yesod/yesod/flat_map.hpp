/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_FLAT_MAP_20140411T1915)
#define UCPF_YESOD_FLAT_MAP_20140411T1915

#include <yesod/detail/flat_map_base.hpp>
#include <yesod/detail/flat_map_ops.hpp>

namespace ucpf { namespace yesod {

template <
	typename KeyType, typename T, typename CompareF, typename Alloc
> struct flat_map {
	typedef KeyType key_type;
	typedef T       mapped_type;
	typedef std::pair<key_type const, mapped_type> value_type;
private:
	struct select_first {
		static key_type const &apply(value_type const &p)
		{
			return std::get<0>(p);
		}
	};

	detail::flat_map_impl<
		key_type, value_type, select_first, CompareF, Alloc
	> base_map;
};

template <
	typename KeyType, typename CompareF, typename Alloc
> struct flat_set {

private:
	struct identity {
		static key_type const &apply(key_type const &k)
		{
			return k;
		}
	};

	detail::flat_map_impl<
		key_type, key_type, identity, CompareF, Alloc
	> base_map;
};

}}
#endif

/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_FEE9D34D2A71F704449B75337A8C8E7C)
#define HPP_FEE9D34D2A71F704449B75337A8C8E7C

#include <yesod/string_map/detail/string_map_impl.hpp>

namespace ucpf { namespace yesod { namespace string_map {

template <typename...>
struct mutable_map;

template <typename ValueType>
struct mutable_map<ValueType> : detail::mutable_map_impl<
	ValueType, char, store::radix_tree, storage::memory, void, void
> {
};

}}}
#endif

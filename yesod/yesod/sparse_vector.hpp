/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SPARSE_VECTOR_JAN_06_2014_1320)
#define UCPF_YESOD_SPARSE_VECTOR_JAN_06_2014_1320

#include <yesod/detail/sparse_vector_base.hpp>
#include <yesod/detail/sparse_vector_ops.hpp>

namespace ucpf { namespace yesod {

struct default_sparse_vector_policy {
	typedef std::allocator<void> allocator_type;
	constexpr static size_t ptr_node_order = 6;
	constexpr static size_t data_node_order = 6;
};

template <
	typename ValueType, typename Policy = default_sparse_vector_policy
> struct sparse_vector;

}}
#endif
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
//#include <yesod/detail/sparse_vector_ops.hpp>

namespace ucpf { namespace yesod {

template <typename ValueType>
struct sparse_vector_default_policy {
	typedef std::allocator<void> allocator_type;
	constexpr static std::array<
		std::size_t, 3
	> ptr_node_order = {{2, 4, 6}};
	constexpr static std::array<std::size_t, 2> data_node_order = {{6, 8}};
	/* optional value_valid_pred type */
};

template <
	typename ValueType,
	typename Policy = sparse_vector_default_policy<ValueType>
> struct sparse_vector;

}}
#endif

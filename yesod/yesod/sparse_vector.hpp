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

struct sparse_vector_default_policy {
	typedef std::allocator<void> allocator_type;
	constexpr static std::array<
		std::size_t, 3
	> ptr_node_order = {{2, 3, 4}};
	constexpr static std::array<
		std::size_t, 2
	> data_node_order = {{5, 10}};
};

template <>
struct sparse_vector<> {
	template <typename ValueType, typename Policy>
	struct rebind {
		typedef sparse_vector<ValueType, Policy> other;
	};
};


template <typename ValueType>
struct sparse_vector<ValueType> : sparse_vector<
	ValueType, sparse_vector_default_policy
> {};

}}
#endif

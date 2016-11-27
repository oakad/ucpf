/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_POD_SPARSE_VECTOR_20141031T1700)
#define UCPF_YESOD_POD_SPARSE_VECTOR_20141031T1700

#include <yesod/detail/pod_sparse_vector_base.hpp>

namespace ucpf { namespace yesod {

struct pod_sparse_vector_default_policy {
	typedef std::allocator<void> allocator_type;
	constexpr static std::size_t ptr_node_order = 6;
	constexpr static std::size_t data_node_order = 8;
};

template <>
struct pod_sparse_vector<> {
	template <typename ValueType, typename Policy>
	struct rebind {
		typedef pod_sparse_vector<ValueType, Policy> other;
	};
};


template <typename ValueType>
struct pod_sparse_vector<ValueType> : pod_sparse_vector<
	ValueType, pod_sparse_vector_default_policy
> {};

}}
#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_DETAIL_HAS_KEY_DEC_11_2013_1600)
#define UCPF_YESOD_MPL_DETAIL_HAS_KEY_DEC_11_2013_1600

#include <yesod/mpl/sequence_tag_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Tag>
struct has_key_impl {
	template <typename Sequence, typename Key>
	struct apply;
};

template <>
struct has_key_impl<non_sequence_tag> {};

}}}}

#endif

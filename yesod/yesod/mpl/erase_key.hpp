/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_ERASE_KEY_DEC_11_2013_1530)
#define UCPF_YESOD_MPL_ERASE_KEY_DEC_11_2013_1530

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/sequence_tag.hpp>
#include <yesod/mpl/detail/erase_key.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct erase_key<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : erase_key<T0, T1> {};
};

template <typename Tag>
struct lambda<erase_key<>, Tag, long_<-1>> {
        typedef false_type is_le;
        typedef erase_key<> result_;
        typedef erase_key<> type;
};

template<typename Sequence, typename Key>
struct erase_key<Sequence, Key> : detail::erase_key_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, Key> {};

}}}

#endif

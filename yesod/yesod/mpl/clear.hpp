/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_CLEAR_DEC_11_2013_1515)
#define UCPF_YESOD_MPL_CLEAR_DEC_11_2013_1515

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/detail/clear.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct clear<> {
	template <typename T0, typename... Tn>
	struct apply : clear<T0> {};
};

template <typename Tag>
struct lambda<clear<>, Tag, long_<-1>> {
        typedef false_type is_le;
        typedef clear<> result_;
        typedef clear<> type;
};

template <typename Sequence>
struct clear<Sequence> : detail::clear_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence>
{};

}}}

#endif

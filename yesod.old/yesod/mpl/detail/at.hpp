/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_AT_DEC_11_2013_1600)
#define UCPF_YESOD_MPL_DETAIL_AT_DEC_11_2013_1600

#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/begin_end.hpp>
#include <yesod/mpl/sequence_tag_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Tag>
struct at_impl {
	template <typename Sequence, typename N>
	struct apply {
		typedef typename advance<
			typename begin<Sequence>::type, N
		>::type iter_;

		typedef typename deref<iter_>::type type;
	};
};

template <>
struct at_impl<non_sequence_tag> {};

}}}}

#endif

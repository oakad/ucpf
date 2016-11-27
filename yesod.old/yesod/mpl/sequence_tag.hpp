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

#if !defined(UCPF_YESOD_MPL_SEQUENCE_TAG_DEC_11_2013_1420)
#define UCPF_YESOD_MPL_SEQUENCE_TAG_DEC_11_2013_1420

#include <yesod/mpl/sequence_tag_fwd.hpp>
#include <yesod/mpl/detail/has_tag.hpp>
#include <yesod/mpl/detail/has_begin.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <bool has_tag_, bool has_begin_>
struct sequence_tag_impl {
	template <typename Sequence> struct result_;
};

template <>
struct sequence_tag_impl<true, true> {
	template <typename Sequence> struct result_ {
		typedef typename Sequence::tag type;
	};
};

template <>
struct sequence_tag_impl<true, false> {
	template <typename Sequence> struct result_ {
		typedef typename Sequence::tag type;
	};
};

template <>
struct sequence_tag_impl<false, true> {
	template <typename Sequence> struct result_ {
		typedef nested_begin_end_tag type;
	};
};

template <>
struct sequence_tag_impl<false, false> {
	template <typename Sequence> struct result_ {
		typedef non_sequence_tag type;
	};
};

}

template <typename Sequence>
struct sequence_tag : detail::sequence_tag_impl<
	detail::has_tag<Sequence>::value,
	detail::has_begin<Sequence>::value
>::template result_<Sequence> {};

}}}

#endif

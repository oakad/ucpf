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

#if !defined(UCPF_YESOD_MPL_DETAIL_BEGIN_END_DEC_11_2013_1630)
#define UCPF_YESOD_MPL_DETAIL_BEGIN_END_DEC_11_2013_1630

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/void.hpp>
#include <yesod/mpl/sequence_tag_fwd.hpp>
#include <yesod/mpl/detail/has_begin.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Sequence>
struct begin_type {
	typedef typename Sequence::begin type;
};

template <typename Sequence>
struct end_type {
	typedef typename Sequence::end type;
};

template <typename Tag>
struct begin_impl {
	template <typename Sequence>
	struct apply {
		typedef typename eval_if<
			has_begin<Sequence>,
			begin_type<Sequence>,
			void_
                >::type type;
	};
};

template <typename Tag>
struct end_impl {
	template <typename Sequence>
	struct apply {
		typedef typename eval_if<
			has_begin<Sequence>,
			end_type<Sequence>,
			void_
		>::type type;
	};
};

template <>
struct begin_impl<nested_begin_end_tag> {
	template <typename Sequence>
	struct apply {
		typedef typename Sequence::begin type;
	};
};

template <>
struct end_impl<nested_begin_end_tag> {
	template <typename Sequence>
	struct apply {
		typedef typename Sequence::end type;
	};
};

template <>
struct begin_impl<non_sequence_tag> {
	template <typename Sequence>
	struct apply {
		typedef void_ type;
	};
};

template <>
struct end_impl<non_sequence_tag> {
	template <typename Sequence>
	struct apply {
		typedef void_ type;
	};
};

}}}}

#endif

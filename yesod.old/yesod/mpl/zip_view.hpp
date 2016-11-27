/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2010 Aleksey Gurtovoy
    Copyright (c) 2000-2002 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_ZIP_VIEW_DEC_17_2013_1810)
#define UCPF_YESOD_MPL_ZIP_VIEW_DEC_17_2013_1810

#include <yesod/mpl/transform.hpp>
#include <yesod/mpl/iterator_tags.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename IteratorSeq>
struct zip_iterator {
	typedef forward_iterator_tag category;
	typedef typename transform1<IteratorSeq, deref<arg<0>>>::type type;

	typedef zip_iterator<
		typename transform1<IteratorSeq, mpl::next<arg<0>>>::type
	> next;
};

template <typename...>
struct zip_view;

template <>
struct zip_view<> {
	template <typename T0, typename... Tn>
	struct apply : zip_view<T0> {};
};

template <typename Tag>
struct lambda<zip_view<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef zip_view<> result_;
	typedef zip_view<> type;
};

template <typename Sequences>
struct zip_view<Sequences> {
	typedef typename transform1<
		Sequences, mpl::begin<arg<0>>
	>::type first_ones_;
	typedef typename transform1<
		Sequences, mpl::end<arg<0>>
	>::type last_ones_;

	typedef nested_begin_end_tag tag;
	typedef zip_view type;
	typedef zip_iterator<first_ones_> begin;
	typedef zip_iterator<last_ones_> end;
};

}}}

#endif

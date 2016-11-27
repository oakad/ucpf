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
    Copyright (c) 2004      David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_MIN_MAX_ELEMENT_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_MIN_MAX_ELEMENT_DEC_17_2013_1730

#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/iter_fold.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Predicate>
struct select_max {
	template <typename OldIterator, typename Iterator>
	struct apply {
		typedef typename mpl::apply<
			Predicate, typename deref<OldIterator>::type,
			typename deref<Iterator>::type
		>::type condition_;

		typedef typename if_<
			condition_, Iterator, OldIterator
		>::type type;
	};
};

}

template <typename...>
struct max_element;

template <>
struct max_element<> {
	template <typename T0, typename... Tn>
	struct apply : max_element<T0> {};
};

template <typename Tag>
struct lambda<max_element<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef max_element<> result_;
	typedef max_element<> type;
	
};

template <typename Sequence>
struct max_element<Sequence> : iter_fold<
	Sequence, typename begin<Sequence>::type,
	protect<detail::select_max<less<arg<-1>, arg<-1>>>>
> {};

template <typename Sequence, typename Predicate>
struct max_element<Sequence, Predicate> : iter_fold<
	Sequence, typename begin<Sequence>::type,
	protect<detail::select_max<Predicate>>
> {};

template <typename...>
struct min_element;

template <>
struct min_element<> {
	template <typename T0, typename... Tn>
	struct apply : min_element<T0> {};
};

template <typename Tag>
struct lambda<min_element<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef min_element<> result_;
	typedef min_element<> type;
}; 

template <typename Sequence>
struct min_element<Sequence> : iter_fold<
	Sequence, typename begin<Sequence>::type,
	protect<detail::select_max<not_<less<arg<-1>, arg<-1>>>>>
> {};

template <typename Sequence, typename Predicate>
struct min_element<Sequence, Predicate>
: max_element<Sequence, not_<Predicate>> {};

}}}

#endif

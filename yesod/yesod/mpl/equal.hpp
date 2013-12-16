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

#if !defined(UCPF_YESOD_MPL_EQUAL_DEC_16_2013_1740)
#define UCPF_YESOD_MPL_EQUAL_DEC_16_2013_1740

#include <yesod/mpl/arg.hpp>
#include <yesod/mpl/begin_end.hpp>
#include <yesod/mpl/iter_fold_if.hpp>
#include <yesod/mpl/detail/iter_apply.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Predicate, typename LastIterator1, typename LastIterator2>
struct equal_pred {
	template <typename Iterator2, typename Iterator1>
	struct apply {
		typedef typename and_<
			not_<std::is_same<Iterator1, LastIterator1>>,
			not_<std::is_same<Iterator2, LastIterator2>>,
			iter_apply<Predicate, Iterator1, Iterator2>
		>::type type;
	};
};

template <typename Sequence1, typename Sequence2, typename Predicate>
struct equal_impl {
	typedef typename begin<Sequence1>::type first1_;
	typedef typename begin<Sequence2>::type first2_;
	typedef typename end<Sequence1>::type last1_;
	typedef typename end<Sequence2>::type last2_;

	typedef iter_fold_if_impl<
		first1_, first2_, next<>,
		protect<equal_pred<Predicate, last1_, last2_>>,
		void_, always<false_type>
	> fold_;

	typedef typename fold_::iterator iter1_;
	typedef typename fold_::state iter2_;
	typedef and_<
		std::is_same<iter1_, last1_>, std::is_same<iter2_, last2_>
	> result_;

	typedef typename result_::type type;
};

}

template <
	typename Sequence1, typename Sequence2,
	typename Predicate = std::is_same<arg<-1>, arg<-1>>
> struct equal : detail::equal_impl<Sequence1, Sequence2, Predicate>::type
{};

}}}

#endif

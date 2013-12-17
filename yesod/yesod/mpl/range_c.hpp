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

#if !defined(UCPF_YESOD_MPL_RANGE_C_DEC_17_2013_1600)
#define UCPF_YESOD_MPL_RANGE_C_DEC_17_2013_1600

#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/arithmetic.hpp>
#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/iterator_tags.hpp>

#include <yesod/mpl/detail/size.hpp>
#include <yesod/mpl/detail/advance.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct half_open_range_tag;
struct range_c_iterator_tag;

template <typename N>
struct range_c_iterator {
	typedef range_c_iterator_tag tag;
	typedef random_access_iterator_tag category;
	typedef N type;
};

template <>
struct advance_impl<range_c_iterator_tag> {
	template <typename Iter, typename Dist>
	struct apply {
		typedef typename deref<Iter>::type n_;
		typedef typename plus<n_, Dist>::type m_;

		typedef integral_constant<
			typename n_::value_type, m_::value
		> result_;

		typedef range_c_iterator<result_> type;
	};
};

template <>
struct back_impl<half_open_range_tag> {
	template <typename Range>
	struct apply {
		typedef typename prior<typename Range::finish>::type type;
	};
};

template <>
struct distance_impl<range_c_iterator_tag> {
	template <typename Iter1, typename Iter2>
	struct apply : minus<
		typename Iter2::type, typename Iter1::type
	> {};
};

template <>
struct empty_impl<half_open_range_tag> {
	template <typename Range>
	struct apply : equal_to<
		typename Range::start, typename Range::finish
	> {};
};

template <>
struct front_impl<half_open_range_tag> {
	template <typename Range>
	struct apply {
		typedef typename Range::start type;
	};
};

template <>
struct size_impl<half_open_range_tag> {
	template <typename Range>
	struct apply : minus<
		typename Range::finish, typename Range::start
	> {};
};

template <>
struct O1_size_impl<half_open_range_tag> : size_impl<half_open_range_tag> {};


}

template <typename N>
struct next<detail::range_c_iterator<N>> {
	typedef detail::range_c_iterator<typename next<N>::type> type;
};

template <typename N>
struct prior<detail::range_c_iterator<N>> {
	typedef detail::range_c_iterator<typename prior<N>::type> type;
};

template <typename T, T Start, T Finish>
struct range_c {
	typedef detail::half_open_range_tag tag;
	typedef T value_type;
	typedef range_c type;

	typedef integral_constant<T, Start> start;
	typedef integral_constant<T, Finish> finish;

	typedef detail::range_c_iterator<start> begin;
	typedef detail::range_c_iterator<finish> end;
};

}}}

#endif

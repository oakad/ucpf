/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_FOLD_DEC_16_2013_1400)
#define UCPF_YESOD_MPL_FOLD_DEC_16_2013_1400

#include <yesod/mpl/arg.hpp>
#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/begin_end.hpp>
#include <yesod/mpl/fold_assoc_iter.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct fold;

template <>
struct fold<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : fold<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<fold<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef fold<> result_;
	typedef fold<> type;
};

template <typename Sequence, typename State, typename Op>
struct fold<Sequence, State, Op> {
	typedef typename fold_assoc_iter<
		typename detail::op_assoc::left,
		typename lambda<Op>::type::template apply<
			arg<0>, deref<arg<1>>
		>,
		State,
		typename begin<Sequence>::type,
		typename end<Sequence>::type
	>::type type;
};

template <typename...>
struct reverse_fold;

template <>
struct reverse_fold<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : fold<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_fold<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_fold<> result_;
	typedef reverse_fold<> type;
};

template <typename Sequence, typename State, typename Op>
struct reverse_fold<Sequence, State, Op> {
	typedef typename fold_assoc_iter<
		typename detail::op_assoc::right,
		typename lambda<Op>::type::template apply<
			arg<1>, deref<arg<0>>
		>,
		State,
		typename begin<Sequence>::type,
		typename end<Sequence>::type
	>::type type;
};

}}}

#endif

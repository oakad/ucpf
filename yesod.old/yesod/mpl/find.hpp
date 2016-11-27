/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2002 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_FIND_DEC_16_2013_1300)
#define UCPF_YESOD_MPL_FIND_DEC_16_2013_1300

#include <yesod/mpl/arg.hpp>
#include <yesod/mpl/same_as.hpp>
#include <yesod/mpl/iter_fold_if.hpp>
#include <yesod/mpl/detail/iter_apply.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Predicate>
struct find_if_pred {
	template <typename Iterator>
	struct apply {
		typedef not_<iter_apply<Predicate, Iterator>> type;
	};
};

}

template <typename...>
struct find_if;

template <>
struct find_if<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : find_if<T0, T1> {};
};

template <typename Tag>
struct lambda<find_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef find_if<> result_;
	typedef find_if<> type;
};

template <typename Sequence, typename Predicate>
struct find_if<Sequence, Predicate> {
	typedef typename iter_fold_if<
		Sequence, void, arg<0>,
		protect<detail::find_if_pred<Predicate>>
	>::type result_;

	typedef typename second<result_>::type type;
};

template <typename...>
struct find;

template <>
struct find<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : find<T0, T1> {};
};

template <typename Tag>
struct lambda<find<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef find<> result_;
	typedef find<> type;
};

template <typename Sequence, typename T>
struct find<Sequence, T> : find_if <Sequence, same_as<T>> {};

}}}

#endif

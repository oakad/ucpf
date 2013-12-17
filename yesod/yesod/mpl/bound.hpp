/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2001-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_BOUND_DEC_17_2013_1550)
#define UCPF_YESOD_MPL_BOUND_DEC_17_2013_1550

#include <yesod/mpl/size.hpp>
#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct lower_bound_step_impl;

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct lower_bound_step {
	typedef typename eval_if<
		Distance,
		lower_bound_step_impl<Distance, Predicate, T, DeferredIterator>,
		DeferredIterator
	>::type type;
};

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct lower_bound_step_impl {
	typedef typename divides<Distance, long_<2>>::type offset_;
	typedef typename DeferredIterator::type iter_;
	typedef typename advance<iter_, offset_>::type middle_;
	typedef typename apply<
		Predicate, typename deref<middle_>::type, T
	>::type cond_;

	typedef typename prior<minus<Distance, offset_>>::type step_;
	typedef lower_bound_step<
		offset_, Predicate, T, DeferredIterator
	> step_forward_;
	typedef lower_bound_step<
		step_, Predicate, T, next<middle_>
	> step_backward_;
	typedef typename eval_if<
		cond_, step_backward_, step_forward_
	>::type type;
};

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct upper_bound_step_impl;

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct upper_bound_step {
	typedef typename eval_if<
		Distance,
		upper_bound_step_impl<Distance, Predicate, T, DeferredIterator>,
		DeferredIterator
	>::type type;
};

template <
	typename Distance, typename Predicate, typename T,
	typename DeferredIterator
> struct upper_bound_step_impl {
	typedef typename divides<Distance, long_<2>>::type offset_;
	typedef typename DeferredIterator::type iter_;
	typedef typename advance<iter_, offset_>::type middle_;
	typedef typename apply<
		Predicate, T, typename deref<middle_>::type
	>::type cond_;

	typedef typename prior<minus<Distance, offset_>>::type step_;
	typedef upper_bound_step<
		offset_, Predicate, T, DeferredIterator
	> step_forward_;
	typedef upper_bound_step<
		step_, Predicate, T, next<middle_>
	> step_backward_;
	typedef typename eval_if<
		cond_, step_forward_, step_backward_
	>::type type;
};

}

template <typename...>
struct lower_bound;

template <>
struct lower_bound<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : lower_bound<T0, T1> {};
};

template <typename Tag>
struct lambda<lower_bound<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef lower_bound<> result_;
	typedef lower_bound<> type;
};

template <typename Sequence, typename T>
struct lower_bound<Sequence, T> : lower_bound<Sequence, T, less<>> {};

template <typename Sequence, typename T, typename Predicate>
struct lower_bound<Sequence, T, Predicate> {
	typedef typename lambda<Predicate>::type pred_;
	typedef typename size<Sequence>::type size_;

	typedef typename detail::lower_bound_step<
		size_, pred_, T, begin<Sequence>
	>::type type;
};

template <typename...>
struct upper_bound;

template <>
struct upper_bound<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : upper_bound<T0, T1> {};
};

template <typename Tag>
struct lambda<upper_bound<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef upper_bound<> result_;
	typedef upper_bound<> type;
};

template <typename Sequence, typename T>
struct upper_bound<Sequence, T> : upper_bound<Sequence, T, less<>> {};

template <typename Sequence, typename T, typename Predicate>
struct upper_bound<Sequence, T, Predicate> {
	typedef typename lambda<Predicate>::type pred_;
	typedef typename size<Sequence>::type size_;

	typedef typename detail::upper_bound_step<
		size_, pred_, T, begin<Sequence>
	>::type type;
};

}}}

#endif

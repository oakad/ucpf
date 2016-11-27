/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2003-2004 Aleksey Gurtovoy
    Copyright (c) 2003      Eric Friedman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_ITER_FOLD_IF_DEC_16_2013_1315)
#define UCPF_YESOD_MPL_ITER_FOLD_IF_DEC_16_2013_1315

#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/always.hpp>
#include <yesod/mpl/begin_end.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Iterator, typename State>
struct iter_fold_if_null_step {
	typedef State state;
	typedef Iterator iterator;
};

template <bool>
struct iter_fold_if_step_impl {
	template<
		typename Iterator, typename State, typename StateOp,
		typename IteratorOp
	> struct result_ {
		typedef typename apply<StateOp, State, Iterator>::type state;
		typedef typename IteratorOp::type iterator;
	};
};

template<>
struct iter_fold_if_step_impl<false> {
	template<
		typename Iterator, typename State, typename StateOp,
		typename IteratorOp
	> struct result_ {
		typedef State state;
		typedef Iterator iterator;
	};
};

template <
	typename Iterator, typename State, typename ForwardOp,
	typename Predicate
> struct iter_fold_if_forward_step {
	typedef typename apply<Predicate, State, Iterator>::type not_last;
	typedef typename iter_fold_if_step_impl<
		not_last::value
	>::template result_<
		Iterator, State, ForwardOp, next<Iterator>
	> impl_;

	typedef typename impl_::state state;
	typedef typename impl_::iterator iterator;
};

template <
	typename Iterator, typename State, typename BackwardOp,
	typename Predicate
> struct iter_fold_if_backward_step {
	typedef typename apply<Predicate, State, Iterator>::type not_last;
	typedef typename iter_fold_if_step_impl<
		not_last::value
	>::template result_<
		Iterator, State, BackwardOp, identity<Iterator>
	> impl_;

	typedef typename impl_::state state;
	typedef typename impl_::iterator iterator;
};

template <
	typename Iterator, typename State, typename ForwardOp,
	typename ForwardPredicate, typename BackwardOp,
	typename BackwardPredicate
> struct iter_fold_if_impl {
private:
	typedef iter_fold_if_null_step<Iterator, State> forward_step0;

	typedef iter_fold_if_forward_step<
		typename forward_step0::iterator,
		typename forward_step0::state,
		ForwardOp, ForwardPredicate
	> forward_step1;

	typedef iter_fold_if_forward_step<
		typename forward_step1::iterator,
		typename forward_step1::state,
		ForwardOp, ForwardPredicate
	> forward_step2;

	typedef iter_fold_if_forward_step<
		typename forward_step2::iterator,
		typename forward_step2::state,
		ForwardOp, ForwardPredicate
	> forward_step3;

	typedef iter_fold_if_forward_step<
		typename forward_step3::iterator,
		typename forward_step3::state,
		ForwardOp, ForwardPredicate
	> forward_step4;

	typedef typename if_<
		typename forward_step4::not_last,
		iter_fold_if_impl<
			typename forward_step4::iterator,
			typename forward_step4::state,
			ForwardOp, ForwardPredicate,
			BackwardOp, BackwardPredicate
		>,
		iter_fold_if_null_step<
			typename forward_step4::iterator,
			typename forward_step4::state
		>
	>::type backward_step4;

	typedef iter_fold_if_backward_step<
		typename forward_step3::iterator,
		typename backward_step4::state,
		BackwardOp, BackwardPredicate
	> backward_step3;

	typedef iter_fold_if_backward_step<
		typename forward_step2::iterator,
		typename backward_step3::state,
		BackwardOp, BackwardPredicate
	> backward_step2;

	typedef iter_fold_if_backward_step<
		typename forward_step1::iterator,
		typename backward_step2::state,
		BackwardOp, BackwardPredicate
	> backward_step1;

	typedef iter_fold_if_backward_step<
		typename forward_step0::iterator,
		typename backward_step1::state,
		BackwardOp, BackwardPredicate
	> backward_step0;

public:
	typedef typename backward_step0::state state;
	typedef typename backward_step4::iterator iterator;
};

template <typename Predicate, typename LastIterator>
struct iter_fold_if_pred {
	template <typename State, typename Iterator>
	struct apply : and_<
		not_<std::is_same<Iterator, LastIterator>>,
		mpl::apply<Predicate, Iterator>
	> {};
};

}

template <typename...>
struct iter_fold_if;

template <
	typename Sequence, typename State, typename ForwardOp,
	typename ForwardPredicate
> struct iter_fold_if<
	Sequence, State, ForwardOp, ForwardPredicate
> : iter_fold_if<
	Sequence, State, ForwardOp, ForwardPredicate,
	false_type, always<false_type>
> {};

template <
	typename Sequence, typename State, typename ForwardOp,
	typename ForwardPredicate, typename BackwardOp
> struct iter_fold_if<
	Sequence, State, ForwardOp, ForwardPredicate, BackwardOp
> : iter_fold_if<
	Sequence, State, ForwardOp, ForwardPredicate,
	BackwardOp, always<true_type>
> {};

template <
	typename Sequence, typename State, typename ForwardOp,
	typename ForwardPredicate, typename BackwardOp,
	typename BackwardPredicate
> struct iter_fold_if<
	Sequence, State, ForwardOp, ForwardPredicate, BackwardOp,
	BackwardPredicate
> {
	typedef typename begin<Sequence>::type first_;
	typedef typename end<Sequence>::type last_;

	typedef BackwardPredicate backward_pred_;

	struct result_ : detail::iter_fold_if_impl<
		first_, State, ForwardOp,
		protect<
			detail::iter_fold_if_pred<ForwardPredicate, last_>
		>,
		BackwardOp, backward_pred_
	> {};

	typedef pair<typename result_::state, typename result_::iterator> type;
};


}}}

#endif

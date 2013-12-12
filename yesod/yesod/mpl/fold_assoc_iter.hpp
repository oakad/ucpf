/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2009 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_FOLD_ASSOC_ITER_DEC_11_2013_1800)
#define UCPF_YESOD_MPL_FOLD_ASSOC_ITER_DEC_11_2013_1800

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/detail/op_assoc.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <
	typename OpAssoc, typename OpBinary, typename State, typename First,
	typename Last
> struct fold_assoc_iter;

template <typename OpBinary, typename State, typename Last>
struct fold_assoc_iter<
	typename detail::op_assoc::left, OpBinary, State, Last, Last
> {
	typedef State type;
};

template <typename OpBinary, typename State, typename Last>
struct fold_assoc_iter<
	typename detail::op_assoc::right, OpBinary, State, Last, Last
> {
	typedef State type;    
};

template <typename OpBinary, typename State, typename First, typename Last>
struct fold_assoc_iter<
	typename detail::op_assoc::left, OpBinary, State, First, Last
> : fold_assoc_iter<
	typename detail::op_assoc::left, OpBinary,
	typename apply<OpBinary, State, First>::type,
	typename next<First>::type, Last
> {};

template <typename OpBinary, typename State, typename First, typename Last>
struct fold_assoc_iter<
	typename detail::op_assoc::right, OpBinary, State, First, Last
> : apply<
	OpBinary, First, typename fold_assoc_iter<
		typename detail::op_assoc::right, OpBinary, State,
		typename next<First>::type, Last
	>::type
> {};

}}}

#endif

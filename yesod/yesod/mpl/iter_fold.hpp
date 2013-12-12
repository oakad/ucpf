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

#if !defined(UCPF_YESOD_MPL_ITER_FOLD_DEC_11_2013_1820)
#define UCPF_YESOD_MPL_ITER_FOLD_DEC_11_2013_1820

#include <yesod/mpl/fold_assoc_iter.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename Sequence, typename State, typename ForwardOp>
struct iter_fold : fold_assoc_iter<
	typename detail::op_assoc::left, ForwardOp, State,
	typename begin<Sequence>::type, typename end<Sequence>::type
> {};

}}}

#endif

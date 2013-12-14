/*
 * Adaptations as part of present software bundle:
 *
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

#if !defined(UCPF_YESOD_MPL_FOLD_ASSOC_PACK_DEC_11_2013_1140)
#define UCPF_YESOD_MPL_FOLD_ASSOC_PACK_DEC_11_2013_1140

#include <yesod/mpl/detail/op_assoc.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <
	typename OpAssoc, template <typename, typename> class OpBinary,
	typename State, typename... Tn
> struct fold_assoc_pack;

template <typename OpAssoc, template <typename, typename> class OpBinary,
	  typename State
> struct fold_assoc_pack<OpAssoc, OpBinary, State> {
	typedef State type;
};

template <
	template <typename State, typename Value> class OpStateValue_State,
	typename State, typename T0, typename... Tn
> struct fold_assoc_pack<
	typename detail::op_assoc::left, OpStateValue_State, State, T0, Tn...
> : fold_assoc_pack<
	typename detail::op_assoc::left, OpStateValue_State,
	typename OpStateValue_State<
		State, T0
	>::type, Tn...
> {};

template <
	template <typename Value, typename State> class OpValueState_State,
	typename State, typename T0, typename... Tn
> struct fold_assoc_pack<
	typename detail::op_assoc::right, OpValueState_State, State, T0, Tn...
> : OpValueState_State<
	T0, typename fold_assoc_pack<
		typename detail::op_assoc::right, OpValueState_State, State,
		Tn...
	>::type
>{};

}}}

#endif

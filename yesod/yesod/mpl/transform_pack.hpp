/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2009 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_TRANSFORM_PACK_DEC_12_2013_1840)
#define UCPF_YESOD_MPL_TRANSFORM_PACK_DEC_12_2013_1840

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/package.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <
	typename Sequence, typename Op,
	typename Inserter = back_pack_inserter<package<>>
> struct transform1_pack;

template <
	template <typename...> class Sequence, typename... Tn,
	typename Op, typename Inserter
> struct transform1_pack<Sequence<Tn...>, Op, Inserter> {
	typedef package<typename apply<Op, Tn>::type...> elements_;
	typedef typename Inserter::template apply<elements_>::type type;
};

template <
	typename Sequence0, typename Sequence1, typename Op,
	typename Inserter = back_pack_inserter<package<>>
> struct transform2_pack;

template <
	template <typename...> class Sequence0, typename... Tn,
	template <typename...> class Sequence1, typename... Un,
	typename Op, typename Inserter
> struct transform2_pack<Sequence0<Tn...>, Sequence1<Un...>, Op, Inserter> {
	typedef package<typename apply<Op, Tn, Un>::type...> elements_;
	typedef typename Inserter::template apply<elements_>::type type;
};

}}}

#endif

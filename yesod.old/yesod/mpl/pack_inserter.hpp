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

#if !defined(UCPF_YESOD_MPL_PACK_INSERTER_DEC_12_2013_1840)
#define UCPF_YESOD_MPL_PACK_INSERTER_DEC_12_2013_1840

#include <yesod/mpl/package.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename Tpack>
struct back_pack_inserter;

template <template <typename...> class Tpack, typename... Tn>
struct back_pack_inserter<Tpack<Tn...>> {
	template <typename Upack>
	struct apply;

	template <template <typename...> class Upack, typename... Un>
	struct apply<Upack<Un...>> {
		typedef Tpack<Tn..., Un...> type;
	};
};

}}}

#endif

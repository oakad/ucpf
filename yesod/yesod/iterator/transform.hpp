/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2002 David Abrahams
    Copyright (c) 2002 Jeremy Siek
    Copyright (c) 2002 Thomas Witt

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_TRANSFORM_20140914T2300)
#define UCPF_YESOD_ITERATOR_TRANSFORM_20140914T2300

namespace ucpf { namespace yesod { namespace iterator {

/* Expected arguments in order:
 * 1. Transform function
 * 2. Base iterator type
 * 3. Reference type
 * 4. Value type
 */
template <typename... Tn>
struct transform {
	static_assert(
		sizeof...(Tn) > 1,
		"At least the transform function type and nested iterator "
		"type must be specified."
	);

	
};

}}}
#endif

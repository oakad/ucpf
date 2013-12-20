/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_VOID_DEC_11_2013_1310)
#define UCPF_YESOD_MPL_VOID_DEC_11_2013_1310

#include <yesod/mpl/integral.hpp>

namespace ucpf { namespace yesod { namespace mpl {

struct void_ {
	typedef void_ type;
};

template <typename T>
struct is_void_ : false_type {};

template <>
struct is_void_<void_> : true_type {};

template <typename T>
struct is_not_void_ : true_type {};

template <>
struct is_not_void_<void_> : false_type {};

}}}

#endif

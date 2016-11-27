/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_DETAIL_TYPE_WRAPPER_DEC_11_2013_1230)
#define UCPF_YESOD_MPL_DETAIL_TYPE_WRAPPER_DEC_11_2013_1230

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename T>
struct type_wrapper {
	typedef T type;
};

template <typename T>
struct wrapped_type;

template <typename T>
struct wrapped_type <type_wrapper<T>> {
	typedef T type;
};

}}}}

#endif

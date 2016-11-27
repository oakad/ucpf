/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_HAS_BEGIN_DEC_11_2013_1425)
#define UCPF_YESOD_MPL_DETAIL_HAS_BEGIN_DEC_11_2013_1425

#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/detail/type_wrapper.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename T>
struct has_begin {
	template <typename U>
	static true_type test(
		type_wrapper<U> const volatile *,
		type_wrapper<typename U::begin> * = 0
	);

	static false_type test(...);

	typedef decltype(test(static_cast<type_wrapper<T> *>(nullptr))) type;

	static const bool value = type::value;
};

}}}}

#endif

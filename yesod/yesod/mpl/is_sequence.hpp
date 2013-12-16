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

#if !defined(UCPF_YESOD_MPL_IS_SEQUENCE_DEC_16_2013_1650)
#define UCPF_YESOD_MPL_IS_SEQUENCE_DEC_16_2013_1650

#include <yesod/mpl/logical.hpp>
#include <yesod/mpl/begin_end.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct is_sequence;

template <>
struct is_sequence<> {
	template <typename T0, typename... Tn>
	struct apply : is_sequence<T0> {};
};

template <typename Tag>
struct lambda<is_sequence<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef is_sequence<> result_;
	typedef is_sequence<> type;
};

template <typename T>
struct is_sequence<T> : not_<std::is_same<typename begin<T>::type, void_>> {};

}}}

#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_LOGICAL_DEC_12_2013_1210)
#define UCPF_YESOD_MPL_LOGICAL_DEC_12_2013_1210

#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/fold_null_unit.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T>
struct nested_type_wknd : T::type {};

template <typename T>
struct bool_cast : bool_<nested_type_wknd<T>::value> {};

}

template <typename... Tn>
struct or_ : fold_null_unit<
	true_type, false_type, detail::bool_cast, Tn...
> {};

template <typename Tag>
struct lambda<or_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef or_<> result_;
	typedef or_<> type;
};


template <typename... Tn>
struct and_ : fold_null_unit<
	false_type, true_type, detail::bool_cast, Tn...
> {};

template <typename Tag>
struct lambda<and_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef and_<> result_;
	typedef and_<> type;
};


template <typename...>
struct not_;

template <>
struct not_<> {
	template <typename T0, typename... Tn>
	struct apply : not_<T0> {};
};

template <typename Tag>
struct lambda<not_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef not_<> result_;
	typedef not_<> type;
};

template <typename T>
struct not_<T> : bool_<!detail::nested_type_wknd<T>::value> {};

}}}

#endif

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

#if !defined(UCPF_YESOD_MPL_ITERATOR_CATEGORY_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_ITERATOR_CATEGORY_DEC_17_2013_1730

#include <yesod/mpl/lambda_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct iterator_category;

template <>
struct iterator_category<> {
	template <typename T0, typename... Tn>
	struct apply : iterator_category<T0> {};
};

template <typename Tag>
struct lambda<iterator_category<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef iterator_category<> result_;
	typedef iterator_category<> type;
};

template <typename Iterator>
struct iterator_category<Iterator> {
	typedef typename Iterator::category type;
};

}}}

#endif

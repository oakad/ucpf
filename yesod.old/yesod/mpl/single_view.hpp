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

#if !defined(UCPF_YESOD_MPL_SINGLE_VIEW_DEC_16_2013_1500)
#define UCPF_YESOD_MPL_SINGLE_VIEW_ITER_DEC_16_2013_1500

#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/distance.hpp>
#include <yesod/mpl/iterator_tags.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T, bool IsLast>
struct single_element_iter;

template <typename T>
struct single_element_iter<T, false> {
	typedef random_access_iterator_tag category;
	typedef single_element_iter<T, true> next;
	typedef T type;
};

template<typename T>
struct single_element_iter<T, true> {
	typedef random_access_iterator_tag category;
	typedef single_element_iter<T, false> prior;
};

}

template <typename T, bool IsLast, typename Distance>
struct advance<detail::single_element_iter<T, IsLast>, Distance> {
	typedef detail::single_element_iter<
		T, (Distance::value ? (Distance::value > 0) : IsLast)
	> type;
};

template <typename T, bool IsLast0, bool IsLast1>
struct distance<
	detail::single_element_iter<T, IsLast0>,
	detail::single_element_iter<T, IsLast1>
> : long_<(IsLast1 == IsLast0 ? 0l : (IsLast1 ? 1l : -1l))> {};

template <typename...>
struct single_view;

template <>
struct single_view<> {
	template <typename T0, typename... Tn>
	struct apply : single_view<T0> {};
};

template <typename Tag>
struct lambda<single_view<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef single_view<> result_;
	typedef single_view<> type;
};

template <typename T>
struct single_view<T> : iterator_range<
	detail::single_element_iter<T, false>,
	detail::single_element_iter<T, true>
> {};

}}}

#endif

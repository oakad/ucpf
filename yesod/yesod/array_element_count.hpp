/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_ARRAY_ELEMENT_COUNT_JAN_10_2014_1600)
#define UCPF_YESOD_ARRAY_ELEMENT_COUNT_JAN_10_2014_1600

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/range_c.hpp>

namespace ucpf { namespace yesod {
namespace detail {

template <typename T, typename Acc, typename Pos>
struct array_element_count_impl {
	typedef mpl::ulong_<
		Acc::value + std::extent<T, Pos::value>::value
	> type;
};

}

template <typename T>
using array_element_count = typename std::conditional<
	std::is_array<T>::value,
	typename mpl::fold<
		mpl::range_c<unsigned long, 0, std::rank<T>::value>,
		mpl::ulong_<0>,
		detail::array_element_count_impl<T, mpl::arg<0>, mpl::arg<1>>
	>::type, mpl::ulong_<1>
>::type;

}}

#endif

/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/bound.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(bound_0)
{
	typedef list_c<int, 1, 2, 3, 3, 3, 5, 8> numbers;
	typedef lower_bound<numbers, int_<4>>::type iter;

	BOOST_CHECK_EQUAL((distance<begin<numbers>::type, iter>::value), 5);
	BOOST_CHECK_EQUAL((deref<iter>::type::value), 5);
}

BOOST_AUTO_TEST_CASE(bound_1)
{
	typedef list_c<int, 1, 2, 3, 3, 3, 5, 8> numbers;
	typedef upper_bound<numbers, int_<3>>::type iter;

	BOOST_CHECK_EQUAL((distance<begin<numbers>::type, iter>::value), 5);
	BOOST_CHECK_EQUAL((deref<iter>::type::value), 5);
}

}}}

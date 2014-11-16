/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/front_back.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(front_back_0)
{
	BOOST_CHECK_EQUAL((front<range_c<int, 1, 10>>::type::value), 1);
	BOOST_CHECK_EQUAL((front<range_c<int, 2, 10>>::type::value), 2);
	BOOST_CHECK_EQUAL((front<range_c<int, -1, 10>>::type::value), -1);
}

BOOST_AUTO_TEST_CASE(front_back_1)
{
	BOOST_CHECK_EQUAL((back<range_c<int, 0, 1>>::type::value), 0);
	BOOST_CHECK_EQUAL((back<range_c<int, 0, 10>>::type::value), 9);
	BOOST_CHECK_EQUAL((back<range_c<int, -10, 0>>::type::value), -1);
}

}}}

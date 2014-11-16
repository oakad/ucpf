/*==============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/same_as.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(same_as_0)
{
	BOOST_CHECK((apply<same_as<int>, int>::type::value));
	BOOST_CHECK(!(apply<same_as<int>, long>::type::value));
}

}}}

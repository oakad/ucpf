/*==============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/sizeof.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct my {
	char a[100];
};

}

BOOST_AUTO_TEST_CASE(sizeof_0)
{
	BOOST_CHECK_EQUAL((sizeof_<char>::value), sizeof(char));
	BOOST_CHECK_EQUAL((sizeof_<int>::value), sizeof(int));
	BOOST_CHECK_EQUAL((sizeof_<double>::value), sizeof(double));
	BOOST_CHECK_EQUAL((sizeof_<test::my>::value), sizeof(test::my));
}

}}}

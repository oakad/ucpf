/*==============================================================================
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/make_value.hpp>

#include <string>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(make_value_0)
{
	typedef vector_c<char, 'a', 'b', 'c', 'd', 'e'> v1;
	std::string str1;

	make_value<v1>(str1);

	BOOST_CHECK_EQUAL(str1, std::string("abcde"));
}

}}}

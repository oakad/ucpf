/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/contains.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(contains_0)
{
	typedef vector<
		int, char, long, short, char, long, double, long
	>::type types;

	BOOST_CHECK((contains<types, short>::value));
	BOOST_CHECK(!(contains<types, unsigned>::value));

}

}}}

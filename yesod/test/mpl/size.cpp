/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/size.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(size_0)
{
	BOOST_CHECK((empty<list<>>::value));
	BOOST_CHECK(!(empty<list<char>>::value));
}

BOOST_AUTO_TEST_CASE(size_1)
{
	typedef list<> empty_list;
	typedef list<char, short, int, long> list;

	BOOST_CHECK_EQUAL((size<empty_list>::value), 0);
	BOOST_CHECK_EQUAL((size<list>::value), 4);
	BOOST_CHECK((equal_to<size<list>, int_<4>>::value));
}

}}}

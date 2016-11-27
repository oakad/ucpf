/*=============================================================================
    Copyright (c) 2002-2003 Eric Friedman
    Copyright (c) 2004      Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/sort.hpp>
#include <yesod/mpl/equal.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(sort_0)
{
	typedef list_c<int, 3, 4, 0, -5, 8, -1, 7>::type numbers;
	typedef list_c<int, -5, -1, 0, 3, 4, 7, 8>::type manual_result;

	typedef sort<numbers>::type result;

	BOOST_CHECK((equal<                                     \
		result, manual_result, equal_to<arg<0>, arg<1>> \
	>::value));
}

BOOST_AUTO_TEST_CASE(sort_1)
{
	typedef list_c<int, 3, 4, 0, -5, 8, -1, 7>::type numbers;
	typedef list_c<int, 8, 7, 4, 3, 0, -1, -5>::type manual_result;

	typedef reverse_sort<numbers>::type result;

	BOOST_CHECK((equal<                                     \
		result, manual_result, equal_to<arg<0>, arg<1>> \
	>::value));
}

}}}

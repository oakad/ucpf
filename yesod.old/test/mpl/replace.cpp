/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2000-2002 John R. Bandela
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/replace.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(replace_0)
{
	typedef list<int, float, char, float, float, double> types;
	typedef replace<types, float, double>::type result;
	typedef list<int, double, char, double, double, double> answer;
	BOOST_CHECK((equal<result, answer>::value));
}

BOOST_AUTO_TEST_CASE(replace_1)
{
	typedef list_c<int, 1, 4, 5, 2, 7, 5, 3, 5>::type numbers;
	typedef replace_if<
		numbers, greater<arg<0>, int_<4>>, int_<0>
	>::type result;

	typedef list_c<int, 1, 4, 0, 2, 0, 0, 3, 0>::type answer;
	BOOST_CHECK((equal<answer, result, equal_to<arg<0>, arg<1>>>::value));
}

}}}

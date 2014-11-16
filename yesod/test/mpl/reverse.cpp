/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/reverse.hpp>
#include <yesod/mpl/range_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(reverse_0)
{
	typedef list_c<int, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0> numbers;
	typedef reverse<numbers>::type result;

	typedef range_c<int, 0, 10> answer;

	BOOST_CHECK((equal<result, answer, equal_to<arg<0>, arg<1>>>::value));
}

}}}

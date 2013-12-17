/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(fold_0)
{
	typedef list<
		long, float, short, double, float, long, long double
	> types;
	typedef fold<types, int_<0>, if_<
			std::is_floating_point<arg<1>>, next<arg<0>>, arg<0>
	>>::type number_of_floats;

	BOOST_CHECK_EQUAL((number_of_floats::value), 4);
}

BOOST_AUTO_TEST_CASE(fold_1)
{
	typedef list_c<int, 5, -1, 0, -7, -2, 0, -5, 4> numbers;
	typedef list_c<int, -1, -7, -2, -5> negatives;
	typedef reverse_fold<
		numbers, list_c<int>, if_<
			less<arg<1>, int_<0>>, push_front<arg<0>, arg<1>>,
			arg<0>
	>>::type result;

	BOOST_CHECK((equal<                                 \
		result, negatives, equal_to<arg<0>, arg<1>> \
	>::value));
}

}}}

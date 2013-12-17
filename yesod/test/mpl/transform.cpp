/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/transform.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(transform_0)
{
	typedef list<char, short, int, long, float, double> types;
	typedef list<
		char *, short *, int *, long *, float *, double *
	> pointers;

	typedef transform1<types, std::add_pointer<arg<0>>>::type result;
	BOOST_CHECK((equal<result, pointers>::value));
}

BOOST_AUTO_TEST_CASE(transform_1)
{
	typedef list_c<long, 0, 2, 4, 6, 8, 10> evens;
	typedef list_c<long, 2, 3, 5, 7, 11, 13> primes;
	typedef list_c<long, 2, 5, 9, 13, 19, 23> sums;

	typedef transform2<evens, primes, plus<>>::type result;
	BOOST_CHECK((equal<result, sums, equal_to<arg<0>, arg<1>>>::value));

	typedef transform<evens, primes, plus<>>::type result2;
	BOOST_CHECK((std::is_same<result2, result>::value));
}

}}}

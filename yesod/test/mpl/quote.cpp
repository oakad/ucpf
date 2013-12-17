/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/quote.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename T>
struct f1 {
	typedef T type;
};

template <
	typename T1, typename T2, typename T3, typename T4, typename T5
>
struct f5 {};

}

BOOST_AUTO_TEST_CASE(quote_0)
{
	typedef quote<test::f1>::apply<int>::type t1;
	typedef quote<test::f5>::apply<char, short, int, long, float>::type t5;

	BOOST_CHECK((std::is_same<t1, int>::value));
	BOOST_CHECK((                                                     \
		std::is_same<t5, test::f5<char, short, int, long, float>> \
		::value                                                   \
	));
}

}}}

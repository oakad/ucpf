/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/vector.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename Seq, int n>
struct at_test {
	typedef typename at_c<Seq, n>::type t;

	at_test()
	{
		BOOST_CHECK((std::is_same<               \
			t, integral_constant<int, 9 - n> \
		>::value));
		BOOST_CHECK_EQUAL((t::value), 9 - n);
	}
};

}

BOOST_AUTO_TEST_CASE(at_0)
{
	typedef vector_c<int, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0> numbers;

	test::at_test<numbers, 0>();
	test::at_test<numbers, 1>();
	test::at_test<numbers, 2>();
	test::at_test<numbers, 3>();
	test::at_test<numbers, 4>();
	test::at_test<numbers, 5>();
	test::at_test<numbers, 6>();
	test::at_test<numbers, 7>();
	test::at_test<numbers, 8>();
	test::at_test<numbers, 9>();
}

}}}

/*=============================================================================
    Copyright (c) 2003      Eric Friedman
    Copyright (c) 2004      Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/partition.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

typedef vector_c<int, 3, 4, 0, -5, 8, -1, 7>::type numbers;
typedef vector_c<int, 0, -5, -1>::type manual_first;
typedef vector_c<int, 3, 4, 8, 7>::type manual_second;

template <typename N>
struct is_odd : modulus<N, int_<2>> {};

}

BOOST_AUTO_TEST_CASE(partition_0)
{
	typedef stable_partition<
		test::numbers, less<arg<-1>, int_<3>>
	>::type result;

	BOOST_CHECK((equal<result::first, test::manual_first>::value));
	BOOST_CHECK((equal<result::second, test::manual_second>::value));
}

BOOST_AUTO_TEST_CASE(partition_1)
{
	typedef stable_partition<
		test::numbers, greater_equal<arg<-1>, int_<3>>
	>::type result;

	BOOST_CHECK((equal<result::first, test::manual_second>::value));
	BOOST_CHECK((equal<result::second, test::manual_first>::value));
}

BOOST_AUTO_TEST_CASE(partition_2)
{
	typedef partition<
		range_c<int,0,10>, test::is_odd<arg<0>>,
		back_inserter<vector<>>, back_inserter<vector<>>
	>::type r;

	BOOST_CHECK((equal<r::first, vector_c<int, 1, 3, 5, 7, 9>>::value));
	BOOST_CHECK((equal<r::second, vector_c<int, 0, 2, 4, 6, 8>>::value));
}

}}}

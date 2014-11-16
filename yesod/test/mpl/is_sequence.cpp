/*=============================================================================
    Copyright (c) 2002-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/is_sequence.hpp>

#include "data_types.hpp"

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename T>
struct std_vector {
	T* begin();
};

}

BOOST_AUTO_TEST_CASE(is_sequence_0)
{
	BOOST_CHECK(!(is_sequence<test::std_vector<int>>::value));
	BOOST_CHECK(!(is_sequence<int_<0> >::value));
	BOOST_CHECK(!(is_sequence<int>::value));
	BOOST_CHECK(!(is_sequence<int &>::value));
	BOOST_CHECK(!(is_sequence<test::UDT>::value));
	BOOST_CHECK(!(is_sequence<test::UDT *>::value));
	BOOST_CHECK((is_sequence<range_c<int, 0, 0>>::value));
	BOOST_CHECK((is_sequence<list<>>::value));
	BOOST_CHECK((is_sequence<list<int>>::value));
	BOOST_CHECK((is_sequence<vector<>>::value));
	BOOST_CHECK((is_sequence<vector<int>>::value));
}

}}}

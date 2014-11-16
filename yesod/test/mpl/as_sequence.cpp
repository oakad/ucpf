/*=============================================================================
    Copyright (c) 2002-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/as_sequence.hpp>

#include "data_types.hpp"

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(as_sequence_0)
{
	BOOST_CHECK((is_sequence<as_sequence<int>::type>::value));
	BOOST_CHECK((is_sequence<as_sequence<test::UDT>::type>::value));
	BOOST_CHECK((is_sequence<as_sequence<vector<>>::type>::value));
}

}}}

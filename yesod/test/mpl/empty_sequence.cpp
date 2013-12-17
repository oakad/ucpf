/*=============================================================================
    Copyright (c) 2004      Aleksey Gurtovoy
    Copyright (c) 2004      Alexander Nasonov
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/size.hpp>
#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/empty_sequence.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(empty_sequence_0)
{
	typedef begin<empty_sequence>::type begin;
	typedef end<empty_sequence>::type end;

	BOOST_CHECK((std::is_same<begin, end>::value));
	BOOST_CHECK_EQUAL((distance<begin, end>::value), 0);
	BOOST_CHECK_EQUAL((size<empty_sequence>::value), 0);

	typedef advance_c<begin, 0>::type advanced;
	BOOST_CHECK((std::is_same<advanced, end>::value));
}

}}}

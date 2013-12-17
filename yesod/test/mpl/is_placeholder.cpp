/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>
#include <boost/preprocessor/repeat.hpp>

#include <yesod/mpl/arg.hpp>

#include "data_types.hpp"

namespace ucpf { namespace yesod { namespace mpl {

#define IS_PLACEHOLDER_TEST(unused1, n, unused2) \
{ BOOST_CHECK((is_placeholder<arg<n>>::value )); }

BOOST_AUTO_TEST_CASE(is_placeholder_0)
{
	BOOST_CHECK(!(is_placeholder<int>::value));
	BOOST_CHECK(!(is_placeholder<test::UDT>::value));
	BOOST_CHECK(!(is_placeholder<test::incomplete>::value));
	BOOST_CHECK(!(is_placeholder<test::abstract>::value));
	BOOST_CHECK(!(is_placeholder<test::noncopyable>::value));
	BOOST_CHECK((is_placeholder<arg<-1>>::value));
	BOOST_PP_REPEAT(10, IS_PLACEHOLDER_TEST, unused);
}

}}}

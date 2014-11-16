/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/always.hpp>
#include <yesod/mpl/lambda.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(always_0)
{
	typedef always<std::true_type> always_true;

	BOOST_CHECK((apply<always_true>::type::value));
	BOOST_CHECK((apply<always_true>::type::value));
	BOOST_CHECK((apply<always_true, std::false_type>::type::value));
	BOOST_CHECK((                                                 \
		apply<                                                \
			always_true, std::false_type, std::false_type \
		>::type::value                                        \
	));
	BOOST_CHECK((                                                  \
		apply<                                                 \
			always_true, std::false_type, std::false_type, \
			std::false_type                                \
		>::type::value                                         \
	));
}

BOOST_AUTO_TEST_CASE(always_1)
{
	typedef always<int_<10>> always_10;

	typedef apply<always_10>::type res;
	typedef apply<always_10>::type res0;
	typedef apply<always_10, int_<0>>::type res1;
	typedef apply<always_10, int_<0>, int_<0>>::type res2;
	typedef apply<always_10, int_<0>, int_<0>, int_<0>>::type res3;

	BOOST_CHECK_EQUAL((res::value), 10 );
	BOOST_CHECK_EQUAL((res0::value), 10 );
	BOOST_CHECK_EQUAL((res1::value), 10 );
	BOOST_CHECK_EQUAL((res2::value), 10 );
	BOOST_CHECK_EQUAL((res3::value), 10 );
}

}}}

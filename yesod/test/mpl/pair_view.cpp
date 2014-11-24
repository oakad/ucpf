/*==============================================================================
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2004      Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/pair_view.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(pair_view_0)
{
	typedef range_c<int, 0, 10> r;
	typedef vector_c<int, 9, 8, 7, 6, 5, 4, 3, 2, 1, 10> v;

	typedef pair_view<r, v> view;
	typedef begin<view>::type first_;
	typedef end<view>::type last_;

	BOOST_CHECK((std::is_same<                           \
		first_::category, random_access_iterator_tag \
	>::value));

	BOOST_CHECK((std::is_same<advance_c<first_, 0>::type, first_>::value));
	BOOST_CHECK((std::is_same<advance_c<last_, 0>::type, last_>::value));
	BOOST_CHECK((std::is_same<advance_c<first_, 10>::type, last_>::value));
	BOOST_CHECK((std::is_same<advance_c<last_, -10>::type, first_>::value));

	typedef advance_c<first_, 5>::type iter;

	BOOST_CHECK((std::is_same<                        \
		deref<iter>::type, pair<int_<5>, int_<4>> \
	>::value));
}

}}}

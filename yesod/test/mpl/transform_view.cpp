/*=============================================================================
    Copyright (c) 2001-2010 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/sizeof.hpp>
#include <yesod/mpl/is_even.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/zip_view.hpp>
#include <yesod/mpl/unpack_args.hpp>
#include <yesod/mpl/filter_view.hpp>
#include <yesod/mpl/transform_view.hpp>
#include <yesod/mpl/min_max_element.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(transform_view_0)
{
	typedef list<int, long, char, char[50], double> types;
	typedef max_element<
		transform_view<types, sizeof_<arg<-1>>>
	>::type iter;

	BOOST_CHECK_EQUAL((deref<iter>::type::value), 50);
}

BOOST_AUTO_TEST_CASE(transform_view_1)
{
	typedef list<
		int, float, long, float, char[50], long double, char
	> types;

	typedef max_element<transform_view<
		filter_view<types, std::is_floating_point<arg<-1>>>,
		sizeof_<arg<-1>>
	>>::type iter;

	BOOST_CHECK((                                                     \
		std::is_same<deref<iter::base>::type, long double>::value \
	));
}

BOOST_AUTO_TEST_CASE(transform_view_2)
{
	typedef transform_view<
		zip_view<
			vector<range_c<int, 0, 10>, range_c<int, 10, 20>>
		>, unpack_args<plus<>>
	> result;

	BOOST_CHECK((equal<result, filter_view<                \
			range_c<int, 10, 30>, is_even<arg<-1>> \
	>, equal_to<arg<-1>, arg<-1>>>::value));

	BOOST_CHECK((std::is_same<                           \
		zip_view<vector<>>, zip_view<vector<>>::type \
	>::value));
}

}}}

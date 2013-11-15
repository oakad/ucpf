/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/float.hpp>

#include <typeinfo>

namespace ucpf { namespace yesod {
namespace test {

struct s0 {};

}

BOOST_AUTO_TEST_CASE(float_0)
{
	BOOST_CHECK(is_floating_point<float>::value);
	BOOST_CHECK(is_floating_point<double>::value);
	BOOST_CHECK(is_floating_point<float_t<32>>::value);
	BOOST_CHECK(is_floating_point<float_t<64>>::value);
	BOOST_CHECK(!is_floating_point<test::s0>::value);
	BOOST_CHECK((std::is_same<                                 \
		typename fp_adapter_type<float>::type, float_t<32> \
	>::value));
	BOOST_CHECK((std::is_same<                                  \
		typename fp_adapter_type<double>::type, float_t<64> \
	>::value));
	BOOST_CHECK((std::is_same<                       \
		typename fp_adapter_type<int>::type, int \
	>::value));
}

}}

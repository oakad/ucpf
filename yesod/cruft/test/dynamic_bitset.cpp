/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/dynamic_bitset.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(dynamic_bitset_0)
{
	dynamic_bitset<> b;

	b.set(10);
	b.set(20);
	b.set(30);
	BOOST_CHECK(!b.test(15) && !b.test(25) && !b.test(35) && !b.test(105));
	b.set(105);
	BOOST_CHECK(b.test(10) && b.test(20) && b.test(30) && b.test(105));
}

BOOST_AUTO_TEST_CASE(dynamic_bitset_1)
{
	dynamic_bitset<> b;

	b.set(10);
	b.set(20);
	b.set(30);
	BOOST_CHECK_EQUAL(b.find_below<true>(40), 30);
	BOOST_CHECK_EQUAL(b.find_below<true>(30), 20);
	BOOST_CHECK_EQUAL(b.find_below<true>(20), 10);
	BOOST_CHECK_EQUAL(b.find_below<true>(10), decltype(b)::npos);
	BOOST_CHECK_EQUAL(b.find_above<true>(10), 20);
	b.set(105);
	BOOST_CHECK_EQUAL(b.find_above<true>(30), 105);
	b.reset(30);
	BOOST_CHECK_EQUAL(b.find_below<true>(105), 20);
}

BOOST_AUTO_TEST_CASE(dynamic_bitset_2)
{
	dynamic_bitset<> b;

	b.set(10);
	b.set(20);
	b.set(30);
	BOOST_CHECK_EQUAL(b.count<true>(), 3);
	b.set(105);
	BOOST_CHECK_EQUAL(b.count<true>(), 4);
	BOOST_CHECK_EQUAL(b.count<true>(12, 21), 1);
}

BOOST_AUTO_TEST_CASE(dynamic_bitset_3)
{
	dynamic_bitset<> b;

	b.set();
	b.reset(10);
	b.reset(20);
	b.reset(30);
	BOOST_CHECK_EQUAL(b.find_below<false>(40), 30);
	BOOST_CHECK_EQUAL(b.find_below<false>(30), 20);
	BOOST_CHECK_EQUAL(b.find_below<false>(20), 10);
	BOOST_CHECK_EQUAL(b.find_below<false>(10), decltype(b)::npos);
	BOOST_CHECK_EQUAL(b.find_above<false>(10), 20);
	b.reset(105);
	BOOST_CHECK_EQUAL(b.find_above<false>(30), 105);
	b.set(30);
	BOOST_CHECK_EQUAL(b.find_below<false>(105), 20);
}

}}

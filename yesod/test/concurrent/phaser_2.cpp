/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
/*=============================================================================
	Based on original implementation of
	[openjdk]test.java.util.concurrent.Phaser:

	This file is available under and governed by the GNU General Public
	License version 2 only, as published by the Free Software Foundation.
	However, the following notice accompanied the original version of this
	file:

	Written by Doug Lea with assistance from members of JCP JSR-166
	Expert Group and released to the public domain, as explained at
	http://creativecommons.org/publicdomain/zero/1.0/
==============================================================================*/

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/concurrent/phaser.hpp>
#include "../test.hpp"

namespace ucpf::yesod::concurrent {
namespace test {

void check_state(
	phaser &p, phaser::phase_value_type phase,
	phaser::count_type parties, phaser::count_type unarrived
)
{
	BOOST_TEST(phase == p.get_phase().value());
	BOOST_TEST(parties == p.get_registered_parties());
	BOOST_TEST(unarrived == p.get_unarrived_parties());
}

}

BOOST_AUTO_TEST_SUITE(phaser_2)

BOOST_AUTO_TEST_CASE(t0)
{
	phaser p;

	test::phaser_probe::set_state(p, 0x7ffffffe, 0, 1);
	test::check_state(p, 0x7ffffffe, 0, 0);
	p.register_one();
	test::check_state(p, 0x7ffffffe, 1, 1);
	p.arrive();
	test::check_state(p, 0x7fffffff, 1, 1);
	p.arrive();
	test::check_state(p, 0, 1, 1);
	p.arrive();
	test::check_state(p, 1, 1, 1);
}

BOOST_AUTO_TEST_CASE(t1)
{
	phaser root;
	phaser::phase_type ph(0x7ffffffe);

	test::phaser_probe::set_state(root, ph.value(), 0, 1);
	test::check_state(root, ph.value(), 0, 0);

	phaser p0(1, &root);
	test::check_state(root, ph.value(), 1, 1);
	test::check_state(p0, ph.value(), 1, 1);

	phaser p1(2, &root);
	test::check_state(root, ph.value(), 2, 2);
	test::check_state(p1, ph.value(), 2, 2);

	for (size_t c(0); c < 5; ++c) {
		test::check_state(root, ph.value(), 2, 2);
		test::check_state(p0, ph.value(), 1, 1);
		test::check_state(p1, ph.value(), 2, 2);
		p0.arrive();
		test::check_state(root, ph.value(), 2, 1);
		test::check_state(p0, ph.value(), 1, 0);
		test::check_state(p1, ph.value(), 2, 2);
		p1.arrive();
		test::check_state(root, ph.value(), 2, 1);
		test::check_state(p0, ph.value(), 1, 0);
		test::check_state(p1, ph.value(), 2, 1);
		p1.arrive();
		++ph;
		test::check_state(root, ph.value(), 2, 2);
		test::check_state(p0, ph.value(), 1, 1);
		test::check_state(p1, ph.value(), 2, 2);
	}
	BOOST_TEST(ph.value() == 3);
}

BOOST_AUTO_TEST_SUITE_END()
}

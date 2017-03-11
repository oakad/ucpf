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
#include <boost/test/data/test_case.hpp>

#include <yesod/concurrent/phaser.hpp>
#include "../test.hpp"

namespace ucpf::yesod::concurrent {

BOOST_AUTO_TEST_SUITE(phaser_1)

BOOST_DATA_TEST_CASE(t0, boost::unit_test::data::xrange(10))
{
	auto n(1 + (this_thread::get_thread_data()->next_seed() % 10));
	
	struct state_t {
		state_t(uint32_t n)
		: starting_gate(n), p(n), count0(0), count1(0)
		{}

		phaser starting_gate;
		phaser p;
		std::atomic<uint32_t> count0 = {0};
		std::atomic<uint32_t> count1 = {0};
	} state(n);

	struct arr {
		arr(arr const &) = delete;

		arr(arr &&other)
		: s(other.s), t(std::move(other.t)),
		  boost_test_assertions(std::move(other.boost_test_assertions))
		{}

		arr(state_t &s_)
		: s(s_)
		{}

		void operator()()
		{
			BOOST_TEST_LOC(s.starting_gate.get_phase().value() == 0);

			s.starting_gate.arrive_and_await_advance();

			BOOST_TEST_LOC(s.starting_gate.get_phase().value() == 1);
			auto phase(s.p.arrive());

			BOOST_TEST(phase.value() < 2);
			if (phase.value() == 0)
				++s.count0;
			else if (phase.value() == 1)
				++s.count1;
		}

		void start()
		{
			t = std::move(thread(std::ref(*this)));
		}

		void report()
		{
			BOOST_TEST_REPORT_LOCALS();
		}

		state_t &s;
		thread t;
		BOOST_TEST_DECLARE_LOC_STORE();
	};

	std::vector<arr> a_list;

	for (uint32_t c(0); c < n; ++c)
		a_list.emplace_back(state);

	for (auto &a: a_list)
		a.start();

	for (auto &a: a_list) {
		a.t.join();
		a.report();
	}

	BOOST_TEST(state.count0.load() == n);
	BOOST_TEST(state.count1.load() == 0);
	BOOST_TEST(state.p.get_phase().value() == 1);
}

BOOST_AUTO_TEST_SUITE_END()
}

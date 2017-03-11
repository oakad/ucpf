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

using namespace std::chrono_literals;

namespace test {

struct state {
	state()
	: test_duration(test_duration_default),
	  start_time(std::chrono::steady_clock::now())
	{}

	static constexpr std::chrono::milliseconds test_duration_default = 10ms;//10s;
	std::chrono::milliseconds test_duration;
	std::chrono::time_point<std::chrono::steady_clock> start_time;
};

struct runner {
	runner() noexcept = default;
	runner(runner &) = delete;
	runner(runner const &) = delete;
	runner(runner const &&) = delete;

	runner(runner &&other)
	: s(other.s), p(other.p), t(std::move(other.t)),
	  boost_test_assertions(std::move(boost_test_assertions))
	{}

	runner(state &s_, phaser &p_)
	: s(s_), p(p_)
	{}

	void operator()()
	{
		auto prev_phase(p.register_one());
		while (!p.is_terminated()) {
			auto phase(p.await_advance(p.arrive()));
			if (phase.terminal())
				return;

			BOOST_TEST_LOC(
				phase.value() == (prev_phase + 1).value()
			);
			auto ph(p.get_phase());
			BOOST_TEST_LOC((
				ph.terminal() || (ph.value() == phase.value())
			));
			prev_phase = phase;
		}
	}

	void start()
	{
		t = std::move(thread(std::ref(*this)));
	}

	void report()
	{
		BOOST_TEST_REPORT_LOCALS();
	}

	state &s;
	phaser &p;
	thread t;
	BOOST_TEST_DECLARE_LOC_STORE();
};

}

BOOST_AUTO_TEST_SUITE(phaser_3)

BOOST_AUTO_TEST_CASE(t0)
{
	phaser parent;
	phaser child0(0, &parent);
	phaser child1(0, &parent);
	test::state s;

	test::runner r0(s, child0);
	test::runner r1(s, child1);

	r0.start();
	r1.start();

	phaser::phase_type prev_phase, phase;
	while (true) {
		phase = child1.get_phase();
		BOOST_TEST(phase.value() >= prev_phase.value());
		auto elapsed(
			std::chrono::steady_clock::now() - s.start_time
		);
		if (elapsed > s.test_duration) {
			child0.force_termination();
			break;
		}
		prev_phase = phase;
	}

	r0.t.join();
	r0.report();
	r1.t.join();
	r1.report();
}

BOOST_AUTO_TEST_SUITE_END()
}

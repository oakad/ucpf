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

using ucpf::yesod::test::check_all;

struct state {
	state()
	: test_duration(test_duration_default),
	  start_time(std::chrono::steady_clock::now())
	{}

	static constexpr std::chrono::milliseconds test_duration_default = 10ms;//10s;
	std::chrono::milliseconds test_duration;
	std::chrono::time_point<std::chrono::steady_clock> start_time;
	size_t chunk_size = 10;//00;
	std::atomic<size_t> count = {0};
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
		phaser::phase_type prev_phase(0x7fffffff);
		for (size_t m(1); true; ++m) {
			for (size_t n(0); n < s.chunk_size; ++n) {
				auto phase(p.register_one());
				if (phase.terminal())
					break;
				BOOST_TEST_LOC(
					phase.value()
					== (prev_phase + 1).value()
				);

				prev_phase = phase;
				BOOST_TEST_LOC(
					phase.value()
					== p.arrive_and_deregister().value()
				);

				BOOST_TEST_LOC(
					phase.value()
					< p.await_advance(phase).value()
				);
			}

			auto elapsed(
				std::chrono::steady_clock::now() - s.start_time
			);
			if (elapsed > s.test_duration) {
				s.count.fetch_add(m * s.chunk_size);
				break;
			}
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

BOOST_AUTO_TEST_SUITE(phaser_4)

BOOST_AUTO_TEST_CASE(t0)
{
	struct : phaser::notification {
		bool on_advance(
			phaser &p, phaser::phase_type phase,
			phaser::count_type registered_parties
		) override
		{
			return false;
		}
	} no_advance;
	test::state s;

	phaser parent(0, nullptr, &no_advance);
	phaser child0(0, &parent);
	phaser child1(0, &parent);
	phaser subchild0(0, &child0);
	phaser subchild1(0, &child1);

	phaser *phasers[] = {
		&parent, &child0, &child1, &subchild0, &subchild1
	};

	std::vector<test::runner> runners;
	for (size_t c(0); c < 4; ++c) {
		runners.emplace_back(s, subchild0);
		runners.emplace_back(s, child0);
		runners.emplace_back(s, parent);
		runners.emplace_back(s, child1);
		runners.emplace_back(s, subchild1);
	}

	for (auto &r: runners)
		r.start();

	for (auto &r:runners) {
		r.t.join();
		r.report();
	}

	for (auto p: phasers) {
		BOOST_TEST(p->get_phase().value() > 0);
		BOOST_TEST(p->get_registered_parties() == 0);
		BOOST_TEST(p->get_unarrived_parties() == 0);
		BOOST_TEST(
			parent.get_phase().value() == p->get_phase().value()
		);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}

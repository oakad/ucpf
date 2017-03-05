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

#define ATTEMPT_COUNT 10

namespace ucpf::yesod::concurrent {

using namespace std::chrono_literals;

namespace test {

std::atomic<int> cycle_arrive_await_advance(1);
phaser starting_gate(3);

void to_the_starting_gate()
{
	auto expect_next_phase(starting_gate.get_unarrived_parties() == 1);
	auto phase(starting_gate.get_phase());

	BOOST_TEST(phase == starting_gate.arrive());
	auto await_phase(
		starting_gate.await_advance_for(phase, 30s)
	);

	if (expect_next_phase)
		BOOST_TEST(await_phase == (phase + 1));
	else
		BOOST_TEST((
			(await_phase == phase) || (await_phase == (phase + 1))
		));
}

struct arriver {
	arriver(phaser &p_)
	: p(p_), phase(0), advanced(false)
	{}

	virtual ~arriver()
	{}

	virtual void operator()()
	{
		to_the_starting_gate();
		phase = p.arrive();
	}

	static std::atomic<int> count;

	phaser &p;
	phaser::phase_type phase;
	bool advanced;
};

std::atomic<int> arriver::count(1);

struct awaiter_type_0 {};
struct awaiter_type_1 {};

template <typename T>
struct awaiter;

template <>
struct awaiter<awaiter_type_0> : arriver {
	awaiter(phaser &p_)
	: arriver(p_)
	{}

	virtual void operator()()
	{
		to_the_starting_gate();

		if (!(cycle_arrive_await_advance++ & 1))
			phase = p.await_advance(p.arrive());
		else
			phase = p.arrive_and_await_advance();
	}
};

template <>
struct awaiter<awaiter_type_1> : arriver {
	awaiter(phaser &p_)
	: arriver(p_)
	{}

	virtual void operator()()
	{
		to_the_starting_gate();
		phase = p.await_advance_interruptibly(p.arrive());
	}
};

template <typename Rep, typename Period>
struct awaiter<std::chrono::duration<Rep, Period>> : arriver {
	typedef std::chrono::duration<Rep, Period> duration_type;

	awaiter(phaser &p_, duration_type const &rel_time_)
	: arriver(p_), rel_time(rel_time_)
	{}

	virtual void operator()()
	{
		to_the_starting_gate();
		phase = p.await_advance_for(p.arrive(), rel_time);
	}

	duration_type rel_time;
};

struct arriver_producer {
	arriver_producer(phaser &p_)
	: p(p_), pos(0)
	{}

	arriver *next()
	{
		switch ((pos++) & 7) {
		case 0:
		case 4:
			return new arriver(p);
		case 1:
		case 5:
			return new awaiter<awaiter_type_0>(p);
		case 2:
		case 6:
		case 7:
			return new awaiter<awaiter_type_1>(p);
		default:
			return new awaiter<decltype(30s)>(p, 30s);
		}
	}

	phaser &p;
	size_t pos;
};

struct awaiter_producer {
	awaiter_producer(phaser &p_)
	: p(p_), pos(0)
	{}

	arriver *next()
	{
		switch ((pos++) & 7) {
		case 1:
		case 4:
		case 7:
			return new awaiter<awaiter_type_0>(p);
		case 2:
		case 5:
			return new awaiter<awaiter_type_1>(p);
		default:
			return new awaiter<decltype(30s)>(p, 30s);
		}
	}

	phaser &p;
	size_t pos;
};

void check_terminated(phaser &p)
{
	BOOST_TEST(p.is_terminated());
	auto unarrived_parties(p.get_unarrived_parties());
	auto registered_parties(p.get_registered_parties());
	auto phase(p.get_phase());
	BOOST_TEST(phase == p.arrive());
	BOOST_TEST(phase == p.arrive_and_deregister());
	BOOST_TEST(phase == p.arrive_and_await_advance());
	BOOST_TEST(phase == p.register_some(10));
	BOOST_TEST(phase == p.register_one());

	BOOST_TEST(phase == p.await_advance_interruptibly(0));
	BOOST_TEST(phase == p.await_advance_for(0, 10s));

	BOOST_TEST(p.get_unarrived_parties() == unarrived_parties);
	BOOST_TEST(p.get_registered_parties() == registered_parties);
}

}

BOOST_AUTO_TEST_CASE(phaser_0_0)
{
	phaser p(3);
	BOOST_TEST(p.get_registered_parties() == 3);
	BOOST_TEST(p.get_arrived_parties() == 0);
	BOOST_TEST(p.get_phase() == 0);
	BOOST_TEST(!p.is_terminated());

	test::arriver_producer a_prod(p);
	phaser::phase_type phase(0);

	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		BOOST_TEST(p.get_phase() == phase);
		++phase;
		auto a0(a_prod.next());
		auto a1(a_prod.next());
		thread a0t(*a0);
		thread a1t(*a1);
		test::to_the_starting_gate();

		p.arrive_and_await_advance();

		a0t.join();
		a1t.join();
		BOOST_TEST(!p.is_terminated());
		BOOST_TEST(p.get_registered_parties() == 3);
		BOOST_TEST(p.get_arrived_parties() == 0);
		delete a0;
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(phaser_0_1)
{
	phaser p(3);
	test::arriver_producer a_prod(p);
	phaser::phase_type phase(0);

	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		BOOST_TEST(p.get_phase() == phase);
		test::awaiter<decltype(30s)> a0(p, 30s);
		auto a1(a_prod.next());
		thread a0t(a0);
		thread a1t(*a1);

		test::to_the_starting_gate();

		a0t.interrupt();
                a0t.join();

		p.arrive_and_await_advance();

		a1t.join();

		BOOST_TEST(!p.is_terminated());
		BOOST_TEST(p.get_registered_parties() == 3);
		BOOST_TEST(p.get_arrived_parties() == 0);
		++phase;
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(phaser_0_2)
{
	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		phaser p(3);
		test::awaiter_producer a_prod(p);

		auto a0(a_prod.next());
		auto a1(a_prod.next());
		thread a0t(*a0);
		thread a1t(*a1);

		test::to_the_starting_gate();

		while (p.get_arrived_parties() < 2)
			std::this_thread::yield();

		BOOST_TEST(p.get_phase() == 0);
		p.force_termination();
		a0t.join();
		a1t.join();
		BOOST_TEST(a0->phase == 0);
		BOOST_TEST(a1->phase == 0);
		auto arrived_parties(p.get_arrived_parties());
		test::check_terminated(p);
		BOOST_TEST(p.get_arrived_parties() == arrived_parties);
		delete a0;
		delete a1;
	}
}

}

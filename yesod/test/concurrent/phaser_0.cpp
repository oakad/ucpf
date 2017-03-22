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

#define ATTEMPT_COUNT 10

namespace boost::test_tools::tt_detail {

template <>
struct print_log_value<ucpf::yesod::concurrent::phaser::phase_type> {
	void operator()(
		std::ostream &os,
		ucpf::yesod::concurrent::phaser::phase_type const &p
	)
	{
		os << p.value();
	}
};
}

namespace ucpf::yesod::concurrent {

using namespace std::chrono_literals;

namespace test {

using ucpf::yesod::test::check_all;

std::atomic<int> cycle_arrive_await_advance(1);
phaser default_starting_gate(3);
phaser *starting_gate = &default_starting_gate;

void to_the_starting_gate(BOOST_TEST_DECLARE_LOC_STORE_REF())
{
	auto expect_next_phase(starting_gate->get_unarrived_parties() == 1);
	auto phase(starting_gate->get_phase());

	BOOST_TEST_LOC(phase == starting_gate->arrive());

	phaser::phase_type await_phase;
	bool interrupted(false);

	while (true) {
		await_phase = starting_gate->await_advance_for(phase, 30s);
		if (await_phase.interrupted()) {
			interrupted = true;
			continue;
		}
		if (interrupted)
			this_thread::get().interrupt();
		break;
	}

	if (expect_next_phase)
		BOOST_TEST_LOC(await_phase.value() == (phase + 1).value());
	else
		BOOST_TEST_LOC((
			(await_phase.value() == phase.value())
			|| (await_phase.value() == (phase + 1).value())
		));
}

struct arriver {
	arriver(arriver const &) = delete;
	arriver(arriver &&) = delete;

	arriver(phaser &p_)
	: p(p_)
	{}

	virtual ~arriver()
	{}

	virtual void operator()()
	{
		to_the_starting_gate(boost_test_assertions);
		phase = p.arrive();
	}

	void report()
	{
		BOOST_TEST_REPORT_LOCALS();
	}

	phaser &p;
	phaser::phase_type phase;
	BOOST_TEST_DECLARE_LOC_STORE();
};

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
		to_the_starting_gate(boost_test_assertions);

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
		to_the_starting_gate(boost_test_assertions);
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
		to_the_starting_gate(boost_test_assertions);
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
	BOOST_TEST(phase.terminal());
	BOOST_TEST(phase == p.arrive());
	BOOST_TEST(phase == p.arrive_and_deregister());
	BOOST_TEST(phase == p.arrive_and_await_advance());
	BOOST_TEST(phase == p.register_some(10));
	BOOST_TEST(phase == p.register_one());

	BOOST_TEST(phase == p.await_advance_interruptibly({}));
	BOOST_TEST(phase == p.await_advance_for({}, 10s));

	BOOST_TEST(p.get_unarrived_parties() == unarrived_parties);
	BOOST_TEST(p.get_registered_parties() == registered_parties);
}

}

BOOST_AUTO_TEST_SUITE(phaser_0)

BOOST_AUTO_TEST_CASE(t0)
{
	phaser p(3);
	BOOST_TEST(p.get_registered_parties() == 3);
	BOOST_TEST(p.get_arrived_parties() == 0);
	BOOST_TEST(p.get_phase().value() == 0);
	BOOST_TEST(!p.is_terminated());
	BOOST_TEST_DECLARE_LOC_STORE();

	test::arriver_producer a_prod(p);
	phaser::phase_type phase;

	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		BOOST_TEST(p.get_phase() == phase);
		++phase;
		auto a0(a_prod.next());
		auto a1(a_prod.next());
		thread a0t(std::ref(*a0));
		thread a1t(std::ref(*a1));
		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();

		p.arrive_and_await_advance();

		a0t.join();
		a1t.join();
		a0->report();
		a1->report();

		BOOST_TEST(!p.is_terminated());
		BOOST_TEST(p.get_registered_parties() == 3);
		BOOST_TEST(p.get_arrived_parties() == 0);
		delete a0;
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(t1)
{
	phaser p(3);
	test::arriver_producer a_prod(p);
	phaser::phase_type phase;
	BOOST_TEST_DECLARE_LOC_STORE();

	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		BOOST_TEST(p.get_phase() == phase);
		test::awaiter<decltype(30s)> a0(p, 30s);
		auto a1(a_prod.next());
		thread a0t(std::ref(a0));
		thread a1t(std::ref(*a1));

		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();

		a0t.interrupt();
                a0t.join();

		p.arrive_and_await_advance();

		a1t.join();
		a0.report();
		a1->report();
		BOOST_TEST(a0.phase.interrupted());
		BOOST_TEST(a1->phase.normal());

		BOOST_TEST(!p.is_terminated());
		BOOST_TEST(p.get_registered_parties() == 3);
		BOOST_TEST(p.get_arrived_parties() == 0);
		++phase;
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(t2)
{
	BOOST_TEST_DECLARE_LOC_STORE();
	for (int c(0); c < ATTEMPT_COUNT; ++c) {
		phaser p(3);
		test::awaiter_producer a_prod(p);
		auto a0(a_prod.next());
		auto a1(a_prod.next());
		thread a0t(std::ref(*a0));
		thread a1t(std::ref(*a1));

		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();

		while (p.get_arrived_parties() < 2)
			std::this_thread::yield();

		BOOST_TEST(p.get_phase().value() == 0);
		p.force_termination();
		a0t.join();
		a1t.join();
		a0->report();
		a1->report();
		BOOST_TEST(a0->phase.terminal());
		BOOST_TEST(a1->phase.terminal());
		auto arrived_parties(p.get_arrived_parties());
		test::check_terminated(p);
		BOOST_TEST(p.get_arrived_parties() == arrived_parties);
		delete a0;
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(t3)
{
	struct arr {
		arr(test::arriver *a_)
		: a(a_)
		{}

		arr(arr &&other)
		: a(other.a), t(std::move(other.t))
		{
			other.a = nullptr;
		}

		void start()
		{
			t = thread(std::ref(*a));
		}

		~arr()
		{
			delete a;
		}

		test::arriver *a;
		thread t;
	};

	phaser p(1);
	test::arriver_producer a_prod(p);
	std::vector<arr> arriver_list;
	auto phase(p.get_phase());
	BOOST_TEST_DECLARE_LOC_STORE();

	for (size_t c(1); c < 5; ++c) {
		phaser sg(1 + (3 * c));
		test::starting_gate = &sg;
		BOOST_TEST(p.get_phase() == phase);

		p.register_one();
		p.register_one();
		p.register_one();

		for (size_t d(0); d < (3 * c); ++d)
			arriver_list.emplace_back(a_prod.next());

		for (auto &a: arriver_list)
			a.start();

		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();

		p.arrive_and_await_advance();

		for (auto &a: arriver_list) {
			a.t.join();
			a.a->report();
			BOOST_TEST(a.a->phase.normal());
		}

		BOOST_TEST(p.get_registered_parties(), 1 + (3 * c));
		BOOST_TEST(p.get_arrived_parties() == 0);
		arriver_list.clear();
		++phase;
	}

	test::starting_gate = &test::default_starting_gate;
}

BOOST_AUTO_TEST_CASE(t4)
{
	phaser p(3);
	test::arriver_producer a_prod(p);
	BOOST_TEST_DECLARE_LOC_STORE();

	for (auto tm: {0ms, 12ms}) {
		test::awaiter<decltype(tm)> a0(p, tm);
		auto a1(a_prod.next());
		thread a0t(std::ref(a0));
		thread a1t(std::ref(*a1));
		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();
		a0t.join();
		BOOST_TEST(a0.phase.timed_out());

		p.arrive();
		a1t.join();

		a0.report();
		a1->report();

		BOOST_TEST(a1->phase.normal());
		BOOST_TEST(!p.is_terminated());
		delete a1;
	}
}

BOOST_AUTO_TEST_CASE(t5)
{
	phaser *q;
	struct nn_ : phaser::notification {
		bool on_advance(
			phaser &p, phaser::phase_type phase,
			phaser::count_type registered_parties
		) override
		{
			auto count_phase = count++;
			BOOST_TEST(count_phase == phase.value());
			BOOST_TEST((*q)->get_phase() == phase);
			BOOST_TEST(
				(*q)->get_registered_parties(),
				registered_parties
			);
			return phase.value() >= 3;
		}

		nn_(phaser **q_)
		: q(q_)
		{}

		std::atomic<uint32_t> count = {0};
		phaser **q;
	} nn(&q);

	phaser p(3, nullptr, &nn);
	q = &p;
	test::awaiter_producer a_prod(p);
	BOOST_TEST_DECLARE_LOC_STORE();
	BOOST_TEST(p.get_registered_parties() == 3);

	for (size_t c(0); c < 4; ++c) {
		auto a0(a_prod.next());
		auto a1(a_prod.next());
		thread a0t(std::ref(*a0));
		thread a1t(std::ref(*a1));
		test::to_the_starting_gate(boost_test_assertions);
		BOOST_TEST_REPORT_LOCALS();

		while (p.get_arrived_parties() < 2)
			std::this_thread::yield();

		p.arrive();
		a0t.join();
		a1t.join();
		a0->report();
		a1->report();

		BOOST_TEST(a0->phase.normal());
		BOOST_TEST(a1->phase.normal());
		BOOST_TEST(nn.count.load(), c + 1);
		if (c < 3) {
			BOOST_TEST(!p.is_terminated());
			BOOST_TEST(p.get_registered_parties() == 3);
			BOOST_TEST(p.get_arrived_parties() == 0);
			BOOST_TEST(p.get_unarrived_parties() == 3);
			BOOST_TEST(p.get_phase().value() == nn.count.load());
		} else
			test::check_terminated(p);

		delete a0;
		delete a1;
	}
}

BOOST_AUTO_TEST_SUITE_END()

}

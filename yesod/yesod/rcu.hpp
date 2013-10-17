/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on implementation of userspace rcu library (http://lttng.org/urcu):
 *
 *      Copyright (c) 2009 - 2013 Mathieu Desnoyers
 *                                <mathieu.desnoyers@efficios.com>
 *      Copyright (c) 2006 - 2010 Paul E. McKenney, IBM Corporation.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_RCU_OCT_03_2013_1805)
#define UCPF_YESOD_RCU_OCT_03_2013_1805

#include <thread>
#include <condition_variable>

#include <boost/intrusive/list.hpp>

#include <yesod/stack.hpp>

namespace ucpf { namespace yesod { namespace rcu {

struct group;

struct reader {
	friend struct group;

	reader() : count(0)
	{}

private:
	boost::intrusive::list_member_hook<
		boost::intrusive::link_mode<boost::intrusive::safe_link>
	> group_hook;

	stack_head waiter_head;

	struct waiter_states {
		static constexpr long WAITING  = 0;
		static constexpr long WAKEUP   = 1;
		static constexpr long RUNNING  = 2;
		static constexpr long TEARDOWN = 4;
	};

	std::atomic<int_fast64_t> count;
	std::atomic<long> state;
	std::atomic_flag working;
};

struct group {
	group() : count(1)
	{}

	void attach(reader &r)
	{
		std::unique_lock<std::mutex> l_g(lock);
		r_list.push_back(r);
		resume(r);
	}

	void detach(reader &r)
	{
		suspend(r);

		std::unique_lock<std::mutex> l_g(lock);
		r_list.erase(r_list.iterator_to(r));
	}

	void suspend(reader &r)
	{
		r.count.store(0);
		if (!r.working.test_and_set()) {
			if (state.load() != -1L)
				return;
			state = 0;
			cond_var.notify_all();
		}
	}

	void resume(reader &r)
	{
		r.count.store(count.load());
	}

	void yield(reader &r)
	{
		auto count_(count.load());

		if (count_ != r.count.exchange(count_)) {
			if (!r.working.test_and_set()) {
				if (state.load() != -1L)
					return;
				state = 0;
				cond_var.notify_all();
			}
		}
	}

	void synchronize(reader &r)
	{
		auto was_online(r.count.load());
		if (was_online)
			suspend(r);

		r.state = reader::waiter_states::WAITING;

		if (!w_stack.push(r))
			busy_wait(r);
		else {
			r.state = reader::waiter_states::RUNNING;
			std::unique_lock<std::mutex> l_g(lock);
			decltype(w_stack) lw_stack;
			lw_stack.splice(w_stack);
			if (!r_list.empty()) {
				count += 2;
				wait_for_readers(l_g);
			}
			l_g.unlock();

			for (
				reader *wr(lw_stack.pop()); wr;
				wr = lw_stack.pop()
			) {
				if (!(
					wr->state.load()
					& reader::waiter_states::RUNNING
				)) {
					wr->state
					= reader::waiter_states::WAKEUP;

					cond_var.notify_all();

					wr->state
					|= reader::waiter_states::TEARDOWN;
				}
			}
		}

		if (was_online)
			resume(r);
	}

private:
	void busy_wait(reader &r)
	{
		for (long retries(0); retries < wait_attempts; ++retries) {
			if (r.state.load() != reader::waiter_states::WAITING)
				break;
			std::atomic_thread_fence(std::memory_order_seq_cst);
		}

		if (r.state.load() == reader::waiter_states::WAITING) {
			std::unique_lock<std::mutex> l_g(lock);
			while (r.state.load() == reader::waiter_states::WAITING)
				cond_var.wait(l_g);
		}

		r.state |= reader::waiter_states::RUNNING;
		for (long retries(0); retries < wait_attempts; ++retries) {
			if (r.state.load() & reader::waiter_states::TEARDOWN)
				break;
			std::atomic_thread_fence(std::memory_order_seq_cst);
		}

		while (!r.state.load() & reader::waiter_states::TEARDOWN)
			std::this_thread::sleep_for(
				std::chrono::milliseconds(10)
			);
	}

	void wait_for_readers(std::unique_lock<std::mutex> &l_g)
	{
		decltype(r_list) lr_list;
		long retries(0);
		auto count_(count.load());

		while (true) {
			++retries;

			if (retries >= wait_attempts) {
				state = -1;
				for (auto &r : r_list)
					r.working.clear();
			}

			auto p(r_list.begin());
			while (p != r_list.end()) {
				auto r_count(p->count.load());

				if (!r_count || (r_count == count_)) {
					auto &r(*p);
					p = r_list.erase(p);
					lr_list.push_back(r);
				}
			}

			if (r_list.empty()) {
				if (retries >= wait_attempts)
					state = 0;
				break;
			} else {
				if (retries >= wait_attempts) {
					while (state.load() == -1L)
						cond_var.wait(l_g);
				} else
					std::atomic_thread_fence(
						std::memory_order_seq_cst
					);
			}
		}

		r_list.splice(r_list.cend(), lr_list);
	}

	static constexpr long wait_attempts = 100;

	std::mutex lock;
	std::atomic<int_fast64_t> count;
	std::atomic<long> state;
	std::condition_variable cond_var;

	boost::intrusive::list<
		reader, boost::intrusive::member_hook<
			reader, decltype(reader::group_hook),
			&reader::group_hook
		>, boost::intrusive::constant_time_size<false>
	> r_list;

	stack<
		reader, decltype(reader::waiter_head), &reader::waiter_head
	> w_stack;
};

}}}
#endif

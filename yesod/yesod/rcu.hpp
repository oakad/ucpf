/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on implementation of userspace rcu library (http://lttng.org/urcu):
 *
 *      Copyright (c) 2009 - 2013 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *      Copyright (c) 2006 - 2010 Paul E. McKenney, IBM Corporation.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_RCU_OCT_03_2013_1805)
#define UCPF_YESOD_RCU_OCT_03_2013_1805

#include <mutex>

#include <boost/intrusive/list.hpp>

#include <yesod/stack.hpp>

namespace ucpf { namespace yesod { namespace rcu {

struct group;

struct reader {
	friend struct group;

private:
	boost::intrusive::list_member_hook<
		boost::intrusive::link_mode<boost::intrusive::safe_link>
	> group_hook;

	stack_head waiter_head;

	struct waiter_states {
		static constexpr long WAITING = 0;
		static constexpr long WAKEUP = 1;
		static constexpr long RUNNING = 2;
		static constexpr long TEARDOWN = 4;
	};

	std::atomic<long> count;
	std::atomic<long> state;
	std::atomic_flag working;
};

struct group {
	void attach(reader &r)
	{
		std::lock_guard<std::mutex> g_(r_lock);
		r_list.push_back(r);
		resume(r);
	}

	void detach(reader &r)
	{
		suspend(r);

		std::lock_guard<std::mutex> g_(r_lock);
		r_list.erase(r_list.iterator_to(r));
	}

	void suspend(reader &r)
	{
		r.count.store(0);
		if (!r.working.test_and_set()) {
			if (cond_val.load() != -1L)
				return;
			cond_val.store(0);
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
				if (cond_val.load() != -1L)
					return;
				cond_val.store(0);
				cond_var.notify_all();
			}
		}
	}

	void synchronize(reader &r)
	{
		auto was_online(r.count.load());
		if (was_online)
			suspend(r);



		//....

		if (was_online)
			resume(r);
	}

private:
	std::mutex r_lock;
	std::atomic<long> count;
	std::atomic<long> cond_val;
	std::condition_variable cond_var;

	boost::intrusive::list<
		reader,
		boost::intrusive::member_hook_option<
			reader, decltype(reader::group_hook),
			&reader::group_hook
		>,
		boost::intrusive::constant_time_size<false>
	> r_list;

	stack<
		reader, decltype(reader::stack_head), &reader::stack_head
	> w_stack;
};

}}}
#endif

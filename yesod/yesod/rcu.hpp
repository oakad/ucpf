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

#include <list>
#include <mutex>
#include <thread>
#include <atomic>

namespace ucpf { namespace yesod { namespace rcu {
namespace detail {

struct thread_entry {
	static std::mutex entry_lock;
	static std::list<thread_entry *> entries;

	std::thread::id t_id;
	unsigned long count;
	std::atomic_flag waiting;
	std::list<thread_entry *>::iterator self_ref;

	thread_entry()

	~thread_entry()
	{
	}
};

extern thread_local thread_entry rcu_reader;
extern std::atomic<unsigned long> gp_count;

static inline void wake_up()
{
}

}

static inline void read_lock()
{
}

static inline void read_unlock()
{
}

int read_ongoing();

static inline void yield()
{
	unsigned long gp_count(detail::gp_count.load());

	if (gp_count == rcu_reader.count)
		return;

	std::atomic_thread_fence(std::memory_order_acquire);

	detail::gp_count.store(gp_count);

	std::atomic_thread_fence(std::memory_order_release);

	detail::wake_up();

	std::atomic_thread_fence(std::memory_order_release);
}

void thread_offline();
void thread_online();
void synchronize();


}}}
#endif

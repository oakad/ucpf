/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SHARED_MUTEX_DEC_09_2013_1200)
#define UCPF_YESOD_SHARED_MUTEX_DEC_09_2013_1200

#include <yesod/arch/timed_mutex.hpp>
#include <yesod/arch/event_variable.hpp>

#include <atomic>
#include <mutex>

namespace ucpf { namespace yesod {

struct shared_mutex {
	shared_mutex()
	: write_lock(false), reader_count(0)
	{}

	void lock()
	{
		std::unique_lock<timed_mutex> l_g(state_lock);

		while (write_lock.test_and_set())
			ev.wait();
	}

	void lock_shared()
	{
		std::unique_lock<timed_mutex> l_g(state_lock);

		if (!reader_count.fetch_add(1)) {
			while (write_lock.test_and_set())
				ev.wait();
		}
	}

	void unlock()
	{
		write_lock.clear();
		ev.notify_one();
	}

	void unlock_shared()
	{
		if (reader_count.fetch_sub(1) == 1)
			unlock();
	}

	bool try_lock()
	{
		std::unique_lock<timed_mutex> l_g(state_lock, std::try_to_lock);
		if (!l_g.owns_lock())
			return false;

		return  !write_lock.test_and_set();
	}

	bool try_lock_shared()
	{
		std::unique_lock<timed_mutex> l_g(state_lock, std::try_to_lock);
		if (!l_g.owns_lock())
			return false;

		if (reader_count) {
			write_lock.test_and_set();
			++reader_count;
			return true;
		} else {
			if (!write_lock.test_and_set()) {
				++reader_count;
				return true;
			} else
				return false;
		}
	}

private:
	event_variable ev;
	timed_mutex state_lock;
	std::atomic_flag write_lock;
	std::atomic_ulong reader_count;
};

}}
#endif

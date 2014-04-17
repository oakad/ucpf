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
	typedef timed_mutex<> base_mutex_type;

	shared_mutex()
	: write_lock(false), reader_count(0)
	{}

	void lock()
	{
		std::unique_lock<base_mutex_type> l_g(state_lock);

		while (write_lock.test_and_set())
			ev.wait();
	}

	void lock_shared()
	{
		std::unique_lock<base_mutex_type> l_g(state_lock);

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
		std::unique_lock<base_mutex_type> l_g(
			state_lock, std::try_to_lock
		);
		if (!l_g.owns_lock())
			return false;

		return  !write_lock.test_and_set();
	}

	bool try_lock_shared()
	{
		std::unique_lock<base_mutex_type> l_g(
			state_lock, std::try_to_lock
		);
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
	event_variable<> ev;
	base_mutex_type state_lock;
	std::atomic_flag write_lock;
	std::atomic_ulong reader_count;
};

template <typename Mutex>
struct shared_lock {
	typedef Mutex mutex_type;

	explicit shared_lock(mutex_type &m_)
	: m(&m_)
	{
		lock();
	}

	~shared_lock()
	{
		unlock();
	}

	void lock()
	{
		m->lock_shared();
	}

	void unlock()
	{
		m->unlock_shared();
	}

private:
	mutex_type *m;
};

}}
#endif

/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_0BB98E3E060042EC0795F16D0FE2A824)
#define HPP_0BB98E3E060042EC0795F16D0FE2A824

#include <mutex>
#include <yesod/timed_mutex.hpp>

namespace ucpf { namespace yesod {

struct shared_mutex {
	typedef timed_mutex base_mutex_type;

	static void init(shared_mutex &m)
	{
		base_mutex_type::init(m.self_lock);
		base_mutex_type::init(m.writer_lock);
		m.reader_count = 0;
	}

	void lock()
	{
		std::unique_lock<base_mutex_type> lg(self_lock);
		writer_lock.lock();
	}

	void lock_shared()
	{
		std::unique_lock<base_mutex_type> lg(self_lock);
		if (!__atomic_fetch_add(&reader_count, 1, __ATOMIC_ACQUIRE))
			writer_lock.lock();
	}

	void unlock()
	{
		writer_lock.unlock();
	}

	void unlock_shared()
	{
		if (1 == __atomic_fetch_sub(&reader_count, 1, __ATOMIC_RELEASE))
			writer_lock.unlock();
	}

	bool try_lock()
	{
		std::unique_lock<base_mutex_type> lg(
			self_lock, std::try_to_lock
		);
		if (!lg.owns_lock())
			return false;

		return  writer_lock.try_lock();
	}

	bool try_lock_shared()
	{
		std::unique_lock<base_mutex_type> lg(
			self_lock, std::try_to_lock
		);
		if (!lg.owns_lock())
			return false;

		if (__atomic_fetch_add(&reader_count, 1, __ATOMIC_ACQUIRE))
			return true;

		if (writer_lock.try_lock())
			return true;

		__atomic_sub_fetch(&reader_count, 1, __ATOMIC_RELEASE);
		return false;
	}

private:
	base_mutex_type self_lock;
	base_mutex_type writer_lock;
	uint32_t volatile reader_count;
};

}}
#endif

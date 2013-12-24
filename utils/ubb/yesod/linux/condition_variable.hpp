/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_CONDITION_VARIABLE_DEC_10_2013_1615)
#define UCPF_YESOD_CONDITION_VARIABLE_DEC_10_2013_1615

extern "C" {

#include <linux/futex.h>
#include <sys/syscall.h>

}

#include <chrono>
#include <atomic>

namespace ucpf { namespace yesod {

template <typename Mutex>
struct condition_variable {
	condition_variable()
	: seq_count(0), mutex_ptr(nullptr), waiters(0)
	{}

	void notify_one()
	{
		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_WAKE_PRIVATE, 1, nullptr,
			nullptr, 0
		);
	}

	void notify_all()
	{
		if (!waiters.load())
			return;

		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_REQUEUE_PRIVATE, 1, nullptr,
			mutex_ptr.load(), 0
		);
	}

	void wait(Mutex &m)
	{
		return wait_impl(m, nullptr);
	}

	template <typename Rep, typename Period>
	bool wait_for(
		Mutex &m, std::chrono::duration<Rep, Period> const &rel_time
	)
	{
		std::chrono::seconds s(rel_time);
		std::chrono::nanoseconds ns(rel_time - s);
		struct timespec ts = { s, ns };

		return wait_impl(&ts);
	}

	template <typename Clock, typename Duration>
	bool wait_until(
		Mutex &m,
		std::chrono::time_point<Clock, Duration> const &timeout_time
	)
	{
		return wait_for(
			timeout_time - std::chrono::system_clock::now()
		);
	}

private:
	bool wait_impl(Mutex &m, struct timespec *rel_time)
	{
		int c_seq(__atomic_load_4(&seq_count, __ATOMIC_SEQ_CST));
		mutex_ptr.store(m.native_ptr());
		++waiters;

		m.unlock();

		bool rv(0 == futex(
			&seq_count, FUTEX_WAIT_PRIVATE, c_seq,
			rel_time, nullptr, 0
		));

		m.lock_blocking();

		--waiters;
		return rv;
	}

	static int futex(
		int *uaddr, int op, int val, struct timespec const *timeout,
		int *uaddr2, int val3
	)
	{
		return syscall(
			SYS_futex, uaddr, op, val, timeout, uaddr2, val3
		);
	}

	alignas(8) int seq_count;
	std::atomic<int *> mutex_ptr;
	std::atomic_uint waiters;
};

}}

#endif

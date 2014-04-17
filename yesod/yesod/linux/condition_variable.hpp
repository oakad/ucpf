/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_CONDITION_VARIABLE_DEC_10_2013_1615)
#define UCPF_YESOD_CONDITION_VARIABLE_DEC_10_2013_1615

#include <yesod/linux/timed_mutex.hpp>

namespace ucpf { namespace yesod {

template <bool Interprocess = false>
struct condition_variable {
	typedef timed_mutex<Interprocess> base_mutex_type;

	static void init(condition_variable &cv)
	{
		__atomic_store_n(&cv.seq_count, 0, __ATOMIC_SEQ_CST);
		__atomic_store_n(&cv.mutex_ptr, 0, __ATOMIC_SEQ_CST);
		__atomic_store_n(&cv.waiters, 0, __ATOMIC_SEQ_CST);
	}

	condition_variable(
		typename std::enable_if<!Interprocess>::type * = nullptr
	)
	{
		init(*this);
	}

	void notify_one()
	{
		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_WAKE | futex_op_flags, 1, nullptr,
			nullptr, 0
		);
	}

	void notify_all()
	{
		if (!__atomic_load_n(&waiters))
			return;

		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_REQUEUE | futex_op_flags, 1, nullptr,
			__atomic_load_n(&mutex_ptr), 0
		);
	}

	void wait(Mutex &m)
	{
		return wait_impl(m, nullptr);
	}

	template <typename Rep, typename Period>
	bool wait_for(
		base_mutex_type &m,
		std::chrono::duration<Rep, Period> const &rel_time
	)
	{
		std::chrono::seconds s(rel_time);
		std::chrono::nanoseconds ns(rel_time - s);
		struct timespec ts = { s, ns };

		return wait_impl(&ts);
	}

	template <typename Clock, typename Duration>
	bool wait_until(
		base_mutex_type &m,
		std::chrono::time_point<Clock, Duration> const &timeout_time
	)
	{
		return wait_for(
			timeout_time - std::chrono::system_clock::now()
		);
	}

private:
	bool wait_impl(base_mutex_type &m, struct timespec *rel_time)
	{
		int c_seq(__atomic_load_n(&seq_count, __ATOMIC_SEQ_CST));
		__atomic_store_n(&mutex_ptr, m.native_ptr(), __ATOMIC_SEQ_CST);
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
		int volatile *uaddr, int op, int val,
		struct timespec const *timeout, int volatile *uaddr2, int val3
	)
	{
		return syscall(
			SYS_futex, uaddr, op, val, timeout, uaddr2, val3
		);
	}

	constexpr static int futex_op_flags
	= Interprocess ? 0 : FUTEX_PRIVATE_FLAG;
	alignas(8) int volatile seq_count;
	int volatile *mutex_ptr;
	unsigned int volatile waiters;
};

}}

#endif

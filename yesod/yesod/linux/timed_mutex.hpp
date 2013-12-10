/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_TIMED_MUTEX_DEC_10_2013_1420)
#define UCPF_YESOD_TIMED_MUTEX_DEC_10_2013_1420

extern "C" {

#include <linux/futex.h>
#include <sys/syscall.h>

}

#include <chrono>

namespace ucpf { namespace yesod {

struct timed_mutex {
	friend struct condition_variable;

	timed_mutex()
	: state(0)
	{}

	void lock()
	{
		for (auto cnt(0); cnt < spin_count; ++cnt) {
			if (locked & __atomic_fetch_or(
				&state, locked, __ATOMIC_SEQ_CST
			))
				__atomic_thread_fence(__ATOMIC_SEQ_CST);
			else
				return;
		}

		lock_blocking();
	}

	void unlock()
	{
		if (locked == __atomic_load_4(&state, __ATOMIC_SEQ_CST)) {
			auto expected(locked);
			if (__atomic_compare_exchange_4(
				&state, &expected, 0, true,
				__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
			))
				return;
		}

		__atomic_and_fetch(&state, ~locked, __ATOMIC_SEQ_CST);

		for (auto cnt(0); cnt < spin_count; ++cnt) {
			if (locked & __atomic_load_4(&state, __ATOMIC_SEQ_CST))
				return;

			__atomic_thread_fence(__ATOMIC_SEQ_CST);
		}

		__atomic_and_fetch(&state, ~busy, __ATOMIC_SEQ_CST);

		futex(&state, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
	}

	bool try_lock()
	{
		return locked != __atomic_fetch_or(
			&state, locked, __ATOMIC_SEQ_CST
		);
	}

	template <typename Rep, typename Period>
	bool try_lock_for(std::chrono::duration<Rep, Period> const &rel_time)
	{
		return try_lock_until(
			std::chrono::system_clock::now() + rel_time
		);
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(
		std::chrono::time_point<Clock, Duration> const &timeout_time
	)
	{
		if (try_lock())
			return true;

		while (locked & __atomic_fetch_or(
			&state, locked | busy, __ATOMIC_SEQ_CST
		)) {
			auto rel_time(
				timeout_time - std::chrono::system_clock::now()
			);
			if (rel_time < 0)
				return false;

			std::chrono::seconds s(rel_time);
			std::chrono::nanoseconds ns(rel_time - s);
			struct timespec ts = { s, ns };
			futex(
				&state, FUTEX_WAIT_PRIVATE, locked | busy,
				&ts, nullptr, 0
			);
		}
		return true;
	}

private:
	int volatile *native_ptr()
	{
		return &state;
	}

	void lock_blocking()
	{
		while (locked & __atomic_fetch_or(
			&state, locked | busy, __ATOMIC_SEQ_CST
		))
			futex(
				&state, FUTEX_WAIT_PRIVATE, locked | busy,
				nullptr, nullptr, 0
			);
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

	constexpr static int spin_count = 100;
	constexpr static int locked = 1;
	constexpr static int busy = 2;
	alignas(8) int state;
};

}}

#endif

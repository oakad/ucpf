/*
 * Copyright (c) 2013-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_A8E2E349CFCE033202B050F7AE6374E8)
#define HPP_A8E2E349CFCE033202B050F7AE6374E8

extern "C" {

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

}

#include <chrono>

namespace ucpf { namespace yesod {
namespace detail {

template <bool Interprocess = false, int SpinCount = 100>
struct timed_mutex {
	static void init(timed_mutex &m)
	{
		__atomic_store_n(&m.state, 0, __ATOMIC_RELAXED);
	}

	void lock()
	{
		for (auto cnt(0); cnt < SpinCount; ++cnt) {
			if (!(locked & __atomic_fetch_or(
				&state, locked, __ATOMIC_ACQUIRE
			)))
				return;
		}

		lock_blocking();
	}

	bool try_lock()
	{
		return locked != __atomic_fetch_or(
			&state, locked, __ATOMIC_ACQUIRE
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
		while (locked & __atomic_fetch_or(
			&state, locked | busy, __ATOMIC_ACQUIRE
		)) {
			auto rel_time(
				timeout_time - std::chrono::system_clock::now()
			);
			if (rel_time < 0)
				return false;

			std::chrono::seconds s(rel_time);
			std::chrono::nanoseconds ns(rel_time - s);
			struct timespec ts = { s, ns };
			if (0 != futex(
				&state, FUTEX_WAIT | futex_op_flags,
				locked | busy, &ts, nullptr, 0
			))
				return false;
		}

		return true;
	}

	void unlock()
	{
		auto expected(locked);
		if (__atomic_compare_exchange_n(
			&state, &expected, 0, true,
			__ATOMIC_ACQUIRE, __ATOMIC_RELAXED
		))
			return;

		__atomic_and_fetch(&state, ~locked, __ATOMIC_RELEASE);

		for (auto cnt(0); cnt < SpinCount; ++cnt) {
			if (locked & __atomic_load_n(&state, __ATOMIC_ACQUIRE))
				return;
		}

		__atomic_and_fetch(&state, ~busy, __ATOMIC_RELEASE);

		futex(
			&state, FUTEX_WAKE | futex_op_flags, 1, nullptr,
			nullptr, 0
		);
	}

private:
	void lock_blocking()
	{
		while (locked & __atomic_fetch_or(
			&state, locked | busy, __ATOMIC_ACQUIRE
		))
			futex(
				&state, FUTEX_WAIT | futex_op_flags,
				locked | busy, nullptr, nullptr, 0
			);
	}

	static auto futex(
		uint32_t volatile *uaddr, int op, uint32_t val,
		struct timespec const *timeout, uint32_t volatile *uaddr2,
		uint32_t val3
	)
	{
		return syscall(
			SYS_futex, uaddr, op, val, timeout, uaddr2, val3
		);
	}

	constexpr static int futex_op_flags
	= Interprocess ? 0 : FUTEX_PRIVATE_FLAG;
	constexpr static uint32_t locked = 1;
	constexpr static uint32_t busy = 2;
	alignas(4) uint32_t volatile state;
};
}

typedef detail::timed_mutex<> timed_mutex;

}}
#endif

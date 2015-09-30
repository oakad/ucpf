/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_73977D5BC67C261D454F31F006081DA6)
#define HPP_73977D5BC67C261D454F31F006081DA6

#include <chrono>

extern "C" {

#define _WIN32_WINNT 0x0600
#define NOMINMAX
#include <ntdef.h>

typedef __LONG32 ACCESS_MASK;

NTSTATUS NTAPI NtCreateKeyedEvent(
	OUT PHANDLE handle, IN ACCESS_MASK access,
	IN POBJECT_ATTRIBUTES attr, IN ULONG flags
);

NTSTATUS NTAPI NtOpenKeyedEvent(
	OUT PHANDLE handle, IN ACCESS_MASK access,
	IN POBJECT_ATTRIBUTES attr
);

NTSTATUS NTAPI NtWaitForKeyedEvent(
	IN HANDLE handle, IN void volatile *key,
	IN BOOLEAN alertable, IN PLARGE_INTEGER mstimeout
);
	
NTSTATUS NTAPI NtReleaseKeyedEvent(
	IN HANDLE handle, IN void volatile *key,
	IN BOOLEAN alertable, IN PLARGE_INTEGER mstimeout
);

}

namespace ucpf { namespace yesod {

struct timed_mutex {
	static void init(timed_mutex &m)
	{
		NtCreateKeyedEvent(&m.handle, -1, nullptr, 0);
		__atomic_store_n(&m.state, 0, __ATOMIC_RELAXED);
	}

	void lock()
	{
		while (locked == __atomic_fetch_or(
			&state, locked, __ATOMIC_ACQUIRE
		)) {
			auto expected(__atomic_load_n(
				&state, __ATOMIC_ACQUIRE
			) | locked);

			if (__atomic_compare_exchange_n(
				&state, &expected, expected + wait,
				false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED
			)) {
				NtWaitForKeyedEvent(
					handle, &state, 0, nullptr
				);
				__atomic_sub_fetch(
					&state, wake, __ATOMIC_RELEASE
				);
			}
		}
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
			&state, locked, __ATOMIC_ACQUIRE
		)) {
			auto rel_time(
				timeout_time - std::chrono::system_clock::now()
			);
			if (rel_time < 0)
				return false;

			LARGE_INTEGER ts(std::chrono::milliseconds(rel_time));

			auto expected(__atomic_load_n(
				&state, __ATOMIC_ACQUIRE
			) | locked);

			if (__atomic_compare_exchange_n(
				&state, &expected, expected + wait,
				false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED
			)) {
				if (NtWaitForKeyedEvent(
					handle, &state, 0, &ts
				)) {
					__atomic_sub_fetch(
						&state, wait, __ATOMIC_RELEASE
					);
					return false;
				}

				__atomic_sub_fetch(
					&state, wake, __ATOMIC_RELEASE
				);
			}
		}

		return true;
	}

	void unlock()
	{
		__atomic_and_fetch(&state, ~locked, __ATOMIC_RELEASE);
		while (true) {
			auto expected(__atomic_load_n(
				&state, __ATOMIC_ACQUIRE
			));

			if ((expected < wait) || (expected & (locked | wake)))
				return;

			if (__atomic_compare_exchange_n(
				&state, &expected, expected + wake - wait,
				false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED
			)) {
				NtReleaseKeyedEvent(
					handle, &state, 0, nullptr
				);
				return;
			}
		}
	}

private:
	constexpr static uint32_t locked = 1;
	constexpr static uint32_t wake = 2;
	constexpr static uint32_t wait = 4;
        alignas(4) uint32_t volatile state;
        HANDLE handle;
};

}}
#endif

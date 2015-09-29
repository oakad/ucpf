/*
 * Copyright (c) 2013-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_19189BBC519189D576180460EC54D9C7)
#define HPP_19189BBC519189D576180460EC54D9C7

extern "C" {

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

}

#include <chrono>

namespace ucpf { namespace yesod {

template <bool Interprocess = false>
struct event_variable {
	static void init(event_variable &ev)
	{
		__atomic_store_n(&ev.wait_seq, 0, __ATOMIC_RELAXED);
	}

	void notify_one()
	{
		notify_any(1);
	}

	void notify_all()
	{
		notify_any(std::numeric_limits<int>::max());
	}

	void notify_any(int cnt)
	{
		__atomic_add_fetch(&wait_seq, 1, __ATOMIC_RELEASE);
		futex(
			&wait_seq, FUTEX_WAKE | futex_op_flags, cnt,
			nullptr, nullptr, 0
		);
	}

	void wait()
	{
		uint32_t seq(__atomic_load_n(&wait_seq, __ATOMIC_ACQUIRE));
		futex(
			&wait_seq, FUTEX_WAIT | futex_op_flags, seq,
			nullptr, nullptr, 0
		);
	}

	template <typename Rep, typename Period>
	bool wait_for(std::chrono::duration<Rep, Period> const &rel_time)
	{
		std::chrono::seconds s(rel_time);
		std::chrono::nanoseconds n(rel_time - s);
		struct timespec ts = { s.count(), n.count() };

		uint32_t seq(__atomic_load_n(&wait_seq, __ATOMIC_ACQUIRE));
		return 0 == futex(
			&wait_seq, FUTEX_WAIT | futex_op_flags, seq, &ts,
			nullptr, 0
		);
	}

private:
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
	alignas(4) uint32_t volatile wait_seq;
};

}}
#endif

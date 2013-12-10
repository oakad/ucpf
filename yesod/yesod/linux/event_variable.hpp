/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_EVENT_VARIABLE_DEC_10_2013_1250)
#define UCPF_YESOD_EVENT_VARIABLE_DEC_10_2013_1250

extern "C" {

#include <linux/futex.h>
#include <sys/syscall.h>

}

#include <chrono>

namespace ucpf { namespace yesod {

struct event_variable {
	event_variable()
	: seq_count(0)
	{}

	void notify_one()
	{
		notify_any(1);
	}

	void notify_all()
	{
		notify_any(INT_MAX);
	}

	void notify_any(int cnt)
	{
		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_WAKE_PRIVATE, cnt,
			nullptr, nullptr, 0
		);
	}

	void wait()
	{
		int c_seq(__atomic_load_4(&seq_count, __ATOMIC_SEQ_CST));
		futex(
			&seq_count, FUTEX_WAIT_PRIVATE, c_seq,
			nullptr, nullptr, 0
		);
	}

	template <typename Rep, typename Period>
	bool wait_for(std::chrono::duration<Rep, Period> const &rel_time)
	{
		std::chrono::seconds s(rel_time);
		std::chrono::nanoseconds n(rel_time - s);
		struct timespec ts = { s.count(), n.count() };

		int c_seq(__atomic_load_4(&seq_count, __ATOMIC_SEQ_CST));
		return 0 == futex(
			&seq_count, FUTEX_WAIT_PRIVATE, c_seq, &ts, nullptr, 0
		);
	}

private:
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
};

}}

#endif

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
#include <unistd.h>

}

#include <chrono>

namespace ucpf { namespace yesod {

template <bool Interprocess = false>
struct event_variable {
	static void init(event_variable &ev)
	{
		__atomic_store_n(&ev.seq_count, 0, __ATOMIC_SEQ_CST);
	}

	event_variable(
		typename std::enable_if<!Interprocess>::type * = nullptr
	)
	{
		init(*this);
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
		__atomic_add_fetch(&seq_count, 1, __ATOMIC_SEQ_CST);
		futex(
			&seq_count, FUTEX_WAKE | futex_op_flags, cnt,
			nullptr, nullptr, 0
		);
	}

	void wait()
	{
		int c_seq(__atomic_load_4(&seq_count, __ATOMIC_SEQ_CST));
		futex(
			&seq_count, FUTEX_WAIT | futex_op_flags, c_seq,
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
			&seq_count, FUTEX_WAIT | futex_op_flags, c_seq, &ts,
			nullptr, 0
		);
	}

private:
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
};

}}

#endif

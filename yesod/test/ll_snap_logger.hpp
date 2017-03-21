/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_7AAD04261B97E15208EFA05493F8580E)
#define HPP_7AAD04261B97E15208EFA05493F8580E

#include <atomic>
#include <thread>
#include <condition_variable>

#include <sys/syscall.h>

#include "test.hpp"

namespace ucpf::yesod::test {

struct ll_snap_logger {
	ll_snap_logger()
	{}

	~ll_snap_logger()
	{
		auto m_list(msgs.exchange(nullptr));
		while (m_list) {
			auto m(m_list);
			m_list = m_list->next;
			::free(m);
		}
	}

	template <typename ...Args>
	void log(char const *format, Args &&...args)
	{
		auto ord = order++;
		size_t msg_len(snprintf(
			nullptr, 0, format, std::forward<Args>(args)...
		) + 1);
		auto m = reinterpret_cast<msg_info *>(::malloc(
			sizeof(msg_info) + msg_len
		));
		snprintf(
			m->msg, msg_len, format, std::forward<Args>(args)...
		);
		m->msg_len = msg_len;
		m->thread_id = ::syscall(SYS_gettid);
		m->next = msgs.load();
		m->ord = ord;
		while (!msgs.compare_exchange_weak(m->next, m)) {}
	}

	template <typename Consumer>
	void dump(Consumer &&cons, size_t count)
	{
		auto m_list(msgs.exchange(nullptr));
		std::vector<msg_info *> m_ord(count, nullptr);

		for (size_t c(0); c < count; ++c) {
			if (!m_list) {
				m_ord.resize(c);
				break;
			}
			m_ord[c] = m_list;
			m_list = m_list->next;
		}

		if (m_ord.empty())
			return;

		std::sort(
			m_ord.begin(), m_ord.end(),
			[](msg_info *p0, msg_info *p1) {
				return p0->ord < p1->ord;
			}
		);

		auto min_ord(m_ord.front()->ord);
		while (m_list) {
			auto m(m_list);
			m_list = m_list->next;

			if (m->ord >= min_ord)
				m_ord.push_back(m);
			else
				::free(m);
		}

		if (m_ord.size() > count) {
			std::sort(
				m_ord.begin(), m_ord.end(),
				[](msg_info *p0, msg_info *p1) {
					return p0->ord < p1->ord;
				}
			);
		}

		if (m_ord.size() < count)
			count = m_ord.size();

		for (auto c(m_ord.size() - count); c < m_ord.size(); ++c)
			cons(m_ord[c]->thread_id, m_ord[c]->msg);

		for (auto m: m_ord)
			::free(m);
	}

private:
	struct msg_info {
		msg_info *next;
		pid_t thread_id;
		uint32_t ord;
		uint32_t msg_len;
		char msg[];
	};

	std::atomic<msg_info *> msgs = {nullptr};
	std::atomic<uint32_t> order = {0};
};

}
#endif

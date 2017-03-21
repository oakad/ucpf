/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_6EA2FB4801CFABF9EE19820089DA4742)
#define HPP_6EA2FB4801CFABF9EE19820089DA4742

#include <atomic>
#include <thread>
#include <condition_variable>

#include <sys/syscall.h>

#include "test.hpp"

namespace ucpf::yesod::test {

struct cb_snap_logger {
	cb_snap_logger(size_t buffer_size_ = 1 << 16)
	: buffer_size(buffer_size_),
	  buf(reinterpret_cast<char *>(::malloc(buffer_size)))
	{
		auto m(reinterpret_cast<msg_info *>(buf + 1));
		buf[0] = 0;
		m->thread_id = ::syscall(SYS_gettid);
		m->prev_pos = sizeof(msg_info) + 1;
		m->msg_len = 1;
	}

	~cb_snap_logger()
	{
		::free(buf);
	}

	template <typename ...Args>
	void log(char const *format, Args &&...args)
	{
		size_t msg_len(snprintf(
			nullptr, 0, format, std::forward<Args>(args)...
		) + 1);
		msg_info *m;
		offset_type s_low, s_high;
		std::tie(m, s_low, s_high) = alloc_slot(msg_len);
		snprintf(
			m->msg(), msg_len, format, std::forward<Args>(args)...
		);
		auto prev_slot(dirty_slots.load());
		while (true) {
			auto d_low(slot_low(prev_slot));
			auto d_high(slot_high(prev_slot));
			if (s_low != d_low) {
				std::this_thread::yield();
				prev_slot = dirty_slots.load();
				continue;
			}
			auto next_slot(define_slot(s_high, d_high));
			if (dirty_slots.compare_exchange_weak(
				prev_slot, next_slot
			)) {
				ready_slot.store(define_slot(s_low, s_high));
				return;
			}
		}
	}

	template <typename Consumer>
	void dump(Consumer &&cons, size_t count)
	{
		if (!count)
			return;

		dirty_slots |= pause_flag;
		auto s(ready_slot.load());
		auto m(reinterpret_cast<msg_info *>(
			buf + slot_high(s) - sizeof(msg_info)
		));

		dump_impl(
			std::forward<Consumer>(cons), slot_high(s), m, count
		);

		lock.lock();
		dirty_slots &= ~pause_flag;
		lock.unlock();
		pause_cv.notify_all();
	}

	void dump_hex()
	{
		print_hex(std::cout, buf, buffer_size);
	}

private:
	typedef uint32_t offset_type;
	typedef uint64_t slot_type;
	constexpr static size_t offset_bits = 30;
	constexpr static uint32_t offset_mask = (
		uint32_t(1) << offset_bits
	) - 1;
	constexpr static slot_type pause_flag = slot_type(1) << 63;

	struct msg_info {
		pid_t thread_id;
		offset_type prev_pos;
		offset_type msg_len;

		char *msg()
		{
			return reinterpret_cast<char *>(this) - msg_len;
		}

		msg_info *prev_slot(
			char *buf, offset_type cur, offset_type bound
		)
		{
			auto p_info(reinterpret_cast<msg_info *>(
				buf + prev_pos - sizeof(msg_info)
			));

			if (is_rolled_over_back(
				cur, prev_pos - sizeof(msg_info), bound
			))
				return nullptr;

			if (is_rolled_over_back(
				cur, p_info->prev_pos, bound
			))
				return nullptr;

			return p_info;
		}
	};

	template <typename Consumer>
	void dump_impl(
		Consumer &&cons, offset_type cur, msg_info *m, size_t count
	)
	{
		auto prev(m->prev_slot(
			buf, cur, slot_high(dirty_slots.load())
		));

		if (prev && (count > 1))
			dump_impl(
				std::forward<Consumer>(cons),
				m->prev_pos, prev, count - 1
			);

		cons(m->thread_id, m->msg());
	}

	constexpr static bool is_rolled_over_back(
		offset_type cur, offset_type prev, offset_type bound
	)
	{
		auto f0(bound < cur);
		auto f1(cur < prev);
		if (f0 != f1)
			return prev < bound;
		else
			return f0;
	}

	constexpr static bool is_rolled_over_fore(
		offset_type cur, offset_type next, offset_type bound
	)
	{
		auto f0(bound >= cur);
		auto f1(cur >= next);
		if (f0 != f1)
			return next > bound;
		else
			return f0;
	}

	constexpr static offset_type slot_low(slot_type s)
	{
		return uint32_t(s) & offset_mask;
	}

	constexpr static offset_type slot_high(slot_type s)
	{
		return uint32_t(s >> offset_bits) & offset_mask;
	}

	constexpr static slot_type define_slot(
		offset_type low, offset_type high
	)
	{
		return (slot_type(high) << offset_bits) | low;
	}

	std::tuple<msg_info *, offset_type, offset_type> alloc_slot(
		offset_type msg_len
	)
	{
		auto prev_slot(dirty_slots.load());
		auto slot_len(msg_len + sizeof(msg_info));
		if (slot_len > (buffer_size >> 2))
			throw std::length_error("message too long");

		while (true) {
			if (prev_slot & pause_flag) {
				std::unique_lock<std::mutex> lk(lock);
				pause_cv.wait(lk, [this]() {
					return !(
						dirty_slots.load()
						& pause_flag
					);
				});
				prev_slot = dirty_slots.load();
				continue;
			}

			auto bound(ready_slot.load());

			auto d_low(slot_low(prev_slot));
			auto d_high(slot_high(prev_slot));

			auto s_high(d_high + slot_len);

			if (s_high > buffer_size)
				s_high = slot_len;

			if (is_rolled_over_fore(
				d_high, s_high, slot_low(bound)
			)) {
				std::this_thread::yield();
				continue;
			}

			if (dirty_slots.compare_exchange_weak(
				prev_slot, define_slot(d_low, s_high)
			)) {
				auto m(reinterpret_cast<msg_info *>(
					buf + s_high - sizeof(msg_info)
				));
				m->thread_id = ::syscall(SYS_gettid);
				m->prev_pos = d_high;
				m->msg_len = msg_len;
				return std::make_tuple(m, d_high, s_high);
			}
		}
	}

	size_t const buffer_size;
	std::atomic<slot_type> dirty_slots = {
		define_slot(sizeof(msg_info) + 1, sizeof(msg_info) + 1)
	};
	std::atomic<slot_type> ready_slot = {
		define_slot(0, sizeof(msg_info) + 1)
	};
	std::condition_variable pause_cv;
	std::mutex lock;
	char *buf;
};

}
#endif

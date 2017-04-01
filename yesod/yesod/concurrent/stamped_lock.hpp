/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
/*=============================================================================
    Based on algorithm and original implementation of
    java.util.concurrent.locks.StampedLock:

    This file is available under and governed by the GNU General Public
    License version 2 only, as published by the Free Software Foundation.
    However, the following notice accompanied the original version of this
    file:

    Written by Doug Lea with assistance from members of JCP JSR-166
    Expert Group and released to the public domain, as explained at
    http://creativecommons.org/publicdomain/zero/1.0/
==============================================================================*/
#if !defined(HPP_8C0051C73519507BBF0E53B13EFD527F)
#define HPP_8C0051C73519507BBF0E53B13EFD527F

#include <yesod/concurrent/thread.hpp>
#include <experimental/optional>

namespace std {

using experimental::optional;
using experimental::make_optional;

}

namespace ucpf::yesod::concurrent {

struct stamped_lock_config {
	virtual int spins_on_acquire() const = 0;

	virtual int spins_on_head() const = 0;

	virtual int max_spins_on_head() const = 0;

	virtual unsigned int concurrency() const = 0;

	virtual void yield_on_conflict() const = 0;
};

namespace detail {

template <typename Unused = void>
struct default_stamped_lock_config : stamped_lock_config {
	default_stamped_lock_config()
	: concurrency_value(
		std::thread::hardware_concurrency()
	), spins_on_acquire_value(
		concurrency_value > 1 ? 64 : 0
	), spins_on_head_value(
		concurrency_value > 1 ? 1024 : 0
	), max_spins_on_head_value(
		concurrency_value > 1 ? 65536 : 0
	)
	{}

	int spins_on_acquire() const override
	{
		return spins_on_acquire_value;
	}

	int spins_on_head() const override
	{
		return spins_on_head_value;
	}

	int max_spins_on_head() const override
	{
		return max_spins_on_head_value;
	}

	unsigned int concurrency() const override
	{
		return concurrency_value;
	}

	void yield_on_conflict() const override
	{
		auto seed(this_thread::get().next_seed());

		if (!(seed & 0x7f))
			std::this_thread::yield();
	}

	static default_stamped_lock_config instance;

private:
	unsigned int concurrency_value;
	int spins_on_acquire_value;
	int spins_on_head_value;
	int max_spins_on_head_value;
};

template <typename Unused>
default_stamped_lock_config<Unused>
default_stamped_lock_config<Unused>::instance;

}

struct stamped_lock {
	typedef uint64_t state_type;

	stamped_lock(stamped_lock &) = delete;
	stamped_lock(stamped_lock &&) = delete;
	stamped_lock(stamped_lock const &) = delete;
	stamped_lock(stamped_lock const &&) = delete;

	stamped_lock(
		stamped_lock_config const &config_
		= detail::default_stamped_lock_config<>::instance
	)
	: q_head{nullptr}, q_tail{nullptr},
	  state{state_type(1) << state_version_shift},
	  shared_count_overflow{0}, config(config_)
	{}

	~stamped_lock()
	{
	}

	template <typename Allocator = std::allocator<void>>
	state_type lock(Allocator const &alloc = Allocator())
	{
		auto s(try_lock());
		return s ? s : acquire_exclusive([this, &alloc]() {
			return exclusive_wait_node<Allocator>::create(
				this, false, alloc
			);
		}).value();
	}

	state_type try_lock()
	{
		auto s(state.load());
		return (s & state_access_level_mask) ? 0 : try_lock(s);
	}

	template <
		typename Rep, typename Period,
		typename Allocator = std::allocator<void>
	>
	std::optional<state_type> try_lock_for(
		std::chrono::duration<Rep, Period> const &rel_time,
		Allocator const &alloc = Allocator()
	)
	{
		if (this_thread::get().is_interrupted())
			return {};

		auto next(try_lock());
		if (next)
			return next;

		
		return acquire_exclusive([this, rel_time, &alloc]() {
			return exclusive_timed_wait_node<Allocator>::create(
				this, true, rel_time, alloc
			);
		});
	}

	template <typename Allocator = std::allocator<void>>
	std::optional<state_type> lock_interruptibly(
		Allocator const &alloc = Allocator()
	)
	{
		if (this_thread::get().is_interrupted())
			return {};

		return acquire_exclusive([this, &alloc]() {
			return exclusive_wait_node<Allocator>::create(
				this, true, alloc
			);
		});
	}

	template <typename Allocator = std::allocator<void>>
	state_type lock_shared(
		Allocator const &alloc = Allocator()
	)
	{
		auto s(state.load());
		auto next(s + 1);

		return ((
			q_head.load() == q_tail.load()
		) && (
			(s & state_access_level_mask) < state_shared_count_max
		) && state.compare_exchange_strong(s, next))
		? next : acquire_shared([this, &alloc]() {
			return shared_wait_node<Allocator>::create(
				this, false, alloc
			);
		}).value();
	}

	state_type try_lock_shared()
	{
		auto s(state.load());
		while (true) {
			if ((
				s & state_access_level_mask
			) == state_exclusive_mask)
				return 0;

			if (shared_count_of(s) < state_shared_count_max) {
				auto next(s + 1);
				if (state.compare_exchange_strong(
					s, next
				))
					return next;
				else
					continue;
			}

			auto next(try_inc_shared_overflow(s));
			if (next)
				return next;
			else
				s = state.load();
		}
	}

	template <
		typename Rep, typename Period,
		typename Allocator = std::allocator<void>
	>
	std::optional<state_type> try_lock_shared_for(
		std::chrono::duration<Rep, Period> const &rel_time,
		Allocator const &alloc = Allocator()
	)
	{
		if (this_thread::get().is_interrupted())
			return {};

		auto s(state.load());

		if ((
			s & state_access_level_mask
		) != state_exclusive_mask) {
			if (shared_count_of(s) < state_shared_count_max) {
				auto next(s + 1);
				if (state.compare_exchange_strong(s, next))
					return next;
			} else {
				auto next(try_inc_shared_overflow(s));
				if (next)
					return next;
			}
		}
		return acquire_shared([this, rel_time, &alloc]() {
			return shared_timed_wait_node<Allocator>::create(
				this, true, rel_time, alloc
			);
		});
	}

	template <typename Allocator = std::allocator<void>>
	std::optional<state_type> lock_shared_interruptibly(
		Allocator const &alloc = Allocator()
	)
	{
		if (this_thread::get().is_interrupted())
			return {};

		auto s(state.load());
		auto next(s + 1);

		return ((
			q_head.load() == q_tail.load()
		) && (
			(s & state_access_level_mask) < state_shared_count_max
		) && state.compare_exchange_strong(s, next))
		? next : acquire_shared([this, &alloc]() {
			return shared_wait_node<Allocator>::create(
				this, true, alloc
			);
		});
	}

	state_type try_access_shared()
	{
		auto s(state.load());
		return (s & state_exclusive_mask) ? 0 : stamp_of(s);
	}

	bool validate(state_type stamp)
	{
		return stamp_of(stamp) == stamp_of(state.load());
	}

	void unlock(state_type stamp)
	{
		if (
			(state.load() != stamp)
			|| !(stamp & state_exclusive_mask)
		)
			throw std::logic_error("lock not exclusive");

		unlock_internal(stamp);
	}

	void unlock_shared(state_type stamp)
	{
		auto s(state.load());
		while (true) {
			auto c(shared_count_of(s));
			if ((
				stamp_of(s) != stamp_of(stamp)
			) || !shared_count_of(stamp) || !c)
				throw std::logic_error("lock not shared");

			if (c < state_shared_count_max) {
				if (state.compare_exchange_weak(
					s, c - 1
				)) {
					if (c == 1)
						release_head();

					break;
				} else
					continue;
			}

			if (try_dec_shared_overflow(s))
				break;

			s = state.load();
		}
	}

	void unlock_any(state_type stamp)
	{
		if (stamp & state_exclusive_mask)
			unlock(stamp);
		else
			unlock_shared(stamp);
	}

	state_type try_convert_to_exclusive(state_type stamp)
	{
		auto s(state.load());
		while (stamp_of(s) == stamp_of(stamp)) {
			auto m(access_level_of(s));

			if (!m) {
				if (access_level_of(stamp))
					break;

				auto next(try_lock(s));
				if (next)
					return next;
			} else if (s & state_exclusive_mask) {
				if (access_level_of(stamp) == m)
					return stamp;
				else
					break;
			} else if ((m == 1) && access_level_of(stamp)) {
				auto next((s - 1) | state_exclusive_mask);
				if (state.compare_exchange_weak(s, next))
					return next;
			} else
				break;
		}

		return 0;
	}

	state_type try_convert_to_shared(state_type stamp)
	{
		auto s(state.load());

		while (stamp_of(s) == stamp_of(stamp)) {
			if (access_level_of(stamp) >= state_exclusive_mask) {
				if (s != stamp)
					break;

				auto next(unlock_state(s) + 1);
				state.store(next);
				release_head();
				return next;
			} else if (!access_level_of(stamp)) {
				if (access_level_of(s) < state_shared_count_max) {
					auto next(s + 1);
					if (state.compare_exchange_weak(s, next))
						return next;
				} else {
					auto next(try_inc_shared_overflow(s));
					if (next)
						return next;
					else
						s = state.load();
				}

			} else {
				if (!(access_level_of(s)))
					break;
				return stamp;
			}
		}

		return 0;
	}

	state_type try_convert_to_shared_access(state_type stamp)
	{
		auto s(state.load());
		while (stamp_of(s) == stamp_of(stamp)) {
			auto m(access_level_of(s));

			if (access_level_of(stamp) >= state_exclusive_mask) {
				if (s != stamp)
					break;

				return unlock_internal(s);
			} else if (!access_level_of(stamp))
				return stamp;
			else if (!m)
				break;
			else if (m < state_shared_count_max) {
				auto next(s - 1);
				if (state.compare_exchange_weak(s, next)) {
					if (m == 1)
						release_head();
					return stamp_of(next);
				} else
					s = state.load();
			} else {
				auto next(try_dec_shared_overflow(s));
				if (next)
					return stamp_of(next);
				else
					s = state.load();
			}
		}
		return 0;
	}

	bool try_unlock()
	{
		auto s(state.load());
		if (s & state_exclusive_mask) {
			unlock_internal(s);
			return true;
		}
		return false;
	}

	bool try_unlock_shared()
	{
		auto s(state.load());
		auto m(access_level_of(s));

		while (m && (m < state_exclusive_mask)) {
			if (m < state_shared_count_max) {
				if (state.compare_exchange_weak(s, s - 1)) {
					if (m == 1)
						release_head();

					return true;
				}
			} else {
				if (try_dec_shared_overflow(s))
					return true;
				else
					s = state.load();
			}

			m = access_level_of(s);
		}
		return false;
	}

	size_t get_shared_count() const
	{
		auto c(shared_count_of(state.load()));
		if (c < state_shared_count_max)
			return c;
		else
			return c + shared_count_overflow.load();
	}

	bool is_exclusive() const
	{
		return state.load() & state_exclusive_mask;
	}

	bool is_shared() const
	{
		return shared_count_of(state.load());
	}

private:
	constexpr static size_t state_exclusive_shift = 7;
	constexpr static size_t state_version_shift
	= state_exclusive_shift + 1;

	constexpr static state_type state_shared_count_mask
	= (state_type(1) << state_exclusive_shift) - 1;

	constexpr static state_type state_shared_count_max
	= state_shared_count_mask - 1;

	constexpr static state_type state_exclusive_mask
	= state_type(1) << state_exclusive_shift;

	constexpr static state_type state_access_level_mask
	= (state_type(1) << state_version_shift) - 1;

	constexpr static state_type state_stamp_mask
	= ~state_shared_count_mask;

	constexpr static uint32_t overflow_yield_rate = (1 << 3) - 1;

	constexpr static state_type shared_count_of(state_type s)
	{
		return s & state_shared_count_mask;
	}

	constexpr static state_type access_level_of(state_type s)
	{
		return s & state_access_level_mask;
	}

	constexpr static state_type stamp_of(state_type s)
	{
		return s & state_stamp_mask;
	}

	struct wait_node_base {
		wait_node_base(wait_node_base &) = delete;
		wait_node_base(wait_node_base &&) = delete;
		wait_node_base(wait_node_base const &) = delete;
		wait_node_base(wait_node_base const &&) = delete;

		wait_node_base(
			stamped_lock const *owner_, bool interruptible_
		)
		{}
	};

	template <typename Allocator>
	struct exclusive_wait_node final : wait_node_base {
		static exclusive_wait_node *create(
			stamped_lock const *owner_, bool interruptible_,
			Allocator const &alloc_
		)
		{
			return nullptr;
		}
	};

	template <typename Allocator>
	struct exclusive_timed_wait_node final : wait_node_base {
		static exclusive_timed_wait_node *create(
			stamped_lock const *owner_, bool interruptible_,
			std::chrono::nanoseconds wait_time_,
			Allocator const &alloc_
		)
		{
			return nullptr;
		}
	};

	template <typename Allocator>
	struct shared_wait_node final : wait_node_base {
		static shared_wait_node *create(
			stamped_lock const *owner_, bool interruptible_,
			Allocator const &alloc_
		)
		{
			return nullptr;
		}
	};

	template <typename Allocator>
	struct shared_timed_wait_node final : wait_node_base {
		static shared_timed_wait_node *create(
			stamped_lock const *owner_, bool interruptible_,
			std::chrono::nanoseconds wait_time_,
			Allocator const &alloc_
		)
		{
			return nullptr;
		}
	};

	state_type try_lock(state_type &s)
	{
		auto next(s | state_exclusive_mask);
		return state.compare_exchange_weak(s, next) ? next : 0;
	}

	static state_type unlock_state(state_type s)
	{
		s += state_exclusive_mask;
		return s ? s : (state_type(1) << state_version_shift);
	}

	state_type unlock_internal(state_type s)
	{
		auto next(unlock_state(s));
		state.store(next);
		release_head();
		return next;
	}

	state_type try_inc_shared_overflow(state_type s)
	{
		if (access_level_of(s) == state_shared_count_max) {
			auto next(s | state_shared_count_mask);
			if (state.compare_exchange_strong(s, next)) {
				++shared_count_overflow;
				state.store(s);
				return s;
			}
		} else if (!(
			this_thread::get().next_seed() & overflow_yield_rate
		))
			std::this_thread::yield();

		return 0;
	}

	state_type try_dec_shared_overflow(state_type s)
	{
		if (access_level_of(s) == state_shared_count_max) {
			auto next(s | state_shared_count_mask);
			if (state.compare_exchange_strong(s, next)) {
				next = s;
				if (shared_count_overflow.load())
					--shared_count_overflow;
				else
					--next;

				state.store(next);
				return next;
			}
		} else if (!(
			this_thread::get().next_seed() & overflow_yield_rate
		))
			std::this_thread::yield();

		return 0;
	}

	void release_head()
	{

	}

	std::optional<state_type> try_lock_repeated()
	{
		auto s(state.load());
		auto m(access_level_of(s));

		if (!m) {
			auto ns(try_lock(s));
			if (ns)
				return std::make_optional(ns);
		}

		if ((
			m != state_exclusive_mask
		) || (w_tail.load() != w_head.load()))
			return {};

		auto s = state.load();

		for (
			auto spins(config.spins_on_acquire());
			spins; --spins
		) {
			m = access_level_of(s);
			if (!m) {
				auto ns(try_lock(s));
				if (ns)
					return std::make_optional(ns);
			} else
				s = state.load();
		}
		return {};
	}

	std::optional<state_type> enqueue_tail_exclusive(wait_node_base *node)
	{
		auto s(state.load());
		auto p(q_tail.load());
		while (true) {
			auto m(access_level_of(s));
			if (!m) {
				auto ns(try_lock(s));
				if (ns != 0L)
					return std::make_optional(ns);
			} else if (node->prev != p) {
				node->prev = p;
				s = state.load();
				p = q_tail.load();
			} else if (q_tail.compare_exchange_weak(p, node)) {
				if (p)
					p->next = node;
				return {};
			}
		}
	}

	template <typename WaitNodeSupplier>
	std::optional<state_type> acquire_exclusive(
		WaitNodeSupplier &&w_supp
	)
	{
		auto rv(try_lock_repeated());
		if (rv)
			return rv;

		auto node(w_supp());
		rv = enqueue_tail_exclusive(node);
		if (rv) {
			node->release();
			return rv;
		}

		return {};
	}

	template <typename WaitNodeSupplier>
	std::optional<state_type> acquire_shared(
		WaitNodeSupplier &&w_supp
	)
	{
		return {};
	}

	std::atomic<wait_node_base *> q_head;
	std::atomic<wait_node_base *> q_tail;
	std::atomic<state_type> state;
	std::atomic<size_t> shared_count_overflow;
	stamped_lock_config const &config;
};

}
#endif

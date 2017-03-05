/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
/*=============================================================================
	Based on algorithm and original implementation of
	[openjdk]java.util.concurrent.Phaser:

	This file is available under and governed by the GNU General Public
	License version 2 only, as published by the Free Software Foundation.
	However, the following notice accompanied the original version of this
	file:

	Written by Doug Lea with assistance from members of JCP JSR-166
	Expert Group and released to the public domain, as explained at
	http://creativecommons.org/publicdomain/zero/1.0/
==============================================================================*/
#if !defined(HPP_48A7A9D0B6947A5D6DFB799C56332D38)
#define HPP_48A7A9D0B6947A5D6DFB799C56332D38

#include <yesod/concurrent/thread.hpp>

namespace ucpf::yesod::concurrent {

struct phaser {
	typedef uint64_t state_type;
	typedef uint32_t phase_type;
	typedef uint16_t count_type;

	struct notification {
		virtual bool on_advance(
			phase_type phase, count_type registered_parties
		) = 0;
	};

	template <typename Allocator = std::allocator<void>>
	phaser(
		count_type parties = 0, phaser *parent_ = nullptr,
		notification *notifier_ = nullptr,
		Allocator const &alloc_ = Allocator()
	)
	: parent(parent_), root(parent ? parent->root : this),
	  even_q_head(nullptr), odd_q_head(nullptr),
	  notifier(notifier_)
	{
		phase_type phase(0);

		if (parent && parties)
			phase = parent->do_register(1, alloc_);

		state.store(parties ? define_state(
			phase, parties, parties
		) : define_state(0, 0, 1));
	}

	template <typename Allocator = std::allocator<void>>
	phase_type register_one(Allocator const &alloc_ = Allocator())
	{
		return do_register(1, alloc_);
	}

	template <typename Allocator = std::allocator<void>>
	phase_type register_some(
		count_type parties, Allocator const &alloc_ = Allocator()
	)
	{
		return parties ? do_register(parties, alloc_) : get_phase();
	}

	phase_type arrive()
	{
		return do_arrive(define_state(0, 0, 1));
	}

	phase_type arrive_and_deregister()
	{
		return do_arrive(define_state(0, 1, 1));
	}

	template <typename Allocator = std::allocator<void>>
	phase_type arrive_and_await_advance(
		Allocator const &alloc = Allocator()
	)
	{
		auto s(state.load());
		state_type next;
		phase_type phase;
		count_type unarrived;

		do {
			if (parent)
				s = reconcile_state(s);

			phase = phase_of(s);
			if (termination_flag_set(s))
				return phase;

			unarrived = unarrived_of(s);
			if (!unarrived)
				throw std::logic_error(
					"unregistered party arrival"
				);
			next = s - 1;
		} while (!state.compare_exchange_weak(s, next));

		s = next;

		if (unarrived > 1)
			return internal_await_advance_default(phase, alloc);

		if (root != this)
			return parent->arrive_and_await_advance();

		auto next_unarrived(parties_of(s));
		auto next_phase(phase + 1);

		if (on_advance(phase, next_unarrived))
			next = define_state(
				next_phase, next_unarrived, 0
			) | phaser_terminated_mask;
		else if (!next_unarrived)
			next = define_state(next_phase, 0, 1);
		else
			next = define_state(
				next_phase, next_unarrived, next_unarrived
			);

		if (!state.compare_exchange_strong(s, next))
			return phase_of(s);

		release_waiters(phase);
		return next_phase;
	}

	template <typename Allocator = std::allocator<void>>
	phase_type await_advance(
		phase_type phase_in, Allocator const &alloc = Allocator()
	)
	{
		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_of(s));

		if (phase_out == phase_in)
			return internal_await_advance_default(phase_in, alloc);

		return phase_out;
	}

	template <typename Allocator = std::allocator<void>>
	phase_type await_advance_interruptibly(
		phase_type phase_in, Allocator const &alloc = Allocator()
	)
	{
		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_of(s));

		if (phase_out == phase_in)
			return root->internal_await_advance_interruptibly(
				phase_in, alloc
			);

		return phase_out;
	}

	template <
		typename Rep, typename Period,
		typename Allocator = std::allocator<void>
	>
	phase_type await_advance_for(
		phase_type phase_in,
		std::chrono::duration<Rep, Period> const &rel_time,
		Allocator const &alloc = Allocator()
	)
	{
		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_of(s));

		if (phase_out == phase_in)
			return internal_await_advance_timed(
				phase_in, rel_time, alloc
			);

		return phase_out;
	}

	void force_termination()
	{
		auto s(root->state.load());
		if (termination_flag_set(s))
			return;

		while (!root->state.compare_exchange_weak(
			s, s | phaser_terminated_mask
		)) {}

		release_waiters(0);
		release_waiters(1);
	}

	phase_type get_phase() const
	{
		return phase_of(state.load());
	}

	count_type get_registered_parties() const
	{
		return parties_of(state.load());
	}

	count_type get_arrived_parties()
	{
		auto s(state.load());
		return arrived_of(parent ? reconcile_state(s) : s);
	}

	count_type get_unarrived_parties()
	{
		auto s(state.load());
		return unarrived_of(parent ? reconcile_state(s) : s);
	}

	phaser *get_parent() const
	{
		return parent;
	}

	phaser *get_root() const
	{
		return root;
	}

	bool is_terminated() const
	{
		return termination_flag_set(root->state.load());
	}

private:
	constexpr static size_t state_bits = 64;
	constexpr static size_t state_phase_bits = 31;
	constexpr static size_t state_parties_bits = 16;
	constexpr static size_t state_pending_bits = 16;

	constexpr static size_t state_phase_shift = 32;
	constexpr static size_t state_parties_shift = 16;
	
	constexpr static state_type phaser_terminated_mask
	= state_type(1) << (state_bits - 1);

	constexpr static state_type state_phase_increment
	= state_type(1) << state_phase_shift;
	constexpr static state_type state_phase_mask
	= ((state_type(1) << state_phase_bits) - 1) << state_phase_shift;

	constexpr static state_type state_parties_mask
	= ((state_type(1) << state_parties_bits) - 1) << state_parties_shift;

	constexpr static state_type state_pending_mask
	= (state_type(1) << state_pending_bits) - 1;

	struct wait_node_base {
		wait_node_base(
			phaser &owner_, phase_type phase_, bool interruptible
		)
		: owner(owner_), next(nullptr),
		  tdata(thread::current_thread_data()),
		  flags{FLAG_IN_USE | (interruptible ? FLAG_INTERRUPTIBLE : 0)},
		  phase(phase_)
		{}

		virtual ~wait_node_base() = default;

		virtual bool is_releasable() = 0;

		virtual void block() = 0;

		virtual void release() = 0;

		virtual void queue_release() = 0;

		enum : uint32_t {
			FLAG_IN_USE = 1,
			FLAG_QUEUED = 2,
			FLAG_WAS_INTERRUPTED = 4,
			FLAG_INTERRUPTIBLE = 8
		};

		constexpr static uint32_t release_mask
		= FLAG_IN_USE | FLAG_QUEUED;

		phaser &owner;
		wait_node_base *next;
		std::atomic<detail::thread_data *> tdata;
		std::atomic<uint32_t> flags;
		phase_type const phase;
	};

	template <typename Allocator>
	struct wait_node final : wait_node_base
	{
		static wait_node *create(
			phaser &owner_, phase_type phase_, bool interruptible,
			Allocator const &alloc
		)
		{
			return yesod::detail::allocated_storage<
				wait_node, Allocator
			>::make(
				alloc, owner_, phase_, interruptible
			)->get();
		}

		bool is_releasable() override
		{
			if (!tdata.load())
				return true;

			if (owner.get_phase() != phase) {
				tdata.store(nullptr);
				return true;
			}

			if (thread::current_thread_data()->is_interrupted(true))
				flags |= FLAG_WAS_INTERRUPTED;

			if (flags.load() & FLAG_WAS_INTERRUPTED) {
				tdata.store(nullptr);
				return true;
			}

			return false;
		}

		void block() override
		{
			while (!is_releasable())
				thread::current_thread_data()->park();
		}

		void release() override
		{
			if (!((flags &= ~FLAG_IN_USE) & release_mask))
				yesod::detail::allocated_storage<
					wait_node, Allocator
				>::to_storage_ptr(this)->destroy();
		}

		void queue_release() override
		{
			if (!((flags &= ~FLAG_QUEUED) & release_mask))
				yesod::detail::allocated_storage<
					wait_node, Allocator
				>::to_storage_ptr(this)->destroy();
		}

		wait_node(
			phaser &owner_, phase_type phase_, bool interruptible
		)
		: wait_node_base(owner_, phase_, interruptible)
		{}
	};

	template <typename Allocator>
	struct timed_wait_node final : wait_node_base
	{
		static wait_node_base *create(
			phaser &owner_, phase_type phase_, bool interruptible_,
			std::chrono::nanoseconds wait_time_,
			Allocator const &alloc
		)
		{
			return yesod::detail::allocated_storage<
				timed_wait_node, Allocator
			>::make(
				alloc, owner_, phase_, interruptible_,
				wait_time_
			)->get();
		}

		bool is_releasable() override
		{
			if (!tdata.load())
				return true;

			if (owner.get_phase() != phase) {
				tdata.store(nullptr);
				return true;
			}

			if (thread::current_thread_data()->is_interrupted(true))
				flags |= FLAG_WAS_INTERRUPTED;

			if (flags.load() & FLAG_WAS_INTERRUPTED) {
				tdata.store(nullptr);
				return true;
			}

			auto cur_time(std::chrono::steady_clock::now());
			auto elapsed(cur_time - start_time);
			if (elapsed < wait_time) {
				wait_time -= elapsed;
				start_time = cur_time;
				return false;
			} else {
				tdata.store(nullptr);
				return true;
			}
		}

		void block() override
		{
			while (!is_releasable())
				thread::current_thread_data()->park_for(
					wait_time
				);
		}

		void release() override
		{
			if (!((flags &= ~FLAG_IN_USE) & release_mask))
				yesod::detail::allocated_storage<
					timed_wait_node, Allocator
				>::to_storage_ptr(this)->destroy();
		}

		void queue_release() override
		{
			if (!((flags &= ~FLAG_QUEUED) & release_mask))
				yesod::detail::allocated_storage<
					timed_wait_node, Allocator
				>::to_storage_ptr(this)->destroy();
		}

		timed_wait_node(
			phaser &owner_, phase_type phase_, bool interruptible_,
			std::chrono::nanoseconds wait_time_
		)
		: wait_node_base(owner_, phase_, interruptible_),
		  wait_time(wait_time_),
		  start_time(std::chrono::steady_clock::now())
		{}

	private:
		std::chrono::nanoseconds wait_time;
		std::chrono::time_point<std::chrono::steady_clock> start_time;
	};

	static phase_type phase_of(state_type s)
	{
		return static_cast<phase_type>(
			(s & state_phase_mask) >> state_phase_shift
		);
	}

	static count_type parties_of(state_type s)
	{
		return static_cast<count_type>(
			(s & state_parties_mask) >> state_parties_shift
		);
	}

	static count_type pending_of(state_type s)
	{
		return static_cast<count_type>(s & state_pending_mask);
	}

	static count_type unarrived_of(state_type s)
	{
		if (parties_of(s))
			return pending_of(s);
		else
			return 0;
	}

	static count_type arrived_of(state_type s)
	{
		if (parties_of(s))
			return parties_of(s) - pending_of(s);
		else
			return 0;
	}

	static bool termination_flag_set(state_type s)
	{
		return s & phaser_terminated_mask;
	}

	phase_type do_arrive(state_type adjust)
	{
		auto s(state.load());
		state_type next;
		phase_type phase;
		count_type unarrived;

		do {
			if (parent)
				s = reconcile_state(s);

			phase = phase_of(s);
			if (termination_flag_set(s))
				return phase;

			unarrived = unarrived_of(s);
			if (!unarrived)
				throw std::logic_error(
					"unregistered party arrival"
				);

			next = s - adjust;
		} while (!state.compare_exchange_weak(s, next));

		if (unarrived > 1)
			return phase;

		s = next;
		auto next_unarrived(parties_of(s));
		auto next_phase(phase + 1);

		if (root == this) {
			if (on_advance(phase, next_unarrived))
				next = define_state(
					next_phase, next_unarrived, 0
				) | phaser_terminated_mask;
			else if (!next_unarrived)
				next = define_state(next_phase, 0, 1);
			else
				next = define_state(
					next_phase, next_unarrived,
					next_unarrived
				);

			if (!state.compare_exchange_strong(s, next))
				return phase_of(s);

			release_waiters(phase);
		} else if (!next_unarrived) {
			phase = parent->do_arrive(define_state(0, 1, 1));
			state.compare_exchange_strong(s, s | 1);
		} else
			phase = parent->do_arrive(define_state(0, 0, 1));

		return phase;
	}

	template <typename Allocator>
	phase_type do_register(
		count_type registrations, Allocator const &alloc
	)
	{
		auto adjust(define_state(0, registrations, registrations));
		state_type s(state.load());
		phase_type phase;

		while (true) {
			if (parent)
				s = reconcile_state(s);

			phase = phase_of(s);
			if (termination_flag_set(s))
				break;

			auto parties(parties_of(s));
			auto unarrived(pending_of(s));

			if ((registrations + parties) < parties)
				throw std::out_of_range(
					"too many registrations"
				);

			if (parties) {
				if (!parent || (reconcile_state(s) == s)) {
					if (!unarrived) {
						internal_await_advance_default(
							phase, alloc
						);
						s = state.load();
					} else if (state.compare_exchange_weak(
						s, s + adjust
					))
						break;
					else
						s = state.load();
				} else
					s = state.load();
			} else if (!parent) {
				if (state.compare_exchange_weak(s, define_state(
					phase, registrations, registrations
				)))
					break;
			} else {
				std::lock_guard<std::mutex> lock(self_lock);
				auto s_(state.load());
				if (s_ != s) {
					s = s_;
					continue;
				}

				phase = parent->do_register(1, alloc);
				if (parent->is_terminated())
					break;

				auto next(define_state(
					phase, registrations, registrations
				));

				while (!state.compare_exchange_weak(s, next)) {
					next = define_state(
						phase_of(s),
						registrations,
						registrations
					) | (s & phaser_terminated_mask);
				}

				break;
			}
		}
		return phase;
	}

	template <typename Allocator>
	phase_type internal_await_advance_default(
		phase_type phase_in, Allocator const &alloc
	)
	{
		return root->internal_await_advance<false>(
			phase_in, [this, &alloc](phase_type phase_) {
				return wait_node<Allocator>::create(
					*this, phase_, false, alloc
				);
			}
		);
	}

	template <typename Allocator>
	phase_type internal_await_advance_interruptibly(
		phase_type phase_in, Allocator const &alloc
	)
	{
		return root->internal_await_advance<true>(
			phase_in, [this, &alloc](phase_type phase_) {
				return wait_node<Allocator>::create(
					*this, phase_, true, alloc
				);
			}
		);
	}

	template <typename Allocator>
	phase_type internal_await_advance_timed(
		phase_type phase_in, std::chrono::nanoseconds rel_time,
		Allocator const &alloc
	)
	{
		return root->internal_await_advance<true>(
			phase_in, [this, rel_time, &alloc](phase_type phase) {
				return timed_wait_node<Allocator>::create(
					*this, phase, true, rel_time, alloc
				);
			}
		);
	}

	template <bool HasNode, typename WaitNodeSupplier>
	phase_type internal_await_advance(
		phase_type phase_in, WaitNodeSupplier &&w_supp
	)
	{
		auto const concurrency(thread::hardware_concurrency());
		auto const spins_per_arrival(concurrency > 1 ? 256 : 1);

		release_waiters(phase_in - 1);

		auto queued(false);
		phase_type phase_out;
		count_type last_unarrived(0);
		auto spins(spins_per_arrival);
		wait_node_base *node(
			HasNode ? w_supp(phase_in) : nullptr
		);

		while (true) {
			auto s(state.load());
			phase_out = phase_of(s);
			if ((phase_out != phase_in) || termination_flag_set(s))
				break;

			if (!node) {
				auto unarrived(pending_of(s));

				if (unarrived != last_unarrived) {
					last_unarrived = unarrived;
					if (last_unarrived < concurrency)
						spins += spins_per_arrival;
				}

				auto interrupted(
					thread::current_thread_data()
					->is_interrupted(true)
				);
				if (interrupted || (--spins < 0)) {
					node = w_supp(phase_in);
					node->flags
					|= wait_node_base::FLAG_WAS_INTERRUPTED;
				}
			} else if (node->is_releasable())
				break;
			else if (!queued)
				queued = enqueue_waiter(phase_in, node);
			else
				node->block();
		}

		if (node) {
			node->tdata.store(nullptr);

			auto f(node->flags.load());
			if (
				(f & wait_node_base::FLAG_WAS_INTERRUPTED)
				&& !(f & wait_node_base::FLAG_INTERRUPTIBLE)
			)
				thread::current_thread_data()->interrupt();

			node->release();
		}

		release_waiters(phase_in);
		return phase_out;
	}

	state_type reconcile_state(state_type s)
	{
		while (true) {
			auto r(root->state.load());

			auto root_phase(phase_of(r));
			auto self_phase(phase_of(s));

			if ((
				termination_flag_set(r)
				== termination_flag_set(s)
			) && (root_phase == self_phase))
				break;

			auto parties(parties_of(s));
			state_type next(define_state(
				root_phase, parties, parties ? parties : 1
			));
			if (termination_flag_set(r))
				next |= phaser_terminated_mask;

			if (state.compare_exchange_weak(s, next)) {
				s = next;
				break;
			}
		}

		return s;
	}

	bool enqueue_waiter(phase_type phase_in, wait_node_base *node)
	{
		auto &head(get_queue(phase_in));
		auto p(head.load());
		node->next = p;

		auto rv((
			phase_of(state.load()) == phase_in
		) ? head.compare_exchange_weak(p, node) : false);

		if (rv)
			node->flags |= wait_node_base::FLAG_QUEUED;

		return rv;
	}

	void release_waiters(phase_type phase_in)
	{
		auto p(get_queue(phase_in).exchange(nullptr));

		while (p) {
			auto t(p->tdata.exchange(nullptr));
			if (t)
				t->unpark();
			auto q(p->next);
			p->queue_release();
			p = q;
		}
	}


	constexpr static state_type define_state(
		phase_type phase, count_type parties, count_type pending
	)
	{
		auto rv((
			static_cast<state_type>(phase) << state_phase_shift
		) & state_phase_mask);
		rv |= static_cast<state_type>(parties) << state_parties_shift;
		rv |= static_cast<state_type>(pending);
		return rv;
	}

	std::atomic<wait_node_base *> &get_queue(phase_type phase)
	{
		return (phase & 1) ? root->odd_q_head : root->even_q_head;
	}

	bool on_advance(phase_type phase, count_type registered_parties) {
		if (notifier)
			return notifier->on_advance(
				phase, registered_parties
			);
		else
			return !registered_parties;
	}

	phaser *parent;
	phaser * const root;
	std::mutex self_lock;
	std::atomic<wait_node_base *> even_q_head;
	std::atomic<wait_node_base *> odd_q_head;
	std::atomic<state_type> state;
	notification *notifier;
};

}
#endif

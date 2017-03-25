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
namespace test {

struct phaser_probe;

}

struct phaser_config {
	virtual ~phaser_config() = default;

	enum class RELEASE_POLICY {
		WAKING_THREAD = 0,
		COOPERATIVE = 1
	};

	virtual RELEASE_POLICY waiter_release_policy() const = 0;

	virtual int spins_per_arrival() const = 0;

	virtual unsigned int concurrency() const = 0;

	virtual void yield_on_conflict() const = 0;
};

namespace detail {

template <typename Unused = void>
struct default_phaser_config : phaser_config {
	default_phaser_config()
	: concurrency_value(
		std::thread::hardware_concurrency()
	), spins_per_arrival_value(
		concurrency_value > 1 ? 256 : 1
	)
	{}

	RELEASE_POLICY waiter_release_policy() const override
	{
		return RELEASE_POLICY::COOPERATIVE;
	}

	int spins_per_arrival() const override
	{
		return spins_per_arrival_value;
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

	static default_phaser_config instance;

private:
	unsigned int concurrency_value;
	int spins_per_arrival_value;
};

template <typename Unused>
default_phaser_config<Unused> default_phaser_config<Unused>::instance;

}

struct phaser {
	typedef uint64_t state_type;
	typedef uint32_t phase_value_type;
	typedef uint16_t count_type;

	struct phase_type {
		phase_type()
		: state(0)
		{}

		phase_type(phase_value_type v)
		: state(state_type(v) << state_phase_shift)
		{}

		phase_value_type value() const
		{
			return phase_of(state);
		}

		bool terminal() const
		{
			return termination_flag_set(state);
		}

		phase_type &operator++()
		{
			auto v(state + (state_type(1) << state_phase_shift));
			v &= state_phase_mask;
			state &= ~state_phase_mask;
			state |= v;
			return *this;
		}

		phase_type operator+(phase_value_type other) const
		{
			auto v(state & state_phase_mask);
			v += state_type(other) << state_phase_shift;

			return make(v & state_phase_mask);
		}

		bool operator==(phase_type const &other) const
		{
			return state == other.state;
		}

		bool operator!=(phase_type const &other) const
		{
			return state != other.state;
		}

		enum CONDITION : int {
			NORMAL = 0,
			ERRONEOUS = 1,
			TIMED_OUT = 2,
			INTERRUPTED = 3
		};

		CONDITION condition()
		{
			auto rv(static_cast<CONDITION>(
				state & ~state_phase_type_mask
			));
			state &= state_phase_type_mask;
			return rv;
		}

		bool normal()
		{
			return condition() == CONDITION::NORMAL;
		}

		bool erroneous()
		{
			return condition() == CONDITION::ERRONEOUS;
		}

		bool timed_out()
		{
			return condition() == CONDITION::TIMED_OUT;
		}

		bool interrupted()
		{
			return condition() == CONDITION::INTERRUPTED;
		}

		state_type to_state(count_type parties, count_type pending)
		{
			auto s(state & state_phase_type_mask);
			s |= state_type(parties) << state_parties_shift;
			s |= pending;
			return s;
		}

	//private:
		friend struct phaser;

		static phase_type make(state_type state_)
		{
			phase_type rv;
			rv.state = state_ & state_phase_type_mask;
			return rv;
		}

		static phase_type make_err(state_type state_)
		{
			phase_type rv;
			rv.state = (
				state_ & state_phase_type_mask
			) + CONDITION::ERRONEOUS;
			return rv;
		}

		state_type state;
	};

	struct notification {
		virtual ~notification() = default;

		virtual bool on_advance(
			phaser &p, phase_type phase,
			count_type registered_parties
		) = 0;
	};

	phaser(phaser &) = delete;
	phaser(phaser &&) = delete;
	phaser(phaser const &) = delete;
	phaser(phaser const &&) = delete;

	template <typename Allocator = std::allocator<void>>
	phaser(
		count_type parties = 0, phaser *parent_ = nullptr,
		notification *notifier_ = nullptr,
		Allocator const &alloc_ = Allocator(),
		phaser_config const &config_
		= detail::default_phaser_config<>::instance
	)
	: parent(parent_), root(parent ? parent->root : this),
	  even_q_head(nullptr), odd_q_head(nullptr),
	  notifier(notifier_), config(config_),
	  release_waiters(select_release_waiters_method(
		config.waiter_release_policy()
	  )), invalid_ptr(select_invalid_ptr_method(
		config.waiter_release_policy()
	  ))
	{
		phase_type phase;

		if (parent && parties)
			phase = parent->do_register(1, alloc_);

		state.store(parties ? define_state(
			phase.value(), parties, parties
		) : define_state(0, 0, 1));
	}

	~phaser()
	{
		force_termination();
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
		phase_type phase_out;
		count_type unarrived;

		do {
			if (parent)
				s = reconcile_state(s);

			phase_out = phase_type::make(s);
			if (phase_out.terminal())
				return phase_out;

			unarrived = unarrived_of(s);
			if (!unarrived)
				return phase_type::make_err(s);

			next = s - 1;
		} while (!state.compare_exchange_weak(s, next));

		s = next;

		if (unarrived > 1)
			return root->internal_await_advance_default(
				phase_out, alloc
			);

		if (!parent) {
			auto next_unarrived(parties_of(s));
			auto next_phase(phase_out + 1);

			if (notifier.invoke_on_advance(
				*this, phase_out, next_unarrived
			))
				next = next_phase.to_state(
					next_unarrived, 0
				) | phaser_terminated_mask;
			else if (!next_unarrived)
				next = next_phase.to_state(0, 1);
			else
				next = next_phase.to_state(
					next_unarrived, next_unarrived
				);

			if (!state.compare_exchange_strong(s, next))
				return phase_type::make(s);

			(this->*release_waiters)(get_queue(phase_out));
			return phase_type::make(next);
		} else
			return parent->arrive_and_await_advance();
	}

	struct diag {
		state_type s_in;
		state_type s0;
		state_type s1;
	};

	template <typename Allocator = std::allocator<void>>
	phase_type await_advance(
		phase_type phase_in, Allocator const &alloc = Allocator()
	)
	{
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return root->internal_await_advance_default(
				phase_in, alloc
			);
		else
			return phase_out;
	}

	template <typename Allocator = std::allocator<void>>
	phase_type await_advance_interruptibly(
		phase_type phase_in, Allocator const &alloc = Allocator()
	)
	{
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return root->internal_await_advance_interruptibly(
				phase_in, alloc
			);
		else
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
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return root->internal_await_advance_timed(
				phase_in, rel_time, alloc
			);
		else
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

		(root->*release_waiters)(root->even_q_head);
		(root->*release_waiters)(root->odd_q_head);
	}

	phase_type get_phase() const
	{
		return phase_type::make(root->state.load());
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
	friend struct test::phaser_probe;

	constexpr static size_t state_bits = 64;
	constexpr static size_t state_phase_bits = 31;
	constexpr static size_t state_parties_bits = 16;
	constexpr static size_t state_pending_bits = 16;

	constexpr static size_t termination_bit_shift = state_bits - 1;
	constexpr static size_t state_phase_shift = 32;
	constexpr static size_t state_parties_shift = 16;

	constexpr static state_type state_phase_type_mask
	= ~state_type(0) << state_phase_shift;

	constexpr static state_type phaser_terminated_mask
	= state_type(1) << termination_bit_shift;

	constexpr static state_type state_phase_increment
	= state_type(1) << state_phase_shift;
	constexpr static state_type state_phase_mask
	= ((state_type(1) << state_phase_bits) - 1) << state_phase_shift;

	constexpr static state_type state_parties_mask
	= ((state_type(1) << state_parties_bits) - 1) << state_parties_shift;

	constexpr static state_type state_pending_mask
	= (state_type(1) << state_pending_bits) - 1;

	struct wait_node_base {
		wait_node_base(wait_node_base &) = delete;
		wait_node_base(wait_node_base &&) = delete;
		wait_node_base(wait_node_base const &) = delete;
		wait_node_base(wait_node_base const &&) = delete;

		wait_node_base(
			phaser const *root_, phase_type phase_,
			bool interruptible
		)
		: root(root_), next(nullptr),
		  flags{FLAG_IN_USE | (interruptible ? FLAG_INTERRUPTIBLE : 0)},
		  phase(phase_), waiter(this_thread::get())
		{}

		virtual ~wait_node_base()
		{}

		virtual bool is_releasable() = 0;

		virtual void enqueue_and_block(std::atomic<
			wait_node_base *
		> &q_head) = 0;

		virtual void destroy() = 0;

		void release()
		{
			if (!((flags &= ~FLAG_IN_USE) & release_mask))
				destroy();
		}

		void queue_release()
		{
			if (!((flags &= ~FLAG_QUEUED) & release_mask))
				destroy();
			else {
				if (waiter)
					waiter.unpark();
			}
		}

		bool is_queued()
		{
			return flags.load() & FLAG_QUEUED;
		}

		enum : uint32_t {
			FLAG_IN_USE = 1,
			FLAG_QUEUED = 2,
			FLAG_WAS_INTERRUPTED = 4,
			FLAG_INTERRUPTIBLE = 8
		};

		constexpr static uint32_t release_mask
		= FLAG_IN_USE | FLAG_QUEUED;

		constexpr static uint32_t interrupt_mask
		= FLAG_WAS_INTERRUPTED | FLAG_INTERRUPTIBLE;

		phaser const *root;
		wait_node_base *next;
		std::atomic<uint32_t> flags;
		phase_type const phase;
		thread waiter;
	};

	template <typename Allocator>
	struct wait_node final : wait_node_base
	{
		static wait_node *create(
			phaser const *root_, phase_type phase_,
			bool interruptible,
			Allocator const &alloc
		)
		{
			return yesod::detail::allocated_storage<
				wait_node, Allocator
			>::make(
				alloc, root_, phase_, interruptible
			)->get();
		}

		bool is_releasable() override
		{
			if (!waiter)
				return true;

			if (phase_type::make(root->state.load()) != phase) {
				waiter.release();
				return true;
			}

			if (this_thread::get().is_interrupted(true))
				flags |= FLAG_WAS_INTERRUPTED;

			if ((flags.load() & interrupt_mask) == interrupt_mask) {
				waiter.release();
				return true;
			}

			return false;
		}

		void enqueue_and_block(std::atomic<
			wait_node_base *
		> &q_head) override
		{
			try {
				this_thread::get().park([this, &q_head]() {
					auto p(q_head.load());

					while (!is_releasable()) {
						if ((root->*(
							root->invalid_ptr
						))(p)) {
							root->config.yield_on_conflict();
							p = q_head.load();
							continue;
						}

						next = p;
						if (q_head.compare_exchange_weak(
							p, this
						)) {
							flags |= wait_node_base::FLAG_QUEUED;
							return !is_releasable();
						}
					}
					return false;
				});
			} catch (__cxxabiv1::__forced_unwind const &) {
				waiter.release();
				throw;
			}
		}

		void destroy() override
		{
			yesod::detail::allocated_storage<
				wait_node, Allocator
			>::to_storage_ptr(this)->destroy();
		}

		wait_node(
			phaser const *root_, phase_type phase_,
			bool interruptible
		)
		: wait_node_base(root_, phase_, interruptible)
		{}
	};

	template <typename Allocator>
	struct timed_wait_node final : wait_node_base
	{
		static wait_node_base *create(
			phaser const *root_, phase_type phase_,
			bool interruptible_,
			std::chrono::nanoseconds wait_time_,
			Allocator const &alloc
		)
		{
			return yesod::detail::allocated_storage<
				timed_wait_node, Allocator
			>::make(
				alloc, root_, phase_, interruptible_,
				wait_time_
			)->get();
		}

		bool is_releasable() override
		{
			if (!waiter)
				return true;

			if (phase_type::make(root->state.load()) != phase) {
				waiter.release();
				return true;
			}

			if (this_thread::get().is_interrupted(true))
				flags |= FLAG_WAS_INTERRUPTED;

			if ((flags.load() & interrupt_mask) == interrupt_mask) {
				waiter.release();
				return true;
			}

			auto cur_time(std::chrono::steady_clock::now());
			auto elapsed(cur_time - start_time);

			if (elapsed < wait_time) {
				wait_time -= elapsed;
				start_time = cur_time;
				return false;
			} else {
				waiter.release();
				return true;
			}
		}

		void enqueue_and_block(std::atomic<
			wait_node_base *
		> &q_head) override
		{
			try {
				this_thread::get().park_for(wait_time, [this, &q_head]() {
					auto p(q_head.load());

					while (!is_releasable()) {
						if ((root->*(
							root->invalid_ptr
						))(p)) {
							root->config.yield_on_conflict();
							p = q_head.load();
							continue;
						}

						next = p;
						if (q_head.compare_exchange_weak(
							p, this
						)) {
							flags |= wait_node_base::FLAG_QUEUED;
							return !is_releasable();
						}
					}
					return false;
				});
			} catch (__cxxabiv1::__forced_unwind const &) {
				waiter.release();
				throw;
			}
		}

		void destroy() override
		{
			yesod::detail::allocated_storage<
				timed_wait_node, Allocator
			>::to_storage_ptr(this)->destroy();
		}

		timed_wait_node(
			phaser const *root_, phase_type phase_,
			bool interruptible_,
			std::chrono::nanoseconds wait_time_
		)
		: wait_node_base(root_, phase_, interruptible_),
		  wait_time(wait_time_),
		  start_time(std::chrono::steady_clock::now())
		{}

	private:
		std::chrono::nanoseconds wait_time;
		std::chrono::time_point<std::chrono::steady_clock> start_time;
	};

	constexpr static phase_value_type phase_of(state_type s)
	{
		return static_cast<phase_value_type>(
			(s & state_phase_mask) >> state_phase_shift
		);
	}

	constexpr static count_type parties_of(state_type s)
	{
		return static_cast<count_type>(
			(s & state_parties_mask) >> state_parties_shift
		);
	}

	constexpr static count_type pending_of(state_type s)
	{
		return static_cast<count_type>(s & state_pending_mask);
	}

	constexpr static count_type unarrived_of(state_type s)
	{
		return parties_of(s) ? pending_of(s) : 0;
	}

	constexpr static count_type arrived_of(state_type s)
	{
		return parties_of(s) ? (parties_of(s) - pending_of(s)) : 0;
	}

	constexpr static bool termination_flag_set(state_type s)
	{
		return s & phaser_terminated_mask;
	}

	phase_type do_arrive(state_type adjust)
	{
		auto s(state.load());
		state_type next;
		phase_type phase_out;
		count_type unarrived;

		do {
			if (parent)
				s = reconcile_state(s);

			phase_out = phase_type::make(s);
			if (phase_out.terminal())
				return phase_out;

			unarrived = unarrived_of(s);
			if (!unarrived)
				return phase_type::make_err(s);

			next = s - adjust;
		} while (!state.compare_exchange_weak(s, next));

		phase_out = phase_type::make(next);
		if (unarrived != 1)
			return phase_out;

		s = next;
		auto next_unarrived(parties_of(s));
		auto next_phase(phase_out + 1);

		if (!parent) {
			if (notifier.invoke_on_advance(
				*this, phase_out, next_unarrived
			))
				next = next_phase.to_state(
					next_unarrived, 0
				) | phaser_terminated_mask;
			else if (!next_unarrived)
				next = next_phase.to_state(0, 1);
			else
				next = next_phase.to_state(
					next_unarrived, next_unarrived
				);

			state.compare_exchange_strong(s, next);
			(this->*release_waiters)(get_queue(phase_out));
		} else if (!next_unarrived) {
			phase_out = parent->do_arrive(define_state(0, 1, 1));
			state.compare_exchange_strong(
				s, define_state(phase_of(s), 0, 1)
			);
		} else
			phase_out = parent->do_arrive(define_state(0, 0, 1));

		return phase_out;
	}

	template <typename Allocator>
	phase_type do_register(
		count_type registrations, Allocator const &alloc
	)
	{
		state_type s(state.load());
		phase_type phase_out;

		while (true) {
			if (parent)
				s = reconcile_state(s);

			phase_out = phase_type::make(s);
			if (phase_out.terminal())
				break;

			auto parties(parties_of(s));
			auto unarrived(pending_of(s));

			if (count_type(registrations + parties) < parties)
				return phase_type::make_err(s);

			if (parties || !unarrived) {
				if (!parent || (reconcile_state(s) == s)) {
					if (!unarrived) {
						root->internal_await_advance_default(
							phase_out, alloc
						);
						s = state.load();
					} else if (state.compare_exchange_weak(
						s, phase_out.to_state(
							parties + registrations,
							unarrived + registrations
						)
					))
						break;
				} else
					s = state.load();
			} else if (!parent) {
				if (state.compare_exchange_weak(
					s, phase_out.to_state(
						registrations, registrations
					)
				))
					break;
			} else {
				std::lock_guard<std::mutex> lock(self_lock);
				auto s_(state.load());
				if (s_ != s) {
					s = s_;
					continue;
				}

				phase_out = parent->do_register(1, alloc);
				if (phase_out.terminal())
					break;

				auto next(phase_out.to_state(
					registrations, registrations
				));

				while (!state.compare_exchange_weak(s, next)) {
					phase_out = phase_type::make(
						root->state.load()
					);
					next = phase_out.to_state(
						registrations, registrations
					);
				}
				break;
			}
		}

		return phase_out;
	}

	template <typename Allocator>
	phase_type internal_await_advance_default(
		phase_type phase_in, Allocator const &alloc
	)
	{
		return internal_await_advance<false>(
			phase_in, [this, &alloc](phase_type phase_) {
				return wait_node<Allocator>::create(
					this, phase_, false, alloc
				);
			}
		);
	}

	template <typename Allocator>
	phase_type internal_await_advance_interruptibly(
		phase_type phase_in, Allocator const &alloc
	)
	{
		return internal_await_advance<true>(
			phase_in, [this, &alloc](phase_type phase_) {
				return wait_node<Allocator>::create(
					this, phase_, true, alloc
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
		return internal_await_advance<true>(
			phase_in, [this, rel_time, &alloc](phase_type phase) {
				return timed_wait_node<Allocator>::create(
					this, phase, true, rel_time, alloc
				);
			}
		);
	}

	template <bool HasNode, typename WaitNodeSupplier>
	phase_type internal_await_advance(
		phase_type phase_in, WaitNodeSupplier &&w_supp
	)
	{
		(this->*release_waiters)(get_queue<true>(phase_in));

		phase_type phase_out;
		count_type last_unarrived(0);
		auto spins(config.spins_per_arrival());
		wait_node_base *node(HasNode ? w_supp(phase_in) : nullptr);

		while (true) {
			auto s(state.load());
			phase_out = phase_type::make(s);
			if (phase_in != phase_out)
				break;

			if (node) {
				node->enqueue_and_block(get_queue(phase_in));
				if (node->is_releasable())
					break;
			} else {
				auto unarrived(pending_of(s));

				if (unarrived != last_unarrived) {
					last_unarrived = unarrived;
					if (
						last_unarrived
						< config.concurrency()
					)
						spins
						+= config.spins_per_arrival();
				}

				auto interrupted(
					this_thread::get().is_interrupted(true)
				);
				if (interrupted || (--spins < 0)) {
					node = w_supp(phase_in);
					node->flags
					|= interrupted
					? wait_node_base::FLAG_WAS_INTERRUPTED
					: 0;
				}
			}
		}

		auto cond(phase_type::CONDITION::NORMAL);
		if (node) {
			node->waiter.release();
			auto f(node->flags.load());

			if (phase_in == phase_out)
				phase_out = phase_type::make(state.load());

			if (phase_in == phase_out)
				cond = phase_type::CONDITION::TIMED_OUT;

			if (f & wait_node_base::FLAG_WAS_INTERRUPTED) {
				if (!(f & wait_node_base::FLAG_INTERRUPTIBLE))
					this_thread::get().interrupt();

				if (cond != phase_type::CONDITION::NORMAL)
					cond = phase_type::CONDITION::INTERRUPTED;
			}

			node->release();
		}

		if ((
			cond == phase_type::CONDITION::NORMAL
		) || termination_flag_set(state.load()))
			(this->*release_waiters)(get_queue(phase_in));

		phase_out.state += cond;
		return phase_out;
	}

	state_type reconcile_state(state_type s)
	{
		while (true) {
			auto next(root->state.load() & state_phase_type_mask);

			if (next == (s & state_phase_type_mask))
				return s;

			if (!termination_flag_set(next)) {
				auto sp(parties_of(s));
				next |= sp
					? define_state(0, sp, sp)
					: define_state(0, 0, 1);
			} else
				next |= s & ~state_phase_type_mask;

			if (state.compare_exchange_weak(s, next))
				return next;
		}
	}

	void release_waiters_single(std::atomic<wait_node_base *> &q_head)
	{
		auto p(q_head.exchange(nullptr));

		while (p) {
			auto q(p->next);
			p->queue_release();
			p = q;
		}
	}

	void release_waiters_cooperative(std::atomic<wait_node_base *> &q_head)
	{
		auto lock_value(reinterpret_cast<wait_node_base *>(this));
		auto p(q_head.load());

		while (true) {
			if (!p)
				return;

			if (p == lock_value) {
				config.yield_on_conflict();
				p = q_head.load();
				continue;
			}

			if (q_head.compare_exchange_weak(p, lock_value)) {
				auto q(p->next);
				q_head.store(q);
				p->queue_release();
				p = q;
			}
		}
	}

	bool invalid_ptr_none(wait_node_base *p) const
	{
		return false;
	}

	bool invalid_ptr_cooperative(wait_node_base *p) const
	{
		return p == reinterpret_cast<wait_node_base const *>(this);
	}

	constexpr static state_type define_state(
		phase_value_type phase, count_type parties,
		count_type pending
	)
	{
		auto rv((
			static_cast<state_type>(phase) << state_phase_shift
		) & state_phase_mask);
		rv |= static_cast<state_type>(parties) << state_parties_shift;
		rv |= static_cast<state_type>(pending);
		return rv;
	}

	template <bool InvertSelection = false>
	std::atomic<wait_node_base *> &get_queue(phase_type phase)
	{
		if (InvertSelection)
			return (phase.state & (state_type(1) << state_phase_shift))
				? root->even_q_head : root->odd_q_head;
		else
			return (phase.state & (state_type(1) << state_phase_shift))
				? root->odd_q_head : root->even_q_head;
	}

	struct notification_invoker : notification {
		notification_invoker(notification *notifier_)
		: notifier(notifier_ ? notifier_ : this)
		{}

		bool invoke_on_advance(
			phaser &p, phase_type phase,
			count_type registered_parties
		)
		{
			return notifier->on_advance(
				p, phase, registered_parties
			);
		}

		virtual bool on_advance(
			phaser &p, phase_type phase,
			count_type registered_parties
		) override
		{
			return !registered_parties;
		}

		notification *notifier;
	};

	typedef void (phaser::* const release_waiters_t)(
		std::atomic<wait_node_base *> &q_head
	);

	typedef bool (phaser::* const invalid_ptr_t)(
		wait_node_base *p
	) const;

	constexpr static release_waiters_t select_release_waiters_method(
		phaser_config::RELEASE_POLICY p
	)
	{
		switch (p) {
		case phaser_config::RELEASE_POLICY::WAKING_THREAD:
			return &phaser::release_waiters_single;
		case phaser_config::RELEASE_POLICY::COOPERATIVE:
			return &phaser::release_waiters_cooperative;
		default:
			return &phaser::release_waiters_single;
		}
	}

	constexpr static invalid_ptr_t select_invalid_ptr_method(
		phaser_config::RELEASE_POLICY p
	)
	{
		switch (p) {
		case phaser_config::RELEASE_POLICY::WAKING_THREAD:
			return &phaser::invalid_ptr_none;
		case phaser_config::RELEASE_POLICY::COOPERATIVE:
			return &phaser::invalid_ptr_cooperative;
		default:
			return &phaser::invalid_ptr_none;
		}
	}

	phaser * const parent;
	phaser * const root;
	std::mutex self_lock;
	std::atomic<wait_node_base *> even_q_head;
	std::atomic<wait_node_base *> odd_q_head;
	std::atomic<state_type> state;
	notification_invoker notifier;
	phaser_config const &config;
	release_waiters_t release_waiters;
	invalid_ptr_t invalid_ptr;
};

namespace test {

struct phaser_probe {
	static void set_state(
		phaser &p, phaser::phase_value_type phase,
		phaser::count_type parties, phaser::count_type unarrived
	)
	{
		p.state.store(phaser::define_state(phase, parties, unarrived));
	}
};

}
}
#endif

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
#if !defined(HPP_BC4D53ED8E18803848191BFF73AFC2FF)
#define HPP_BC4D53ED8E18803848191BFF73AFC2FF

#include <yesod/concurrent/thread.hpp>
#include <yesod/detail/allocator_helper.hpp>
#include <yesod/concurrent/detail/ring_fifo.hpp>

namespace ucpf::yesod::concurrent {

namespace test {

struct phaser_probe;

}

struct phaser;

struct phaser_config {
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

	private:
		friend phaser;

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

	virtual ~phaser_config() = default;

	enum class RELEASE_POLICY {
		WAKING_THREAD = 0,
		COOPERATIVE = 1
	};

	virtual RELEASE_POLICY waiter_release_policy() const = 0;

	virtual int spins_per_arrival() const = 0;

	virtual unsigned int concurrency() const = 0;

	virtual void yield_on_conflict() const = 0;

	virtual size_t initial_wait_queue_size() const = 0;

	virtual bool on_advance(
		phase_type phase, count_type registered_parties
	) const = 0;

protected:
	friend phaser;

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
};

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

	size_t initial_wait_queue_size() const override
	{
		return 16;
	}

	bool on_advance(
		phase_type phase, count_type registered_parties
	) const override
	{
		return !registered_parties;
	}

	static default_phaser_config instance;

private:
	unsigned int concurrency_value;
	int spins_per_arrival_value;
};

template <typename Unused>
default_phaser_config<Unused> default_phaser_config<Unused>::instance;

struct phaser {
	typedef phaser_config config_type;
	typedef typename config_type::state_type state_type;
	typedef typename config_type::count_type count_type;
	typedef typename config_type::phase_type phase_type;

	phaser(phaser &) = delete;
	phaser(phaser &&) = delete;
	phaser(phaser const &) = delete;
	phaser(phaser const &&) = delete;

	template <typename Allocator = std::allocator<void>>
	phaser(
		count_type parties = 0, phaser *parent_ = nullptr,
		config_type const &config = default_phaser_config<>::instance,
		Allocator const &alloc = Allocator()
	)
	: parent(parent_), wait_queue(
		parent
		? parent->wait_queue
		: wait_queue_impl<Allocator>::create(alloc, *this, config)
	)
	{
		phase_type phase;

		if (parent && parties)
			phase = parent->do_register(1);

		state.store(parties ? define_state(
			phase.value(), parties, parties
		) : define_state(0, 0, 1));
	}

	~phaser()
	{
		force_termination();
		if (!parent)
			wait_queue->destroy();
	}

	phase_type register_one()
	{
		return do_register(1);
	}

	phase_type register_some(count_type parties)
	{
		return parties ? do_register(parties) : get_phase();
	}

	phase_type arrive()
	{
		return do_arrive(define_state(0, 0, 1));
	}

	phase_type arrive_and_deregister()
	{
		return do_arrive(define_state(0, 1, 1));
	}

	phase_type arrive_and_await_advance()
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

			unarrived = config_type::unarrived_of(s);
			if (!unarrived)
				return phase_type::make_err(s);

			next = s - 1;
		} while (!state.compare_exchange_weak(s, next));

		s = next;

		if (unarrived > 1)
			return wait_queue->await_advance(phase_out, false);

		if (!parent) {
			auto next_unarrived(config_type::parties_of(s));
			auto next_phase(phase_out + 1);

			if (wait_queue->config.on_advance(
				phase_out, next_unarrived
			))
				next = next_phase.to_state(
					next_unarrived, 0
				) | config_type::phaser_terminated_mask;
			else if (!next_unarrived)
				next = next_phase.to_state(0, 1);
			else
				next = next_phase.to_state(
					next_unarrived, next_unarrived
				);

			if (!state.compare_exchange_strong(s, next))
				return phase_type::make(s);

			wait_queue->release_waiters(phase_out);

			return phase_type::make(next);
		} else
			return parent->arrive_and_await_advance();
	}

	phase_type await_advance(phase_type phase_in)
	{
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return wait_queue->await_advance(phase_in, false);
		else
			return phase_out;
	}

	phase_type await_advance_interruptibly(phase_type phase_in)
	{
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return wait_queue->await_advance(phase_in, true);
		else
			return phase_out;
	}

	template <typename Rep, typename Period>
	phase_type await_advance_for(
		phase_type phase_in,
		std::chrono::duration<Rep, Period> const &rel_time
	)
	{
		if (phase_in.terminal())
			return phase_in;

		auto s(state.load());
		if (parent)
			s = reconcile_state(s);

		auto phase_out(phase_type::make(s));
		if (phase_out == phase_in)
			return wait_queue->await_advance_timed(
				phase_in, rel_time
			);
		else
			return phase_out;
	}

	void force_termination()
	{
		auto s(wait_queue->root.state.load());
		if (config_type::termination_flag_set(s))
			return;

		while (!wait_queue->root.state.compare_exchange_weak(
			s, s | config_type::phaser_terminated_mask
		)) {}

		wait_queue->release_all();
	}

	phase_type get_phase() const
	{
		return phase_type::make(wait_queue->root.state.load());
	}

	count_type get_registered_parties() const
	{
		return config_type::parties_of(state.load());
	}

	count_type get_arrived_parties()
	{
		auto s(state.load());
		return config_type::arrived_of(
			parent ? reconcile_state(s) : s
		);
	}

	count_type get_unarrived_parties()
	{
		auto s(state.load());
		return config_type::unarrived_of(
			parent ? reconcile_state(s) : s
		);
	}

	phaser *get_parent() const
	{
		return parent;
	}

	phaser *get_root() const
	{
		return &wait_queue->root;
	}

	bool is_terminated() const
	{
		return config_type::termination_flag_set(
			wait_queue->root.state.load()
		);
	}

private:
	friend test::phaser_probe;
	typedef typename config_type::phase_value_type phase_value_type;

	struct wait_node {
	};

	struct wait_queue_base {
		typedef wait_node fifo_item_type;

		virtual void destroy() = 0;

		virtual fifo_item_type *allocate_array(size_t count) const = 0;

		virtual void deallocate_array(
			fifo_item_type *p, size_t count
		) const = 0;

		wait_queue_base(phaser &root_, config_type const &config_)
		: root(root_), config(config_),
		  even(config.initial_wait_queue_size(), *this),
		  odd(config.initial_wait_queue_size(), *this)
		{}

		typename phaser_config::phase_type await_advance(
			config_type::phase_type phase_in, bool interruptible
		);

		typename phaser_config::phase_type await_advance_timed(
			config_type::phase_type phase_in,
			std::chrono::nanoseconds wait_time
		);

		void release_waiters(phaser_config::phase_type phase_in);

		void release_all();

		phaser &root;
		config_type const &config;
		detail::ring_fifo<wait_queue_base> even;
		detail::ring_fifo<wait_queue_base> odd;
	};

	template <typename Allocator>
	struct wait_queue_impl : wait_queue_base {
		static wait_queue_base *create(
			Allocator const &alloc, phaser &root_,
			config_type const &config_
		)
		{
			return yesod::detail::allocated_object<
				wait_queue_impl<Allocator>, Allocator
			>::create(alloc, root_, config_)->get();
		}

		wait_queue_impl(phaser &root_, config_type const &config_)
		: wait_queue_base(root_, config_)
		{}

		void destroy() override
		{
			yesod::detail::allocated_object<
				wait_queue_impl, Allocator
			>::to_storage_ptr(this)->destroy();
		}

		fifo_item_type *allocate_array(size_t count) const override
		{
			return yesod::detail::allocator_helper<
				fifo_item_type
			>::claim_n(
				yesod::detail::allocated_object<
					wait_queue_impl, Allocator
				>::to_storage_ptr(this)->get_allocator(),
				count
			);
		}

		void deallocate_array(
			fifo_item_type *p, size_t count
		) const override
		{
			yesod::detail::allocator_helper<
				fifo_item_type
			>::relinquish_n(
				yesod::detail::allocated_object<
					wait_queue_impl, Allocator
				>::to_storage_ptr(this)->get_allocator(),
				p, count
			);
		}
	};

	constexpr static state_type define_state(
		phase_value_type phase, count_type parties,
		count_type pending
	)
	{
		auto rv((
			static_cast<state_type>(phase)
			<< config_type::state_phase_shift
		) & config_type::state_phase_mask);
		rv |= static_cast<state_type>(parties)
		      << config_type::state_parties_shift;
		rv |= static_cast<state_type>(pending);
		return rv;
	}

	state_type reconcile_state(state_type s)
	{
		while (true) {
			auto next(
				wait_queue->root.state.load()
				& config_type::state_phase_type_mask
			);

			if (next == (s & config_type::state_phase_type_mask))
				return s;

			if (!config_type::termination_flag_set(next)) {
				auto sp(config_type::parties_of(s));
				next |= sp ? define_state(
					0, sp, sp
				) : define_state(0, 0, 1);
			} else
				next |= s & ~config_type::state_phase_type_mask;

			if (state.compare_exchange_weak(s, next))
				return next;
		}
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

			unarrived = config_type::unarrived_of(s);
			if (!unarrived)
				return phase_type::make_err(s);

			next = s - adjust;
		} while (!state.compare_exchange_weak(s, next));

		phase_out = phase_type::make(next);
		if (unarrived != 1)
			return phase_out;

		s = next;
		auto next_unarrived(config_type::parties_of(s));
		auto next_phase(phase_out + 1);

		if (!parent) {
			if (wait_queue->config.on_advance(
				phase_out, next_unarrived
			))
				next = next_phase.to_state(
					next_unarrived, 0
				) | config_type::phaser_terminated_mask;
			else if (!next_unarrived)
				next = next_phase.to_state(0, 1);
			else
				next = next_phase.to_state(
					next_unarrived, next_unarrived
				);

			state.compare_exchange_strong(s, next);
			wait_queue->release_waiters(phase_out);
		} else if (!next_unarrived) {
			phase_out = parent->do_arrive(define_state(0, 1, 1));
			state.compare_exchange_strong(
				s, define_state(config_type::phase_of(s), 0, 1)
			);
		} else
			phase_out = parent->do_arrive(define_state(0, 0, 1));

		return phase_out;
	}

	phase_type do_register(count_type registrations)
	{
		state_type s(state.load());
		phase_type phase_out;

		while (true) {
			if (parent)
				s = reconcile_state(s);

			phase_out = phase_type::make(s);
			if (phase_out.terminal())
				break;

			auto parties(config_type::parties_of(s));
			auto unarrived(config_type::pending_of(s));

			if (count_type(registrations + parties) < parties)
				return phase_type::make_err(s);

			if (parties || !unarrived) {
				if (!parent || (reconcile_state(s) == s)) {
					if (!unarrived) {
						wait_queue->await_advance(
							phase_out, false
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

				phase_out = parent->do_register(1);
				if (phase_out.terminal())
					break;

				auto next(phase_out.to_state(
					registrations, registrations
				));

				while (!state.compare_exchange_weak(s, next)) {
					phase_out = phase_type::make(
						wait_queue->root.state.load()
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

	phaser * const parent;
	wait_queue_base * const wait_queue;
	std::mutex self_lock;
	std::atomic<state_type> state;
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

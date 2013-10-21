/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on algorithm from:
 * 
 *      A Dynamic Elimination-Combining Stack Algorithm (Gal Bar-Nissan,
 *      Danny Hendler, Adi Suissa) in 15th International Conference,
 *      OPODIS 2011
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STACK_DECS_OCT_18_2013_1330)
#define UCPF_YESOD_STACK_DECS_OCT_18_2013_1330

#include <thread>
#include <bitset>
#include <atomic>
#include <condition_variable>

#include <boost/lockfree/detail/tagged_ptr.hpp>

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/detail/utilities.hpp>
#include <boost/intrusive/member_value_traits.hpp>

#include <yesod/slist_node.hpp>

namespace ucpf { namespace yesod {

template <
	typename ValueType, typename NodeType, NodeType ValueType::* NodePtr,
	std::size_t MaxActors = 8
>
struct stack_decs {
	typedef ValueType       value_type;
	typedef ValueType       &reference;
	typedef ValueType const &const_reference;
	typedef ValueType       *pointer;
        typedef ValueType const *const_pointer;

	typedef typename NodeType::node           node;
	typedef typename NodeType::node_ptr       node_ptr;
	typedef typename NodeType::const_node_ptr const_node_ptr;

	typedef boost::intrusive::member_value_traits<
		value_type, NodeType, NodePtr
	> member_value_traits;

	typedef typename boost::lockfree::detail::tagged_ptr<
		node
	> tagged_head_ptr;

	static constexpr std::size_t initial_action_count = 4;

	struct actor {
		std::size_t decs_id;
		std::thread::id thread_id;
	};

	stack_decs()
	{
		id_set.set();
	}

	bool attach(actor &ar)
	{
		std::unique_lock<std::mutex> l_g(id_lock);
		ar.decs_id = id_set._Find_first();
		if (ar.id >= id_set.size())
			return false;
		else {
			id_set.reset(ar.id);
			ar.thread_id = std::this_thread::get_id();
			return true;
		}
	}

	void detach(actor &ar)
	{
		std::unique_lock<std::mutex> l_g(id_lock);
		id_set.set(ar.decs_id);
	}

	void push(actor const &ar, reference v)
	{
		assert(ar.thread_id == std::this_thread::get_id());

		auto an(action_alloc(ar));
		an.actor_id = ar.decs_id;
		an.count = 1;
		an.status = action::INIT;
		an.h = member_value_traits::to_node_ptr(v);
		an.next = nullptr;
		an.last = &an;
		an.push_op = true;

		while (!(action_push(ar, an) || action_collide(ar, an))) {}
	}

	pointer pop(actor const &ar)
	{
		assert(ar.thread_id == std::this_thread::get_id());

		auto an(action_alloc(ar));
		an.actor_id = ar.decs_id;
		an.count = 1;
		an.status = action::INIT;
		an.h = nullptr;
		an.next = nullptr;
		an.last = &an;
		an.push_op = false;

		while (!(action_pop(ar, an) || action_collide(ar, an))) {}

		return member_value_traits::to_value_ptr(an.h);
	}

private:
	struct action {
		std::size_t actor_id;
		size_t count;
		enum {
			INIT = 0,
			FINISHED,
			RETRY
		} status;
		node_ptr h;
		action *next;
		action *last;
		bool push_op;
		boost::intrusive::list_member_hook<
			boost::intrusive::link_mode<boost::intrusive::safe_link>
		> freelist_hook;
	};

	typedef typename boost::intrusive::list<
			action, boost::intrusive::member_hook<
				action, decltype(action::freelist_hook),
				&action::freelist_hook
			>, boost::intrusive::constant_time_size<false>
	> action_freelist_t;

	action &action_alloc(actor const &ar)
	{
		auto &l(freelist[ar.decs_id]);

		if (!l.empty()) {
			auto &rv(l.front());
			l.pop_front();
			return rv;
		} else
			return *(new action());
	}

	void action_free(actor const &ar, action &an)
	{
		freelist[ar.decs_id].push_back(an);
	}

	bool action_push(actor const &ar, action &an)
	{
		auto old_head(head.load());
		tagged_head_ptr new_head(an.h, old_head.get_tag());
		an.h->next  = old_head.get_ptr();
		if (head.compare_exchange_weak(old_head, new_head)) {
			auto p(&an);
			do {
				p->status = action::FINISHED;
				p = p->next;
			} while (p);
			cond_var.notify_all();
			return true;
		} else
			return false;
		
	}

	bool action_pop(actor const &ar, action &an)
	{
		auto old_head(head.load());
		auto h(old_head.get_ptr());
		if (!h) {
			auto p(&an);
			do {
				p->h = nullptr;
				p->status = action::FINISHED;
				p = p->next;
			} while (p);
			cond_var.notify_all();
			return true;
		}

		std::size_t count(1);
		auto q(h->next);
		while (q && (count < an.count)) {
			q = q->next;
			++count;
		}
		tagged_head_ptr new_head(
			q, /*old_head.get_next_tag()*/
			old_head.get_tag() + 1
		);
		if (head.compare_exchange_weak(old_head, new_head)) {
			an.h = h;
			h = h->next;
			auto p(an.next);
			while (p) {
				p->h = h;
				if (h)
					h = h->next;
				p->status = action::FINISHED;
				p = p->next;
			}
			cond_var.notify_all();
			return true;
		} else
			return false;
	}

	bool action_collide(actor const &ar, action &an)
	{
		auto an_p(&an);
		location[ar.decs_id] = an_p;
		auto other_idx(selector());
		auto other(collision[other_idx].load());
		while (!collision[other_idx].compare_exchange_weak(
			other, &ar
		)) {}

		if (other) {
			auto other_an(location[other->decs_id].load());
			if (
				other_an && (other_an.actor_id != ar.decs_id)
				&& (other_an.actor_id == other.decs_id)
			) {
				if (location[ar.decs_id].compare_exchange_weak(
					an_p, nullptr
				))
					return action_collide_active(
						an, other_an
					);
				else
					return action_collide_passive(ar, an);
			}
		}

		std::this_thread::sleep_for(
			std::chrono::milliseconds(10)
		);

		if (!location[ar.decs_id].compare_exchange_weak(an_p, nullptr))
			return action_collide_passive(ar, an);

		return false;
	}

	bool action_collide_active(action &an, action &other_an)
	{
		auto an_p(&other_an);
		if (location[other_an.actor_id].compare_exchange_weak(
			an_p, &an
		)) {
			if (an.push_op == other_an.push_op) {
				if (an.push_op)
					an.last->h->next = other_an.h;

				an.last->next = &other_an;
				an.last = other_an.last;
				an.count += other_an.count;
				return false;
			} else {
				action_eliminate(an, other_an);
				auto an_p(&an);
				auto an_q(&other_an);
				do {
					if (!an.push_op)
						an_p->h = an_q->h;
					else
						an_q->h = an_p->h;

					an_p->status = action::FINISHED;
					an_q->status = action::FINISHED;
					--an.count;
					--other_an.count;
					an_p = an_p->next;
					an_q = an_q->next;
				} while (an_p && an_q);

				if (an_p) {
					an_p->count = an.count;
					an_p->last = an.last;
					an_p->status = action::RETRY;
				} else if (an_q) {
					an_q->count = other_an.count;
					an_q->last = other_an.last;
					an_q->status = action::RETRY;
				}
				cond_var.notify_all();
				return true;
			}
		} else
			return false;
	}

	bool action_collide_passive(action &an)
	{
		action *an_p(location[an.actor_id].load());
		location[an.actor_id] = nullptr;
		if (an_p->op != an.op) {
			if (!an.push_op)
				an.h = an_p->h;

			return true;
		} else {
			std::unique_lock<std::mutex> l_g(wait_lock);
			while (an.status == action::INIT)
				cond_var.wait(l_g);

			if (an.status == action::FINISHED)
				return true;
			else {
				an.status = action::INIT;
				return false;
			}
		}
	}

	std::mutex id_lock;
	std::mutex wait_lock;
	std::bitset<MaxActors> id_set;
	std::array<std::atomic<actor const *>, MaxActors> collision;
	std::array<std::atomic<action *>, MaxActors> location;
	std::array<action_freelist_t, MaxActors> freelist;
	std::atomic<tagged_head_ptr> head;
	std::condition_variable cond_var;
	std::ranlux48 selector;
};

}}

#endif

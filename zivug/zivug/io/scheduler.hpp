/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4FA543F2863804554BDE0F1328BAE25E)
#define HPP_4FA543F2863804554BDE0F1328BAE25E

#include <zivug/io/actor.hpp>
#include <zivug/arch/io/event_dispatcher.hpp>

namespace ucpf { namespace zivug { namespace io {

struct scheduler {
	virtual void imbue(descriptor &&d, actor &act) = 0;
	virtual void poll() = 0;
	virtual void wait() = 0;
};

template <typename Alloc>
struct rr_scheduler : scheduler {
	rr_scheduler(Alloc const &a)
	: tup_endp_alloc(node<all_nodes_tag>(), a)
	{}

	virtual ~rr_scheduler()
	{
		auto r(&std::get<0>(tup_endp_alloc));

		while (r->next != r) {
			auto p(r->next);
			auto endp(static_cast<managed_endp *>(p));
			auto q(static_cast<node<active_nodes_tag>>(endp));
			if (q == q->next)
				disp.remove(endp.d);
			else {
				q->next->prev = q->prev;
				q->prev->next = q->next;
			}
			p->next->prev = p->prev;
			p->prev->next = p->next;
			endp->act.release(*this, endp->d);
			ah_type::destroy(
				std::get<1>(tup_endp_alloc), endp, 1, true
			);
		}
	}

	virtual void imbue(descriptor &&d, actor &act)
	{
		auto endp(ah_type::alloc_n(
			std::get<1>(tup_endp_alloc), 1,
			*this, std::move(d), act
		));
		auto endp_n(static_cast<node<all_nodes_tag> *>(endp));
		auto r(&std::get<0>(tup_endp_alloc));
		endp_n->next = r;
		endp_n->prev = r->prev;
		r->prev->next = endp_n;
		r->prev = endp_n;
		disp.reset(endp->d, static_cast<endpoint &>(*endp), false);
	}

	virtual void poll()
	{
		while (disp.poll_next())
		{}

		for (auto endp(poll_active()); endp; endp = poll_active()) {
			auto &act(endp->act);
			int next(0);

			if (endp->next_action & actor::READ)
				next |= act->read(
					*this, endp->d, false, false
				);

			if (endp->next_action & actor::WRITE)
				next |= act->write(
					*this, endp->d, false, false
				);

			if (next & actor::WAIT)
				disp.reset(
					endp->d, *endp, !(next & actor::WRITE)
				);
			else
				add_active(next, *endp);
		}
	}

	virtual void wait()
	{
		if (disp.wait_next())
			poll();
	}

private:
	template <typename Tag>
	struct node {
		node()
		: next(this), prev(this)
		{}

	private:
		friend struct scheduler;

		node *next;
		node *prev;
	};

	struct all_nodes_tag
	{};

	struct active_nodes_tag
	{};

	struct managed_endp
	: endpoint, private node<all_nodes_tag>,
	  private node<active_nodes_tag> {
		managed_endp(scheduler &parent_, descriptor &&d_, actor &act_)
		:  parent(parent_), d(std::move(d_)), act(act_), next_action(0)
		{}

		virtual void read_ready(bool out_of_band, bool priority)
		{
			parent.handle_read(*this, out_of_band, priority);
		}

		virtual void write_ready(bool out_of_band, bool priority)
		{
			parent.handle_write(*this, out_of_band, priority);
		}

		virtual void error(bool priority)
		{
			parent.handle_error(*this, priority);
		}

		virtual void hang_up(bool read_only)
		{
			parent.handle_hang_up(*this, read_only);
		}
	private:
		friend struct scheduler;

		scheduler &parent;
		descriptor d;
		actor &act;
		int next_action;
	};

	friend struct managed_endp;
	typedef ucpf::yesod::allocator::array_helper<
		managed_endp, Alloc
	> ah_type;

	void handle_read(managed_endp &endp, bool out_of_band, bool priority)
	{
		auto next(endp.act.read(endp.d, out_of_band, priority));
		if (next & actor::WAIT)
			disp.reset(endp.d, endp, !(next & actor::WRITE));
		else
			add_active(next, endp);
	}

	void handle_write(managed_endp &endp, bool out_of_band, bool priority)
	{
		auto &act(endp.d.context<actor>());

		auto next(endp.act.write(endp.d, out_of_band, priority));
		if (next & actor::WAIT)
			disp.reset(endp.d, endp, !(next & actor::WRITE));
		else
			add_active(next, endp);
	}

	void handle_error(managed_endp &endp, bool priority)
	{
	}

	void handle_hang_up(managed_endp &endp, bool read_only)
	{
	}

	void add_active(int next_action, managed_endp &endp)
	{
		auto n(static_cast<node<active_nodes_tag> &>(endp));
		endp.next_action = next_action;
		n.next = &endp_active;
		n.prev = endp_active.prev;
		endp_active.prev->next = n;
		endp_active.prev = n;
	}

	managed_endp *poll_active()
	{
		if (endp_active.next != &endp_active) {
			auto n(endp_active.next);
			endp_active.next = n->next;
			n->next->prev = &endp_active;
			n->next = n;
			n->prev = n;
			return static_cast<managed_endp *>(n);
		} else
			return nullptr;
	}

	event_dispatcher disp;
	node<active_nodes_tag> endp_active;
	std::tuple<
		node<all_nodes_tag>, typename ah_type::allocator_type
	> tup_endp_alloc;
};

}}}
#endif

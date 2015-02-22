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

template <typename Alloc>
struct scheduler {
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
		managed_endp(descriptor &&d_, scheduler &parent_)
		: d(std::move(d_)), parent(parent_), next_action(0)
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
		int next_action;
	};

	scheduler(Alloc const &a)
	{}

	void poll()
	{
		while (disp.poll_next())
		{}

		for (auto endp(poll_active()); endp; endp = poll_active()) {
			auto &act(endp->d.context<actor>());
			int next(0);

			if (endp->next_action & actor::READ)
				next |= act->read(endp->d, false, false);

			if (endp->next_action & actor::WRITE)
				next |= act->write(endp->d, false, false);

			if (endp->next_action & actor::ACCEPT) {
				descriptor con_d;
				next |= act->accept(endp->d, con_d);
				if (con_d) {
					auto &x_endp(
						add_endp(std::move(con_d))
					);
					disp.reset(x_endp.d, x_endp, false);
				}
			}

			if (next & actor::WAIT)
				disp.reset(
					endp->d, *endp, !(next & actor::WRITE)
				);
			else
				add_active(next, *endp);
		}
	}

	void wait()
	{
		if (disp.wait_next())
			poll();
	}

private:
	friend struct managed_endp;

	void handle_read(managed_endp &endp, bool out_of_band, bool priority)
	{
		auto &act(endp.d.context<actor>());

		auto next(act->read(endp.d, out_of_band, priority));
		if (next & actor::WAIT)
			disp.reset(endp.d, endp, !(next & actor::WRITE));
		else
			add_active(next, endp);
	}

	void handle_write(managed_endp &endp, bool out_of_band, bool priority)
	{
		auto &act(endp.d.context<actor>());

		auto next(act->write(endp.d, out_of_band, priority));
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
	node<all_nodes_tag> endp_all;
	node<active_nodes_tag> endp_active;
};

}}}
#endif

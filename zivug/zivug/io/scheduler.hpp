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
private:
	
public:
	template <typename ConfigType>
	rr_scheduler(ConfigType const &config, Alloc const &a)
	: disp(config.epoll), tup_endp_alloc(node<all_nodes_tag>(), a)
	{}

	virtual ~rr_scheduler()
	{
		auto r(&std::get<0>(tup_endp_alloc));

		while (r->next != r) {
			auto p(r->next);
			auto endp(static_cast<managed_endp *>(p));
			auto q(static_cast<node<active_nodes_tag> *>(endp));
			if (q == q->next)
				disp.remove(endp->d);
			else {
				q->next->prev = q->prev;
				q->prev->next = q->next;
			}
			p->next->prev = p->prev;
			p->prev->next = p->next;
			endp->act.release(action(*endp));
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
		act.init(action(*endp));
	}

	virtual void poll()
	{
		while (disp.poll_next())
		{}

		for (auto endp(poll_active()); endp; endp = poll_active()) {
			switch (endp->next_action) {
			case READ:
				endp->act.read(action(*endp), false, false);
				break;
			case WRITE:
				endp->act.write(action(*endp), false, false);
				break;
			};
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
		friend struct rr_scheduler;

		node *next;
		node *prev;
	};

	struct all_nodes_tag
	{};

	struct active_nodes_tag
	{};

	struct action;

	enum next_action_t : int {
		READ = 0,
		WRITE
	};

	struct managed_endp
	: endpoint, private node<all_nodes_tag>,
	  private node<active_nodes_tag> {
		managed_endp(
			rr_scheduler &parent_, descriptor &&d_, actor &act_
		) : parent(parent_), d(std::move(d_)), act(act_),
		    next_action(next_action_t::READ)
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
		friend struct rr_scheduler;
		friend struct action;

		rr_scheduler &parent;
		descriptor d;
		actor &act;
		next_action_t next_action;
	};

	struct action : scheduler_action {
		action(managed_endp &endp_)
		: endp(endp_)
		{}

		virtual void resume_read()
		{
			endp.next_action = READ;
			static_cast<rr_scheduler &>(
				get_scheduler()
			).add_active(endp);
		}

		virtual void resume_write()
		{
			endp.next_action = WRITE;
			static_cast<rr_scheduler &>(
				get_scheduler()
			).add_active(endp);
		}

		virtual void wait_read()
		{
			static_cast<rr_scheduler &>(
				get_scheduler()
			).disp.reset_read(endp.d, endp);
		}

		virtual void wait_write()
		{
			static_cast<rr_scheduler &>(
				get_scheduler()
			).disp.reset_write(endp.d, endp);
		}

		virtual scheduler &get_scheduler()
		{
			return endp.parent;
		}

		virtual descriptor const &get_descriptor()
		{
			return endp.d;
		}

	private:
		managed_endp &endp;
	};

	friend struct managed_endp;
	friend action;

	typedef ucpf::yesod::allocator::array_helper<
		managed_endp, Alloc
	> ah_type;

	void handle_read(managed_endp &endp, bool out_of_band, bool priority)
	{
		endp.act.read(action(endp), out_of_band, priority);
	}

	void handle_write(managed_endp &endp, bool out_of_band, bool priority)
	{
		endp.act.write(action(endp), out_of_band, priority);
	}

	void handle_error(managed_endp &endp, bool priority)
	{
	}

	void handle_hang_up(managed_endp &endp, bool read_only)
	{
	}

	void add_active(managed_endp &endp)
	{
		auto n(static_cast<node<active_nodes_tag> *>(&endp));
		n->next = &endp_active;
		n->prev = endp_active.prev;
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

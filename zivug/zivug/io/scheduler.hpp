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

protected:
	static scheduler_token produce_token(void *ptr)
	{
		return scheduler_token(ptr);
	}

	static void *access_token(scheduler_token const &tk)
	{
		return tk.ptr;
	}
};

template <typename Alloc>
struct rr_scheduler : scheduler {
	template <typename ConfigType>
	rr_scheduler(ConfigType const &config, Alloc const &a)
	: disp(config.epoll), endp_list(a)
	{}

	virtual ~rr_scheduler()
	{
		destroy_released();

		while (true) {
			auto n(endp_list.all.template unlink_next<
				managed_endp
			>());

			if (!n)
				break;

			destroy_endp(n);
		}
	}

	virtual void imbue(descriptor &&d, actor &act)
	{
		auto endp(ah_type::alloc_n(
			endp_list.get_allocator(), 1, *this, std::move(d), &act
		));

		static_cast<node<all_nodes_tag> *>(endp)->link_before(
			&endp_list.all
		);

		endp->act->init(action(*endp), true);
		destroy_released();
	}

	virtual void poll()
	{
		while (disp.poll_next())
		{}

		while (true) {
			auto endp(poll_active());
			if (!endp)
				break;

			switch (endp->next_action) {
			case READ:
				endp->act->read(action(*endp), false, false);
				break;
			case WRITE:
				endp->act->write(action(*endp), false, false);
				break;
			};
		}

		destroy_released();
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

		node(Tag dummy)
		: next(this), prev(this)
		{}

		node(node const &other) = delete;
		node &operator=(node const &other) = delete;

		void unlink()
		{
			if (linked()) {
				next->prev = prev;
				prev->next = next;
				prev = this;
				next = this;
			}
		}

		template <typename T>
		T *unlink_next()
		{
			if (linked()) {
				auto n(next);
				n->unlink();
				return static_cast<T *>(n);
			} else
				return nullptr;
		}

		template <typename T>
		T *unlink_prev()
		{
			if (linked()) {
				auto n(prev);
				n->unlink();
				return static_cast<T *>(n);
			} else
				return nullptr;
		}

		void link_before(node *n)
		{
			next = n;
			prev = n->prev;
			n->prev = this;
			prev->next = this;
		}

		void link_after(node *n)
		{
			next = n->next;
			prev = n;
			n->next = this;
			next->prev = this;
		}

		bool linked() const
		{
			return next != this;
		}

	private:
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
			rr_scheduler &parent_, descriptor &&d_, actor *act_
		) : parent(parent_), d(std::move(d_)), act(act_),
		    next_action(next_action_t::READ)
		{}

		virtual bool read_ready(bool out_of_band, bool priority)
		{
			return parent.handle_read(
				*this, out_of_band, priority
			);
		}

		virtual bool write_ready(bool out_of_band, bool priority)
		{
			return parent.handle_write(
				*this, out_of_band, priority
			);
		}

		virtual bool error(bool priority)
		{
			return parent.handle_error(*this, priority);
		}

		virtual bool hang_up(bool read_only)
		{
			return parent.handle_hang_up(*this, read_only);
		}

	private:
		friend struct rr_scheduler;
		friend struct action;

		rr_scheduler &parent;
		descriptor d;
		actor *act;
		next_action_t next_action;
	};

	struct action : scheduler_action {
		action(managed_endp &endp_)
		: endp(endp_)
		{}

		virtual void resume_read()
		{
			endp.next_action = READ;
			endp.parent.add_active(endp);
		}

		virtual void resume_read(scheduler_token &&tk)
		{
			auto x_endp(reinterpret_cast<managed_endp *>(
				rr_scheduler::access_token(tk)
			));
			if (x_endp) {
				x_endp->next_action = READ;
				x_endp->parent.add_active(*x_endp);
			}
		}

		virtual void resume_write()
		{
			endp.next_action = WRITE;
			endp.parent.add_active(endp);
		}

		virtual void resume_write(scheduler_token &&tk)
		{
			auto x_endp(reinterpret_cast<managed_endp *>(
				access_token(tk)
			));
			if (x_endp) {
				x_endp->next_action = WRITE;
				x_endp->parent.add_active(*x_endp);
			}
		}

		virtual void wait_read()
		{
			endp.parent.disp.reset_read(endp.d, endp);
		}

		virtual void wait_read(scheduler_token &&tk)
		{
			auto x_endp(reinterpret_cast<managed_endp *>(
				rr_scheduler::access_token(tk)
			));
			if (x_endp)
				x_endp->parent.disp.reset_read(
					x_endp->d, *x_endp
				);
		}

		virtual void wait_write()
		{
			endp.parent.disp.reset_write(endp.d, endp);
		}

		virtual void wait_write(scheduler_token &&tk)
		{
			auto x_endp(reinterpret_cast<managed_endp *>(
				rr_scheduler::access_token(tk)
			));
			if (x_endp)
				x_endp->parent.disp.reset_write(
					x_endp->d, *x_endp
				);
		}

		virtual void release()
		{
			endp.parent.release_endp(&endp);
		}

		virtual void release(scheduler_token &&tk)
		{
			auto x_endp(reinterpret_cast<managed_endp *>(
				rr_scheduler::access_token(tk)
			));
			if (x_endp)
				x_endp->parent.release_endp(x_endp);
		}

		virtual scheduler_token suspend()
		{
			return rr_scheduler::produce_token(
				reinterpret_cast<void *>(&endp)
			);
		}

		virtual void set_actor(actor &act)
		{
			endp.act = &act;
			endp.act->init(action(endp), false);
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

	bool handle_read(managed_endp &endp, bool out_of_band, bool priority)
	{
		return endp.act->read(action(endp), out_of_band, priority);
	}

	bool handle_write(
		managed_endp &endp, bool out_of_band, bool priority
	)
	{
		return endp.act->write(action(endp), out_of_band, priority);
	}

	bool handle_error(managed_endp &endp, bool priority)
	{
		return endp.act->error(action(endp), priority);
	}

	bool handle_hang_up(managed_endp &endp, bool read_only)
	{
		return endp.act->hang_up(action(endp), read_only);
	}

	void release_endp(managed_endp *endp)
	{
		static_cast<node<all_nodes_tag> *>(endp)->unlink();
		static_cast<node<all_nodes_tag> *>(endp)->link_before(
			&endp_list.released
		);
	}

	void destroy_released()
	{
		while (true) {
			auto n(endp_list.released.template unlink_next<
				managed_endp
			>());

			if (!n)
				break;

			destroy_endp(n);
		}
	}

	void destroy_endp(managed_endp *endp)
	{
		disp.remove(endp->d);
		endp->act->fini(action(*endp));
		ah_type::destroy(endp_list.get_allocator(), endp, 1, true);
	}

	void add_active(managed_endp &endp)
	{
		static_cast<node<active_nodes_tag> *>(&endp)->link_after(
			&endp_list.active
		);
	}

	managed_endp *poll_active()
	{
		return endp_list.active.template unlink_prev<managed_endp>();
	}

	event_dispatcher disp;
	struct endp_list_t : ah_type::allocator_type {
		endp_list_t(Alloc const &a)
		: ah_type::allocator_type(a)
		{}

		auto &get_allocator()
		{
			return *static_cast<
				typename ah_type::allocator_type *
			>(this);
		}

		node<all_nodes_tag> all;
		node<all_nodes_tag> released;
		node<active_nodes_tag> active;
	} endp_list;
};

}}}
#endif

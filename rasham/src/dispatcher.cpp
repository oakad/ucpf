/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <rasham/internal/dispatcher.hpp>
#include <cstdarg>

namespace rasham
{

static dispatcher global_dispatcher;

dispatcher *get_global_dispatcher()
{
	return &global_dispatcher;
}

dispatcher::dispatcher()
: root(sink_nentry::type::make())
{
}

dispatcher::~dispatcher()
{
	locus_map.clear();
	sink_nentry::type::cull_tree(root);
	root.reset();
}

sink_nentry dispatcher::get_locus_nentry(locus const *loc)
{
	sink_nentry r;

	bool check(false);
	{
		std::lock_guard<std::mutex> l_g(lock);
		auto iter(locus_map.find(loc));

		if (iter != locus_map.end())
			return iter->second;
		else
			r = root;
	}

	r = sink_nentry::type::lookup_path(r, loc->dest, true);
	r->lock();
	auto &e(r->get_entry());
	if (!e.loci.empty()) {
		auto iter(e.loci.cbegin());
		*(loc->cond) = *((*iter)->cond);
	} else
		check = true;

	e.loci.emplace(loc);
	r->unlock();

	{
		std::lock_guard<std::mutex> l_g(lock);
		locus_map.emplace(loc, r);
		if (check) {
			check = false;
			sink_nentry::type::apply_up(
				r, [&check](sink_nentry r) -> bool {
					auto e(r->get_entry());
					if (!e.active.empty()) {
						check = true;
						return false;
					} else
						return true;
				}
			);
			r->lock();
			auto e(r->get_entry());
			*((*e.loci.begin())->cond) = check ? 1 : 0;
			r->unlock();
		}
	}

	return r;
}

void dispatcher::submit_message(locus const *loc, message msg)
{
	auto r(get_locus_nentry(loc));

	if (!(*loc->cond))
		return;

	sink_nentry::type::apply_up(
		r, [loc, msg](sink_nentry r) -> bool {
			auto e(r->get_entry());
			for (auto sink : e.active)
				sink->submit_message(loc, msg);

			return true;
		}
	);
}

std::ostream &dispatcher::dump_tree(std::ostream &os)
{
	std::lock_guard<std::mutex> l_g(lock);
	return sink_nentry::type::dump(
		root, os,
		[] (sink_set const &e, decltype(os) &os_) -> decltype(os) {
			os_ << "loci: " << e.loci.size() << ", sinks a/p: "
			    << e.active.size() << '/' << e.passive.size();

			return os_;
		}
	);
}

std::ostream &dump_hierarchy(std::ostream &os)
{
	auto d(get_global_dispatcher());
	return d->dump_tree(os);
}

void dispatcher::bind_sink(std::string dest, sink *s)
{
	std::lock_guard<std::mutex> l_g(lock);
	sink_nentry a;

	sink_nentry r(sink_nentry::type::apply_to_path(
		root, dest, [&a, s](sink_nentry r) -> bool {
			auto e(r->get_entry());
			if (!a) {
				if (e.active.find(s) != e.active.end())
					a = r;
			}
			return true;
		},
		true
	));

	{
		r->lock();
		auto &e(r->get_entry());
		e.passive.emplace(s);
		r->unlock();
	}

	if (a)
		return;

	sink_nentry::type::apply_down(
		r, [s](sink_nentry r) -> bool {
			auto &e(r->get_entry());
			e.active.erase(s);

			for (auto l : e.loci)
				*(l->cond) = 1;

			return true;
		}
	);

	{
		r->lock();
		auto &e(r->get_entry());
		e.active.emplace(s);
		r->unlock();
	}
}

void dispatcher::unbind_sink(std::string dest, sink *s_ptr)
{
	std::lock_guard<std::mutex> l_g(lock);
	size_t a_count(0);
	auto r(sink_nentry::type::apply_to_path(
		root, dest, [&a_count](sink_nentry r) -> bool {
			auto &e(r->get_entry());
			a_count += e.active.size();
			return true;
		},
		false
	));

	if (!r)
		return;

	r->lock();
	auto &e(r->get_entry());
	e.passive.erase(s_ptr);
	auto iter(e.active.find(s_ptr));
	if (iter == e.active.end()) {
		r->unlock();
		return;
	} else
		e.active.erase(iter);

	if (a_count > 1) {
		r->unlock();
		return;
	}

	for (auto l : e.loci)
		*(l->cond) = 0;

	r->unlock();
	bool skip(true);

	sink_nentry::type::apply_down(
		r, [s_ptr, &skip](sink_nentry r) -> bool {
			if (skip) {
				skip = false;
				return true;
			}
			auto &e(r->get_entry());
			bool is_active(!e.active.empty());

			auto iter(e.passive.find(s_ptr));
			if (iter != e.passive.end()) {
				e.active.emplace(s_ptr);

				if (is_active)
					return false;

				r->unlock();
				sink_nentry::type::apply_down(
					r, [](sink_nentry r) -> bool {
						auto &e(r->get_entry());
						for (auto l : e.loci)
							*(l->cond) = 1;

						return true;
					}
				);
				r->lock();
				return false;
			} else {
				if (is_active)
					return false;
			}

			for (auto l : e.loci)
				*(l->cond) = 0;

			return true;
		}
	);
}

void dispatcher::unbind_sink(sink *s_ptr)
{
	std::lock_guard<std::mutex> l_g(lock);
	sink_nentry::type::apply_down(
		root, [s_ptr](sink_nentry r) -> bool {
			auto &e(r->get_entry());

			e.active.erase(s_ptr);
			e.passive.erase(s_ptr);

			if (e.active.empty()) {
				for (auto l : e.loci)
					*(l->cond) = 0;

				return true;
			} else {
				r->unlock();
				sink_nentry::type::apply_down(
					r, [s_ptr](sink_nentry r) -> bool {
						auto &e(r->get_entry());
						e.active.erase(s_ptr);
						e.passive.erase(s_ptr);
						return true;
					}
				);
				r->lock();
				return false;
			}
		}
	);
}

void dispatcher::unbind_sink()
{
	std::lock_guard<std::mutex> l_g(lock);

	sink_nentry::type::apply_down(
		root, [](sink_nentry r) -> bool {
			auto &e(r->get_entry());
			e.active.clear();
			e.passive.clear();

			for (auto l : e.loci)
				*(l->cond) = 0;

			return true;
		}
	);
}

fd_sink::~fd_sink()
{
	if (owned)
		close(fd);
}

void fd_sink::submit_message(locus const *loc, message msg)
{
	char *msg_out(nullptr);
	int msg_cnt(asprintf(
		&msg_out, "[%ld.%06ld] %s: %s\n", msg->time_stamp.tv_sec,
		msg->time_stamp.tv_nsec / 1000, msg->origin->dest,
		msg.get_extra<char>()
	));

	if (msg_cnt > 0) {
		write(fd, msg_out, msg_cnt);
		free(msg_out);
	}
}

message make_message(char const *fmt, va_list ap)
{
	va_list aq;
	va_copy(aq, ap);
	size_t len(vsnprintf(0, 0, fmt, aq));
	va_end(aq);
	auto msg(make_message(len + 1));
	vsnprintf(msg.get_extra<char>(), len + 1, fmt, ap);
	return msg;
}

message make_message(char const *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	auto msg(make_message(fmt, ap));
	va_end(ap);
	return msg;
}

void print(locus const *loc, char const *fmt, ...)
{
	if (!*(loc->cond))
		return;

	va_list ap;
	auto d(get_global_dispatcher());

	va_start(ap, fmt);
	auto msg(make_message(fmt, ap));
	va_end(ap);
	msg->set_origin(loc);
	d->submit_message(loc, msg);
}

void bind_sink(char const *dest, sink *s)
{
	auto d(get_global_dispatcher());
	d->bind_sink(dest, s);
}

void unbind_sink(char const *dest, sink *s)
{
	auto d(get_global_dispatcher());
	d->unbind_sink(dest, s);
}

void unbind_sink(sink *s)
{
	auto d(get_global_dispatcher());
	d->unbind_sink(s);
}

void unbind_sink()
{
	auto d(get_global_dispatcher());
	d->unbind_sink();
}

}

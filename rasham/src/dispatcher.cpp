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
				r, [&check](sink_nentry r_) -> bool {
					auto e(r_->get_entry());
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
		r, [loc, msg](sink_nentry r_) -> bool {
			auto e(r_->get_entry());
			for (auto sink : e.active)
				sink->submit_message(loc, msg);

			return true;
		}
	);
}

template <typename char_type, typename traits_type>
auto dispatcher::dump_tree(std::basic_ostream<char_type, traits_type> &os)
-> decltype(os) &
{
	std::lock_guard<std::mutex> l_g(lock);
	return sink_nentry::type::dump<char_type, traits_type>(
		root, os,
		[] (sink_set const &s, decltype(os) &os_) -> decltype(os) {
			os_ << "loci: " << s.loci.size() << ", sinks a/p: "
			    << s.active.size() << '/' << s.passive.size();
			return os_;
		}
	);
}

std::ostream &dump_hierarchy(std::ostream &os)
{
	auto d(get_global_dispatcher());
	return d->dump_tree(os);
}

void dispatcher::bind_sink(char const *dest, sink *s)
{
	std::lock_guard<std::mutex> l_g(lock);
	sink_nentry a;

	sink_nentry r(sink_nentry::type::apply_to_path(
		root, dest, [&a, s](sink_nentry r_) -> bool {
			auto e(r_->get_entry());
			if (!a) {
				if (e.active.find(s) != e.active.end())
					a = r_;
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
		r, [s](sink_nentry r_) -> bool {
			auto &e(r_->get_entry());
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

void dispatcher::unbind_sink()
{
	std::lock_guard<std::mutex> l_g(lock);

	sink_nentry::type::apply_down(
		root, [](sink_nentry r_) -> bool {
			auto &e(r_->get_entry());
			e.active.clear();
			e.passive.clear();

			for (auto l : e.loci)
				*(l->cond) = 0;

			return true;
		}
	);
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
/*
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
*/
void unbind_sink()
{
	auto d(get_global_dispatcher());
	d->unbind_sink();
}

}

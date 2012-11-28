/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_rasham_INTERNAL_DISPATCHER_HPP)
#define _RASHAM_INTERNAL_DISPATCHER_HPP

#include <rasham/sink.hpp>
#include <rasham/rasham.hpp>
#include <rasham/internal/nentry.hpp>

#include <unordered_set>
#include <unordered_map>

namespace rasham
{

struct sink_set {
	std::unordered_set<sink *>        active;
	std::unordered_set<sink *>        passive;
	std::unordered_set<locus const *> loci;
};

typedef nentry<sink_set> sink_nentry;

struct dispatcher {
	dispatcher(dispatcher const &) = delete;
	dispatcher &operator=(dispatcher const &) = delete;

	dispatcher();
	~dispatcher();

	void submit_message(locus const *loc, message msg);
	sink_nentry get_locus_nentry(locus const *loc);

	std::ostream &dump_tree(std::ostream &os);

	void bind_sink(std::string dest, sink *s);
	void unbind_sink(std::string dest, sink *s_ptr);
	void unbind_sink(sink *s_ptr);
	void unbind_sink();

private:
	/* All sink modifications are serialized on this lock. */
	std::mutex lock;
	sink_nentry root;
	std::unordered_map<locus const *, sink_nentry> locus_map;
};

dispatcher *get_global_dispatcher();

}

#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on implementation of userspace rcu library (http://lttng.org/urcu):
 *
 *      Copyright (c) 2009 - 2013 Mathieu Desnoyers
 *                                <mathieu.desnoyers@efficios.com>
 *      Copyright (c) 2006 - 2010 Paul E. McKenney, IBM Corporation.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <yesod/rcu.hpp>

namespace ucpf { namespace yesod { namespace rcu {
namespace detail {

std::mutex thread_entry::entry_lock;
std::list<thread_entry *> thread_entry::entries;

thread_local thread_entry rcu_reader;

std::atomic<unsigned long> gp_count = 0;

thread_entry::thread_entry()
: t_id(std::this_thread::get_id()), count(0)
{
	entry_lock.lock();
	entries.push_front(this);
	self_ref = entries.begin();
}

thread_entry::~thread_entry()
{
	entry_lock.lock();
	entries.erase(self_ref);
}

}



}}}

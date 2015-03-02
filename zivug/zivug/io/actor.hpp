/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_11109CC3D0125D55C012CFBB2EDD03C5)
#define HPP_11109CC3D0125D55C012CFBB2EDD03C5

namespace ucpf { namespace zivug { namespace io {

struct descriptor;
struct scheduler;

struct scheduler_action {
	virtual void resume_read() = 0;
	virtual void resume_write() = 0;
	virtual void wait_read() = 0;
	virtual void wait_write() = 0;
	virtual void release() = 0;

	virtual scheduler &get_scheduler() = 0;
	virtual descriptor const &get_descriptor() = 0;
};

struct actor {
	virtual void init(scheduler_action &&sa)
	{
	}

	virtual void read(
		scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
	}

	virtual void write(
		scheduler_action &&sa, bool out_of_band, bool priority
 	)
	{
	}

	virtual void error(scheduler_action &&sa, bool priority)
	{
	}

	virtual void hang_up(scheduler_action &&sa, bool read_only)
	{
	}
};

}}}
#endif

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

struct scheduler;
struct descriptor;

struct actor {
	enum {
		READ = 1,
		WRITE = 2,
		WAIT = 4
	};

	virtual int read(
		scheduler &s, descriptor const &d, bool out_of_band,
		bool priority
	)
	{
		return 0;
	}

	virtual int write(
		scheduler &s, descriptor const &d, bool out_of_band,
		bool priority
 	)
	{
		return 0;
	}

	virtual void release(scheduler &s, descriptor const &d)
	{
	}
};

}}}
#endif

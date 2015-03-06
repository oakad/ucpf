/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F99F8802BC6F8F723BA263D6A2ECC7D1)
#define HPP_F99F8802BC6F8F723BA263D6A2ECC7D1

#include <zivug/io/actor.hpp>

namespace ucpf { namespace zivug { namespace io {

struct terminating_actor : actor {
	virtual void init(scheduler_action &&sa)
	{
		sa.release();
	}

	virtual bool read(
		scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
		sa.release();
		return true;
	}

	virtual bool write(
		scheduler_action &&sa, bool out_of_band, bool priority
 	)
	{
		sa.release();
		return true;
	}

	virtual bool error(scheduler_action &&sa, bool priority)
	{
		sa.release();
		return true;
	}

	virtual bool hang_up(scheduler_action &&sa, bool read_only)
	{
		sa.release();
		return true;
	}
};

}}}
#endif

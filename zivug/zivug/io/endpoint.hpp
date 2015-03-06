/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_88A4A9DDA074FE42277E1A4CE41845B2)
#define HPP_88A4A9DDA074FE42277E1A4CE41845B2

namespace ucpf { namespace zivug { namespace io {

struct descriptor;

struct endpoint {
	virtual ~endpoint()
	{}

	virtual bool read_ready(bool out_of_band, bool priority)
	{
		return false;
	}

	virtual bool write_ready(bool out_of_band, bool priority)
	{
		return false;
	}

	virtual bool error(bool priority)
	{
		return false;
	}

	virtual bool hang_up(bool read_only)
	{
		return false;
	}
};

}}}
#endif

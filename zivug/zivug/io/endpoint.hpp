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

	virtual void read_ready(bool out_of_band, bool priority)
	{}

	virtual void write_ready(bool out_of_band, bool priority)
	{}

	virtual void error(bool priority)
	{}

	virtual void hang_up(bool read_only)
	{}
};

}}}
#endif

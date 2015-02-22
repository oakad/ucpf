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

struct actor {
	enum {
		READ = 1,
		WRITE = 2,
		ACCEPT = 4,
		WAIT = 8
	};

	virtual int read(descriptor const &d, bool out_of_band, bool priority);
	virtual int write(descriptor const &d, bool out_of_band, bool priority);
	virtual int accept(descriptor const &d, descriptor &con_d);
};

}}}
#endif

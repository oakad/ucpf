/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4FA543F2863804554BDE0F1328BAE25E)
#define HPP_4FA543F2863804554BDE0F1328BAE25E

#include <zivug/arch/io/event_dispatcher.hpp>

namespace ucpf { namespace zivug { namespace io {

template <typename Alloc>
struct scheduler {
	struct active_endp : endpoint {
	};

	struct passive_endp : endpoint {
	};

	void add_listener(endpoint e);
	void add_endpoint(endpoint e);

private:
	event_dispatcher disp;
	endpoint **listeners;
	endpoint **data_endpoints;
};

}}}
#endif

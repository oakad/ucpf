/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SLIST_NODE_OCT_21_2013_1230)
#define UCPF_YESOD_SLIST_NODE_OCT_21_2013_1230

namespace ucpf { namespace yesod {

struct slist_node {
	typedef slist_node       node;
	typedef slist_node       *node_ptr;
	typedef slist_node const *const_node_ptr;

	slist_node() : next(nullptr)
	{}

	node_ptr next;
};

}}

#endif

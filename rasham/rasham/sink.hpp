/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_SINK_HPP)
#define _RASHAM_SINK_HPP

#include <rasham/rasham.hpp>
#include <rasham/message.hpp>

namespace rasham
{

struct sink {
	sink(sink const &) = delete;
	sink &operator=(sink const &) = delete;

	virtual ~sink() {}
	virtual void submit_message(locus const *loc, message msg) = 0;

protected:
	sink() {}
};

void bind_sink(char const *dest, sink *s);
void unbind_sink(char const *dest, sink *s);
void unbind_sink(sink *s);
void unbind_sink();

std::ostream &dump_hierarchy(std::ostream &os);

}

#endif

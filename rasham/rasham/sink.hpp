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

struct sink;

void bind_sink(char const *dest, sink *s);
void unbind_sink(char const *dest, sink *s);
void unbind_sink(sink *s);
void unbind_sink();

struct sink {
	sink(sink const &) = delete;
	sink &operator=(sink const &) = delete;

	virtual ~sink()
	{
		unbind_sink(this);
	}

	virtual void submit_message(locus const *loc, message msg) = 0;

protected:
	sink() {}
};

struct fd_sink : public sink {
	fd_sink(int fd_, bool owned_)
	: fd(fd_), owned(owned_)
	{}

	virtual ~fd_sink();
	virtual void submit_message(locus const *loc, message msg);

private:
	int fd;
	bool owned;
};

struct ostream_sink : public sink {
	ostream_sink(std::ostream &os_)
	: os(os_)
	{}

	virtual ~ostream_sink() {}
	virtual void submit_message(locus const *loc, message msg);

private:
	std::ostream &os;
};

std::ostream &dump_hierarchy(std::ostream &os);

}

#endif

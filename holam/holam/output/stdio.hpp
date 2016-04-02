/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_911025C2D15566CF17ED745498F01C16)
#define HPP_911025C2D15566CF17ED745498F01C16

#include <cstdio>

namespace ucpf { namespace holam { namespace output {

struct stdio {
	typedef char value_type;

	stdio(FILE *stream_)
	: stream(stream_)
	{}

	stdio &operator=(value_type ch)
	{
		std::fputc(ch, stream);
		return *this;
	}

	stdio &operator*()
	{
		return *this;
	}

	stdio &operator++()
	{
		return *this;
	}

	stdio &operator++(int)
	{
		return *this;
	}

private:
	FILE *stream;
};

}}}
#endif

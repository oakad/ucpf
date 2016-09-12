/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_911025C2D15566CF17ED745498F01C16)
#define HPP_911025C2D15566CF17ED745498F01C16

#include <iterator>
#include <cstdio>

namespace ucpf { namespace holam { namespace output {

struct stdio {
	typedef char value_type;
	typedef void difference_type;
	typedef void pointer;
	typedef void reference;
	typedef std::output_iterator_tag iterator_category;

	stdio(FILE *stream_)
	: stream(stream_), char_count(0)
	{}

	stdio &operator=(value_type ch)
	{
		std::fputc(ch, stream);
		++char_count;
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

	std::size_t count() const
	{
		return char_count;
	}

private:
	FILE *stream;
	std::size_t char_count;
};

}}}
#endif

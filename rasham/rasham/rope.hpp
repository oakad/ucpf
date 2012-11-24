/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_ROPE_HPP)
#define _RASHAM_ROPE_HPP

#include <rasham/internal/rope_base.hpp>
#include <rasham/internal/rope_ops.hpp>

namespace rasham
{

typedef rope<char> crope;
typedef rope<wchar_t> wrope;

}

namespace std
{

template<>
struct hash<rasham::crope>
{
	size_t operator()(rasham::crope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

template<>
struct hash<rasham::wrope>
{
	size_t operator()(wrope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

}

#endif

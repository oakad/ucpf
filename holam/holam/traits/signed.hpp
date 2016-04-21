/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_CD37476353E065EA3F8A690DCB91A8DC)
#define HPP_CD37476353E065EA3F8A690DCB91A8DC

namespace ucpf { namespace holam { namespace traits {

template <typename T>
struct signed_value {
	signed_value(T &&val)
	{}

	bool implemented()
	{
		return false;
	}

	int value()
	{
		return 0;
	}
};

template <>
struct signed_value<std::int32_t> {
	signed_value(std::int32_t &&val_)
	: val(val_)
	{}

	bool implemented()
	{
		return true;
	}

	int value()
	{
		return val;
	}

	std::int32_t val;
};

}}}
#endif

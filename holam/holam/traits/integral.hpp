/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_CD37476353E065EA3F8A690DCB91A8DC)
#define HPP_CD37476353E065EA3F8A690DCB91A8DC

#include <holam/support/int128.hpp>

namespace ucpf { namespace holam { namespace traits {

template <typename T>
struct integral_value {
	typedef std::true_type disabled;

	static int apply(T &&val)
	{
		return 0;
	}
};

template <>
struct integral_value<int8_t> {
	static int8_t apply(int8_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<uint8_t> {
	static uint8_t apply(uint8_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<int16_t> {
	static int16_t apply(int16_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<uint16_t> {
	static uint16_t apply(uint16_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<int32_t> {
	static int32_t apply(int32_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<uint32_t> {
	static uint32_t apply(uint32_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<int64_t> {
	static int64_t apply(int64_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<uint64_t> {
	static uint64_t apply(uint64_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<int128_t> {
	static int128_t apply(int128_t &&val)
	{
		return val;
	}
};

template <>
struct integral_value<uint128_t> {
	static uint128_t apply(uint128_t &&val)
	{
		return val;
	}
};

}}}
#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

extern "C" {

#include <sys/types.h>
#include <sys/socket.h>

}

#include <array>
#include <zivug/linux/io_event_dispatcher.hpp>
#include <zivug/linux/io_socket_configurator.hpp>

namespace {

template <typename ValueType, int Level, int OptName>
struct option;

template <int Level, int OptName>
struct option<int, Level, OptName> {
	static void set(ucpf::zivug::io::descriptor &d, char const *val)
	{
		int x_val;

		auto rc(setsocopt(
			d.native(), Level, OptName, &x_val, sizeof(x_val)
		));
		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

struct static_hash_base {
};

template <int Level>
struct option_symbols_t;

template <>
struct option_symbols_t<SOL_SOCKET> : static_hash_base {
	constexpr static uint32_t seed = 0x014eb0d6;
	constexpr static uint32_t order = 5;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	constexpr static std::array<
		std::pair<
			char const *, void (*)(
				ucpf::zivug::io::descriptor &d,
				char const *val
			)
		>, (std::size_t(1) << order)
	> symbols = {{
		{"sndbuf", &option<int, SOL_SOCKET, SO_SNDBUF>::set},
		{"dontroute", &option<int, SOL_SOCKET, SO_DONTROUTE>::set}
		{nullptr, nullptr},
		{"oobinline", SO_OOBINLINE},
		{"keepalive", SO_KEEPALIVE},
		{"linger_sec", SO_LINGER_SEC},
		{"nread", SO_NREAD},
		{nullptr, nullptr},
		{"sndlowat", SO_SNDLOWAT},
		{nullptr, nullptr},
		{"nosigpipe", SO_NOSIGPIPE},
		{"reuseaddr", SO_REUSEADDR},
		{"broadcast", SO_BROADCAST},
		{"rcvbuf", SO_RCVBUF},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{"sndtimeo", SO_SNDTIMEO},
		{nullptr, nullptr},
		{"type", SO_TYPE},
		{"nwrite", SO_NWRITE},
		{"rcvtimeo", SO_RCVTIMEO},
		{"linger", SO_LINGER},
		{"reuseport", SO_REUSEPORT},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{nullptr, nullptr},
		{"debug", SO_DEBUG},
		{"rcvlowat", SO_RCVLOWAT},
		{"error", SO_ERROR}
	}};
} option_symbols;

}

namespace ucpf { namespace zivug { namespace io {
}}}

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
#include <yesod/coder/xxhash.hpp>
#include <zivug/linux/io_event_dispatcher.hpp>
#include <zivug/linux/io_socket_configurator.hpp>

namespace {

template <typename ValueType, int Level, int OptName>
struct option {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
	}
};

template <int Level, int OptName>
struct option<int, Level, OptName> {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		int x_val(0);

		auto rc(::setsockopt(
			d.native(), Level, OptName, &x_val, sizeof(x_val)
		));
		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

template <int Level>
struct option_symbols;

template <>
struct option_symbols<SOL_SOCKET> {
	constexpr static int level = SOL_SOCKET;
	constexpr static uint32_t seed = 0x13e1d776;
	constexpr static uint32_t order = 5;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		void (*func)(
			ucpf::zivug::io::descriptor &d,
			char const *val_first, char const *val_last
		);
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
		{nullptr, 0, nullptr},
		{"broadcast", 9, &option<int, level, SO_BROADCAST>::set},
		{"passcred", 8, &option<int, level, SO_PASSCRED>::set},
		{"reuseport", 9, &option<int, level, SO_REUSEPORT>::set},
		{"sndtimeo", 8, &option<::timeval, level, SO_SNDTIMEO>::set},
		{nullptr, 0, nullptr},
		{"sndbuf", 6, &option<int, level, SO_SNDBUF>::set},
		{"keepalive", 9, &option<int, level, SO_KEEPALIVE>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"priority", 8, &option<int, level, SO_PRIORITY>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"linger", 6, &option<::linger, level, SO_LINGER>::set},
		{"reuseaddr", 9, &option<int, level, SO_REUSEADDR>::set},
		{"type", 4, &option<void, level, SO_TYPE>::set},
		{"error", 5, &option<void, level, SO_ERROR>::set},
		{"debug", 5, &option<int, level, SO_DEBUG>::set},
		{nullptr, 0, nullptr},
		{"sndbufforce", 11, &option<int, level, SO_SNDBUFFORCE>::set},
		{"rcvbuf", 6, &option<int, level, SO_RCVBUF>::set},
		{nullptr, 0, nullptr},
		{"rcvtimeo", 8, &option<::timeval, level, SO_RCVTIMEO>::set},
		{"no_check", 8, &option<int, level, SO_NO_CHECK>::set},
		{"bsdcompat", 9, &option<int, level, SO_BSDCOMPAT>::set},
		{nullptr, 0, nullptr},
		{"oobinline", 9, &option<int, level, SO_OOBINLINE>::set},
		{"rcvlowat", 8, &option<int, level, SO_RCVLOWAT>::set},
		{"rcvbufforce", 11, &option<int, level, SO_RCVBUFFORCE>::set},
		{"peercred", 8, &option<int, level, SO_PEERCRED>::set},
		{"dontroute", 9, &option<int, level, SO_DONTROUTE>::set},
		{"sndlowat", 8, &option<int, level, SO_SNDLOWAT>::set}
	}};

	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *name_first, char const *name_last,
		char const *val_first, char const *val_last
	)
	{
		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(name_first, name_last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);

		if (!std::equal(
			name_first, name_last, e.name, e.name + e.name_sz
		))
			return;

		e.func(d, val_first, val_last);
	}
};

}

namespace ucpf { namespace zivug { namespace io {
}}}

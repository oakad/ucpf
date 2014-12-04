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
		char const *opt_first, char const *opt_last
	)
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

template <int Level, int OptName>
struct option<int, Level, OptName> {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *opt_first, char const *opt_last
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
struct option_symbols {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *opt_first, char const *opt_last,
	)
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

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
			char const *opt_first, char const *opt_last
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
		char const *opt_first, char const *opt_last
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

struct level_symbols {
	constexpr static uint32_t seed = 0x97cc1370;
	constexpr static uint32_t order = 6;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		void (*func)(
			ucpf::zivug::io::descriptor &d,
			char const *opt_first, char const *opt_last
		);
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
		{"udplite", 7, &option_symbols<SOL_UDPLITE>::set},
		{"rose", 4, &option_symbols<SOL_ROSE>::set},
		{"udp", 3, &option_symbols<SOL_UDP>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"ipx", 3, &option_symbols<SOL_IPX>::set},
		{nullptr, 0, nullptr},
		{"pppol2tp", 8, &option_symbols<SOL_PPPOL2TP>::set},
		{"ipv6", 4, &option_symbols<SOL_IPV6>::set},
		{"atm", 3, &option_symbols<SOL_ATM>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"icmpv6", 6, &option_symbols<SOL_ICMPV6>::set},
		{"caif", 4, &option_symbols<SOL_CAIF>::set},
		{nullptr, 0, nullptr},
		{"x25", 3, &option_symbols<SOL_X25>::set},
		{nullptr, 0, nullptr},
		{"alg", 3, &option_symbols<SOL_ALG>::set},
		{"rxrpc", 5, &option_symbols<SOL_RXRPC>::set},
		{"nfc", 3, &option_symbols<SOL_NFC>::set},
		{"packet", 6, &option_symbols<SOL_PACKET>::set},
		{"decnet", 6, &option_symbols<SOL_DECNET>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"raw", 3, &option_symbols<SOL_RAW>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"tcp", 3, &option_symbols<SOL_TCP>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"atalk", 5, &option_symbols<SOL_ATALK>::set},
		{nullptr, 0, nullptr},
		{"netbeui", 7, &option_symbols<SOL_NETBEUI>::set},
		{"ax25", 4, &option_symbols<SOL_AX25>::set},
		{nullptr, 0, nullptr},
		{"llc", 3, &option_symbols<SOL_LLC>::set},
		{"rds", 3, &option_symbols<SOL_RDS>::set},
		{"pnpipe", 6, &option_symbols<SOL_PNPIPE>::set},
		{nullptr, 0, nullptr},
		{"ip", 2, &option_symbols<SOL_IP>::set},
		{nullptr, 0, nullptr},
		{"irda", 4, &option_symbols<SOL_IRDA>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"icmp", 4, &option_symbols<SOL_ICMP>::set},
		{"bluetooth", 9, &option_symbols<SOL_BLUETOOTH>::set},
		{"netlink", 7, &option_symbols<SOL_NETLINK>::set},
		{"sctp", 4, &option_symbols<SOL_SCTP>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"iucv", 4, &option_symbols<SOL_IUCV>::set},
		{"aal", 3, &option_symbols<SOL_AAL>::set},
		{"netrom", 6, &option_symbols<SOL_NETROM>::set},
		{nullptr, 0, nullptr},
		{nullptr, 0, nullptr},
		{"dccp", 4, &option_symbols<SOL_DCCP>::set},
		{"tipc", 4, &option_symbols<SOL_TIPC>::set},
	}};

	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *opt_first, char const *opt_last
	)
	{
	}
};

}

namespace ucpf { namespace zivug { namespace io {
}}}

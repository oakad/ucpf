/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_so.hpp"

#if !defined(SO_BPF_EXTENSIONS)
#define SO_BPF_EXTENSIONS 48
#endif

#if !defined(SO_INCOMING_CPU)
#define SO_INCOMING_CPU 49
#endif

#if !defined(SO_ATTACH_BPF)
#define SO_ATTACH_BPF 50
#endif

#if !defined(SO_DETACH_BPF)
#define SO_DETACH_BPF SO_DETACH_FILTER
#endif

namespace {

#include "symbols/sol_socket_option_map.hpp"

using ucpf::zivug::io::detail::string_tag;
using ucpf::zivug::io::detail::option_base;
using ucpf::zivug::io::detail::option_entry;

constexpr option_base const *registry[] = {
	option_entry<bool, SOL_SOCKET, SO_DEBUG>::impl,
	option_entry<bool, SOL_SOCKET, SO_REUSEADDR>::impl,
	option_entry<int, SOL_SOCKET, SO_TYPE>::impl,
	option_entry<int, SOL_SOCKET, SO_ERROR>::impl,
	option_entry<bool, SOL_SOCKET, SO_DONTROUTE>::impl,
	option_entry<bool, SOL_SOCKET, SO_BROADCAST>::impl,
	option_entry<int, SOL_SOCKET, SO_SNDBUF>::impl,
	option_entry<int, SOL_SOCKET, SO_RCVBUF>::impl,
	option_entry<int, SOL_SOCKET, SO_SNDBUFFORCE>::impl,
	option_entry<int, SOL_SOCKET, SO_RCVBUFFORCE>::impl,
	option_entry<bool, SOL_SOCKET, SO_KEEPALIVE>::impl,
	option_entry<bool, SOL_SOCKET, SO_OOBINLINE>::impl,
	option_entry<bool, SOL_SOCKET, SO_NO_CHECK>::impl,
	option_entry<int, SOL_SOCKET, SO_PRIORITY>::impl,
	option_entry<::linger, SOL_SOCKET, SO_LINGER>::impl,
	option_entry<void, SOL_SOCKET, SO_BSDCOMPAT>::impl,
	option_entry<bool, SOL_SOCKET, SO_REUSEPORT>::impl,
	option_entry<bool, SOL_SOCKET, SO_PASSCRED>::impl,
	option_entry<::ucred, SOL_SOCKET, SO_PEERCRED>::impl,
	option_entry<int, SOL_SOCKET, SO_RCVLOWAT>::impl,
	option_entry<int, SOL_SOCKET, SO_SNDLOWAT>::impl,
	option_entry<::timeval, SOL_SOCKET, SO_RCVTIMEO>::impl,
	option_entry<::timeval, SOL_SOCKET, SO_SNDTIMEO>::impl,
	option_entry<void, SOL_SOCKET, SO_SECURITY_AUTHENTICATION>::impl,
	option_entry<void, SOL_SOCKET, SO_SECURITY_ENCRYPTION_TRANSPORT>::impl,
	option_entry<void, SOL_SOCKET, SO_SECURITY_ENCRYPTION_NETWORK>::impl,
	option_entry<string_tag, SOL_SOCKET, SO_BINDTODEVICE>::impl,
	option_entry<::sock_fprog, SOL_SOCKET, SO_ATTACH_FILTER>::impl,
	option_entry<bool, SOL_SOCKET, SO_DETACH_FILTER>::impl,
	option_entry<::sock_fprog, SOL_SOCKET, SO_GET_FILTER>::impl,
	option_entry<string_tag, SOL_SOCKET, SO_PEERNAME>::impl,
	option_entry<bool, SOL_SOCKET, SO_TIMESTAMP>::impl,
	option_entry<int, SOL_SOCKET, SO_ACCEPTCONN>::impl,
	option_entry<string_tag, SOL_SOCKET, SO_PEERSEC>::impl,
	option_entry<bool, SOL_SOCKET, SO_PASSSEC>::impl,
	option_entry<bool, SOL_SOCKET, SO_TIMESTAMPNS>::impl,
	option_entry<int, SOL_SOCKET, SO_MARK>::impl,
	option_entry<int, SOL_SOCKET, SO_TIMESTAMPING>::impl,
	option_entry<int, SOL_SOCKET, SO_PROTOCOL>::impl,
	option_entry<int, SOL_SOCKET, SO_DOMAIN>::impl,
	option_entry<bool, SOL_SOCKET, SO_RXQ_OVFL>::impl,
	option_entry<bool, SOL_SOCKET, SO_WIFI_STATUS>::impl,
	option_entry<int, SOL_SOCKET, SO_PEEK_OFF>::impl,
	option_entry<bool, SOL_SOCKET, SO_NOFCS>::impl,
	option_entry<bool, SOL_SOCKET, SO_LOCK_FILTER>::impl,
	option_entry<bool, SOL_SOCKET, SO_SELECT_ERR_QUEUE>::impl,
	option_entry<int, SOL_SOCKET, SO_BUSY_POLL>::impl,
	option_entry<int, SOL_SOCKET, SO_MAX_PACING_RATE>::impl,
	option_entry<int, SOL_SOCKET, SO_BPF_EXTENSIONS>::impl,
	option_entry<int, SOL_SOCKET, SO_INCOMING_CPU>::impl,
	option_entry<int, SOL_SOCKET, SO_ATTACH_BPF>::impl,
	option_entry<bool, SOL_SOCKET, SO_DETACH_BPF>::impl
};

}

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <int Level>
struct socket_level : socket_level_base {};

template <>
struct socket_level<SOL_SOCKET> : socket_level_base {
	virtual option_base const *option_from_string(
		char const *first, char const *last
	) const;
};

option_base const *socket_level<SOL_SOCKET>::option_from_string(
	char const *first, char const *last
) const
{
	auto idx(sol_socket_option_map::find(first, last));
	if (idx)
		return registry[idx - 1];
	else
		throw std::system_error(ENOPROTOOPT, std::system_category());
}

}}}}

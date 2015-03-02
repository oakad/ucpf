/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_so.hpp"

#if !defined(SOL_IP)
#define SOL_IP 0
#endif

#if !defined(SOL_TCP)
#define SOL_TCP 6
#endif

#if !defined(SOL_UDP)
#define SOL_UDP 17
#endif

#if !defined(SOL_IPV6)
#define SOL_IPV6 41
#endif

#if !defined(SOL_ICMPV6)
#define SOL_ICMPV6 58
#endif

#if !defined(SOL_SCTP)
#define SOL_SCTP 132
#endif

#if !defined(SOL_UDPLITE)
#define SOL_UDPLITE 136
#endif

#if !defined(SOL_IPX)
#define SOL_IPX 256
#endif

#if !defined(SOL_AX25)
#define SOL_AX25 257
#endif

#if !defined(SOL_ATALK)
#define SOL_ATALK 258
#endif

#if !defined(SOL_NETROM)
#define SOL_NETROM 259
#endif

#if !defined(SOL_ROSE)
#define SOL_ROSE 260
#endif

#if !defined(SOL_NETBEUI)
#define SOL_NETBEUI 267
#endif

#if !defined(SOL_LLC)
#define SOL_LLC 268
#endif

#if !defined(SOL_DCCP)
#define SOL_DCCP 269
#endif

#if !defined(SOL_NETLINK)
#define SOL_NETLINK 270
#endif

#if !defined(SOL_TIPC)
#define SOL_TIPC 271
#endif

#if !defined(SOL_RXRPC)
#define SOL_RXRPC 272
#endif

#if !defined(SOL_PPPOL2TP)
#define SOL_PPPOL2TP 273
#endif

#if !defined(SOL_BLUETOOTH)
#define SOL_BLUETOOTH 274
#endif

#if !defined(SOL_PNPIPE)
#define SOL_PNPIPE 275
#endif

#if !defined(SOL_RDS)
#define SOL_RDS 276
#endif

#if !defined(SOL_IUCV)
#define SOL_IUCV 277
#endif

#if !defined(SOL_CAIF)
#define SOL_CAIF 278
#endif

#if !defined(SOL_ALG)
#define SOL_ALG 279
#endif

#if !defined(SOL_NFC)
#define SOL_NFC 280
#endif

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <int Level>
struct socket_level : socket_level_base {};

extern template struct socket_level<SOL_SOCKET>;

}}}}

using ucpf::zivug::io::detail::socket_level;
using ucpf::zivug::io::detail::socket_level_base;

namespace {

#include "symbols/socket_level_map.hpp"

template <int Level>
struct socket_level_entry {
	constexpr static socket_level<Level> sol = {};
	constexpr static socket_level_base const *impl = &sol;
};

template <int Level>
constexpr socket_level<Level> socket_level_entry<Level>::sol;

constexpr socket_level_base const *registry[] = {
	socket_level_entry<SOL_SOCKET>::impl,
	socket_level_entry<SOL_IP>::impl,
	socket_level_entry<SOL_TCP>::impl,
	socket_level_entry<SOL_UDP>::impl,
	socket_level_entry<SOL_IPV6>::impl,
	socket_level_entry<SOL_ICMPV6>::impl,
	socket_level_entry<SOL_SCTP>::impl,
	socket_level_entry<SOL_UDPLITE>::impl,
	socket_level_entry<SOL_RAW>::impl,
	socket_level_entry<SOL_IPX>::impl,
	socket_level_entry<SOL_AX25>::impl,
	socket_level_entry<SOL_ATALK>::impl,
	socket_level_entry<SOL_NETROM>::impl,
	socket_level_entry<SOL_ROSE>::impl,
	socket_level_entry<SOL_DECNET>::impl,
	socket_level_entry<SOL_X25>::impl,
	socket_level_entry<SOL_PACKET>::impl,
	socket_level_entry<SOL_ATM>::impl,
	socket_level_entry<SOL_AAL>::impl,
	socket_level_entry<SOL_IRDA>::impl,
	socket_level_entry<SOL_NETBEUI>::impl,
	socket_level_entry<SOL_LLC>::impl,
	socket_level_entry<SOL_DCCP>::impl,
	socket_level_entry<SOL_NETLINK>::impl,
	socket_level_entry<SOL_TIPC>::impl,
	socket_level_entry<SOL_RXRPC>::impl,
	socket_level_entry<SOL_PPPOL2TP>::impl,
	socket_level_entry<SOL_BLUETOOTH>::impl,
	socket_level_entry<SOL_PNPIPE>::impl,
	socket_level_entry<SOL_RDS>::impl,
	socket_level_entry<SOL_IUCV>::impl,
	socket_level_entry<SOL_CAIF>::impl,
	socket_level_entry<SOL_ALG>::impl,
	socket_level_entry<SOL_NFC>::impl
};

}

namespace ucpf { namespace zivug { namespace io { namespace detail {

socket_level_base const *socket_level_base::level_from_string(
	char const *first, char const *last
)
{
	auto idx(socket_level_map::find(first, last));
	if (idx)
		return registry[idx - 1];
	else
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
}

}}}}

/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_af.hpp"

namespace ucpf { namespace zivug { namespace io { namespace detail {

extern template struct family<AF_INET>;
extern template struct family<AF_INET6>;

}}}}

using ucpf::zivug::io::detail::family;
using ucpf::zivug::io::detail::family_base;

namespace {

#include "symbols/address_family_map.hpp"

template <int AddrFamily>
struct family_entry {
	constexpr static family<AddrFamily> af = {};
	constexpr static family_base const *impl = &af;
};

template <int AddrFamily>
constexpr family<AddrFamily> family_entry<AddrFamily>::af;

constexpr static family_base const *registry[] = {
	family_entry<AF_UNIX>::impl,
	family_entry<AF_LOCAL>::impl,
	family_entry<AF_INET>::impl,
	family_entry<AF_AX25>::impl,
	family_entry<AF_IPX>::impl,
	family_entry<AF_APPLETALK>::impl,
	family_entry<AF_NETROM>::impl,
	family_entry<AF_BRIDGE>::impl,
	family_entry<AF_ATMPVC>::impl,
	family_entry<AF_X25>::impl,
	family_entry<AF_INET6>::impl,
	family_entry<AF_ROSE>::impl,
	family_entry<AF_DECNET>::impl,
	family_entry<AF_NETBEUI>::impl,
	family_entry<AF_SECURITY>::impl,
	family_entry<AF_KEY>::impl,
	family_entry<AF_NETLINK>::impl,
	family_entry<AF_ROUTE>::impl,
	family_entry<AF_PACKET>::impl,
	family_entry<AF_ASH>::impl,
	family_entry<AF_ECONET>::impl,
	family_entry<AF_ATMSVC>::impl,
	family_entry<AF_RDS>::impl,
	family_entry<AF_SNA>::impl,
	family_entry<AF_IRDA>::impl,
	family_entry<AF_PPPOX>::impl,
	family_entry<AF_WANPIPE>::impl,
	family_entry<AF_LLC>::impl,
	family_entry<AF_IB>::impl,
	family_entry<AF_CAN>::impl,
	family_entry<AF_TIPC>::impl,
	family_entry<AF_BLUETOOTH>::impl,
	family_entry<AF_IUCV>::impl,
	family_entry<AF_RXRPC>::impl,
	family_entry<AF_ISDN>::impl,
	family_entry<AF_PHONET>::impl,
	family_entry<AF_IEEE802154>::impl,
	family_entry<AF_CAIF>::impl,
	family_entry<AF_ALG>::impl,
	family_entry<AF_NFC>::impl,
	family_entry<AF_VSOCK>::impl
};

}

namespace ucpf { namespace zivug { namespace io { namespace address_family {

family_base const *from_string(char const *first, char const *last)
{
	auto idx(address_family_map::find(first, last));
	if (idx)
		return registry[idx - 1];
	else
		throw std::system_error(
			EAFNOSUPPORT, std::system_category()
		);
}

}}}}

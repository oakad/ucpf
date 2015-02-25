/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

extern "C" {

#include <sys/socket.h>

}

#include "io_socket_so.hpp"
#include <zivug/linux/io/address_family.hpp>

#if !defined(AF_IB)
#define AF_IB 27
#endif

namespace ucpf { namespace zivug { namespace io {

template <::sa_family_t AddrFamily>
struct address_family_inst : address_family
{};

//extern template struct address_family_inst<AF_INET>;
//extern template struct address_family_inst<AF_INET6>;

}}}

using ucpf::zivug::io::address_family;
using ucpf::zivug::io::address_family_inst;

namespace {

#include "symbols/address_family_map.hpp"
#include "symbols/socket_type_map.hpp"

constexpr int registry_type[] = {
	SOCK_STREAM,
	SOCK_DGRAM,
	SOCK_RAW,
	SOCK_RDM,
	SOCK_SEQPACKET,
	SOCK_DCCP,
	SOCK_PACKET
};

template <::sa_family_t AddrFamily>
struct family_entry {
	constexpr static address_family_inst<AddrFamily> af = {};
	constexpr static address_family const *impl = &af;
};

template <::sa_family_t AddrFamily>
constexpr address_family_inst<AddrFamily> family_entry<AddrFamily>::af;

constexpr address_family const *registry_af[] = {
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
	family_entry<AF_DECnet>::impl,
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

namespace ucpf { namespace zivug { namespace io {

std::pair<descriptor, address_family const &> address_family::make_descriptor(
	char const *first, char const *last
)
{
	/* family.type || family.type.protocol */
	auto af_last(first);
	while ((af_last != last) && (*af_last != '.'))
		++af_last;

	auto idx(address_family_map::find(first, last));
	if (!idx)
		throw std::system_error(
			EAFNOSUPPORT, std::system_category()
		);

	auto af(registry_af[idx - 1]);

	auto type_first(af_last);
	if (af_last != last)
		++type_first;
	else
		throw std::system_error(EINVAL, std::system_category());

	auto type_last(type_first);
	while ((type_last != last) && (*type_last != '.'))
		++type_last;

	idx = socket_type_map::find(type_first, type_last);
	if (!idx)
		throw std::system_error(
			ESOCKTNOSUPPORT, std::system_category()
		);

	auto s_type(registry_type[idx - 1]);

	auto proto_first(type_last);
	if (proto_first != last)
		++proto_first;

	return std::make_pair(
		af->create(s_type, proto_first, last), *af
	);
}

void address_family::set_option(
	descriptor const &d, char const *first, char const *last
) const
{
	/* socket_level.option = val */
	auto sol_last(first);
	while ((sol_last != last) && (*sol_last != '.'))
		++sol_last;
	auto sol(detail::socket_level_base::level_from_string(first, sol_last));

	auto opt_first(sol_last);
	if (opt_first != last)
		++opt_first;

	auto opt_last(opt_first);
	while (
		(opt_last != last)
		&& !std::isspace(*opt_last)
		&& (*opt_last != '=')
	)
		++opt_last;
	auto opt(sol->option_from_string(opt_first, opt_last));

	auto val_first(opt_last);
	if (val_first != last)
		++val_first;

	while ((val_first != last) && std::isspace(*val_first))
		++val_first;

	opt->set(d, val_first, last);
}

}}}

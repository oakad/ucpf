/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_A627ACC3D63E51B43D8F825BBFAC5226)
#define HPP_A627ACC3D63E51B43D8F825BBFAC5226

/* GCC bug 52036 */
struct address_family_list {
	constexpr static ::sa_family_t list[] = {
		AF_UNIX,
		AF_LOCAL,
		AF_INET,
		AF_AX25,
		AF_IPX,
		AF_APPLETALK,
		AF_NETROM,
		AF_BRIDGE,
		AF_ATMPVC,
		AF_X25,
		AF_INET6,
		AF_ROSE,
		AF_DECnet,
		AF_NETBEUI,
		AF_SECURITY,
		AF_KEY,
		AF_NETLINK,
		AF_ROUTE,
		AF_PACKET,
		AF_ASH,
		AF_ECONET,
		AF_ATMSVC,
		AF_RDS,
		AF_SNA,
		AF_IRDA,
		AF_PPPOX,
		AF_WANPIPE,
		AF_LLC,
		AF_IB,
		AF_CAN,
		AF_TIPC,
		AF_BLUETOOTH,
		AF_IUCV,
		AF_RXRPC,
		AF_ISDN,
		AF_PHONET,
		AF_IEEE802154,
		AF_CAIF,
		AF_ALG,
		AF_NFC,
		AF_VSOCK
	};
};

#endif

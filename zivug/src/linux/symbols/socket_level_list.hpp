/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_B3907C23BCB447987D42DF0D298DC14B)
#define HPP_B3907C23BCB447987D42DF0D298DC14B

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

constexpr static int socket_level_list[] = {
	SOL_SOCKET,
	SOL_IP,
	SOL_TCP,
	SOL_UDP,
	SOL_IPV6,
	SOL_ICMPV6,
	SOL_SCTP,
	SOL_UDPLITE,
	SOL_RAW,
	SOL_IPX,
	SOL_AX25,
	SOL_ATALK,
	SOL_NETROM,
	SOL_ROSE,
	SOL_DECNET,
	SOL_X25,
	SOL_PACKET,
	SOL_ATM,
	SOL_AAL,
	SOL_IRDA,
	SOL_NETBEUI,
	SOL_LLC,
	SOL_DCCP,
	SOL_NETLINK,
	SOL_TIPC,
	SOL_RXRPC,
	SOL_PPPOL2TP,
	SOL_BLUETOOTH,
	SOL_PNPIPE,
	SOL_RDS,
	SOL_IUCV,
	SOL_CAIF,
	SOL_ALG,
	SOL_NFC
};

#endif

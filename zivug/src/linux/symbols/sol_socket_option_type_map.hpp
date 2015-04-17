/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_8A08FFD1F7D4E9FEBB0A86132C9733E0)
#define HPP_8A08FFD1F7D4E9FEBB0A86132C9733E0

typedef int sol_socket_option_default_type;

typedef mpl::map<
	mpl::pair<mpl::int_<SO_LINGER>, ::linger>,
	mpl::pair<mpl::int_<SO_BSDCOMPAT>, void>,
	mpl::pair<mpl::int_<SO_PEERCRED>, ::ucred>,
	mpl::pair<mpl::int_<SO_RCVTIMEO>, ::timeval>,
	mpl::pair<mpl::int_<SO_SNDTIMEO>, ::timeval>,
	mpl::pair<mpl::int_<SO_SECURITY_AUTHENTICATION>, void>,
	mpl::pair<mpl::int_<SO_SECURITY_ENCRYPTION_TRANSPORT>, void>,
	mpl::pair<mpl::int_<SO_SECURITY_ENCRYPTION_NETWORK>, void>,
	mpl::pair<mpl::int_<SO_BINDTODEVICE>, string_tag>,
	mpl::pair<mpl::int_<SO_ATTACH_FILTER>, ::sock_fprog>,
	mpl::pair<mpl::int_<SO_GET_FILTER>, ::sock_fprog>,
	mpl::pair<mpl::int_<SO_PEERNAME>, string_tag>,
	mpl::pair<mpl::int_<SO_PEERSEC>, string_tag>
> sol_socket_option_type_map;

#endif

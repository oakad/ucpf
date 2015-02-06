/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_F338EFF8ACCEEA01F492438B941B0657)
#define HPP_F338EFF8ACCEEA01F492438B941B0657

#include <zivug/config_base.hpp>

namespace ucpf { namespace zivug { namespace config {

template <typename Alloc>
struct io_server_socket {
	template <typename Packager>
	void mina_pack(Packager &p)
	{
		p(MINA_NPP(
			protocol, bind_address, pre_bind_options,
			post_bind_options
		));
	}

	io_server_socket(Alloc const &a = Alloc())
	: protocol(a), bind_address(a), pre_bind_options(a),
	  post_bind_options(a)
	{}

	string<Alloc> protocol;
	string<Alloc> bind_address;
	string_vec<Alloc> pre_bind_options;
	string_vec<Alloc> post_bind_options;
};

}}}

#endif

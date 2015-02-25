/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_F338EFF8ACCEEA01F492438B941B0657)
#define HPP_F338EFF8ACCEEA01F492438B941B0657

#include <zivug/config/base.hpp>

namespace ucpf { namespace zivug { namespace config {

template <typename Alloc>
struct io_server_socket {
	template <typename Packager>
	void mina_pack(Packager &p)
	{
		p(MINA_NPP(
			protocol, bind_address, pre_bind_options,
			post_bind_options, listen_backlog
		));
	}

	io_server_socket(Alloc const &a = Alloc())
	: protocol(a), bind_address(a), pre_bind_options(a),
	  post_bind_options(a), listen_backlog(0)
	{}

	string<Alloc> protocol;
	string<Alloc> bind_address;
	string_vec<Alloc> pre_bind_options;
	string_vec<Alloc> post_bind_options;
	int listen_backlog;
};

template <typename Alloc>
struct io_server {
	template <typename Packager>
	void mina_pack(Packager &p)
	{
		p(MINA_NPP(epoll, sockets));
	}

	io_server(Alloc const &a = Alloc())
	: sockets(a)
	{}

	struct {
		template <typename Packager>
		void mina_pack(Packager &p)
		{
			p(MINA_NPP(timeout_ms, event_count));
		}

		int timeout_ms;
		int event_count;
	} epoll;
	std::vector<
		io_server_socket<Alloc>,
		alloc_t<io_server_socket<Alloc>, Alloc>
	> sockets;
};

}}}
#endif

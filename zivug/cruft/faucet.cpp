/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <mina/store/gdbm.hpp>
#include <mina/np_packager.hpp>
#include <mina/text_store_adaptor.hpp>

#include <zivug/arch/io_server.hpp>
#include <zivug/arch/io_server_config.hpp>

#include <zivug/io/scheduler.hpp>

namespace mi = ucpf::mina;
namespace ms = ucpf::mina::store;
namespace zc = ucpf::zivug::config;
namespace zi = ucpf::zivug::io;

int main(int argc, char **argv)
{
	typedef std::allocator<void> a_type;
	a_type a;

	scheduler<a_type> s(a);

	ms::gdbm gs("faucet.cfg");

	mi::np_packager<
		mi::text_store_adaptor<decltype(gs), a_type>
	> pack(gs, a);

	zc::io_server<a_type> srv_cfg(a);
	pack.restore({"server"}, srv_cfg);

	printf("aa %s\n", srv_cfg.sockets[0].pre_bind_options[0].c_str());
	return 0;
}

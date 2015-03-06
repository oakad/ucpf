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

#include <zivug/arch/config/io_server.hpp>
#include <zivug/arch/io/address_family.hpp>

#include <zivug/io/scheduler.hpp>

namespace mi = ucpf::mina;
namespace ms = ucpf::mina::store;
namespace zc = ucpf::zivug::config;
namespace zi = ucpf::zivug::io;

struct server_actor : zi::actor {
	virtual void init(zi::scheduler_action &sa)
	{
		sa.wait_read();
	}

	virtual bool read(
		zi::scheduler_action &sa, bool out_of_band, bool priority
	)
	{
		zi::descriptor d(
		return false;
	}
};

int main(int argc, char **argv)
{
	typedef std::allocator<void> a_type;
	a_type a;

	ms::gdbm gs("faucet.cfg");

	mi::np_packager<
		mi::text_store_adaptor<decltype(gs), a_type>
	> pack(gs, a);

	zc::io_server<a_type> srv_cfg(a);
	pack.restore({"server"}, srv_cfg);

	zi::rr_scheduler<a_type> sched(srv_cfg, a);
	server_actor srv_actor;

	for (auto const &s: srv_cfg.sockets) {
		auto dp(zi::address_family::make_descriptor(
			std::begin(s.protocol).base(),
			std::end(s.protocol).base()
		));

		for (auto const &opt: s.pre_bind_options)
			dp.second->set_option(
				dp.first, std::begin(opt).base(),
				std::end(opt).base()
			);

		dp.second->bind(
			dp.first, std::begin(s.bind_address).base(),
			std::end(s.bind_address).base()
		);

		for (auto const &opt: s.post_bind_options)
			dp.second->set_option(
				dp.first, std::begin(opt).base(),
				std::end(opt).base()
			);

		dp.second->listen(dp.first, s.listen_backlog);

		sched.imbue(std::move(dp.first), srv_actor);
	}

	return 0;
}

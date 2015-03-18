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

struct server_actor;

struct receiver_actor : zi::splice_input_actor {
	receiver_actor(
		server_actor &parent_, descriptor &&dst_d_,
		std::size_t block_size_
	) : zi::splice_input_actor(
		std::forward<descriptor>(dst_d_), block_size_
	), parent(parent_)
	{}

	virtual void fini(scheduler_action &&sa)
	{
		parent.conn_finished(std::forward<scheduler_action>(sa));
	}

	server_actor &parent;
};

struct server_actor : zi::actor {
	server_actor(zi::address_family const *af_)
	: af(af_)
	{}

	virtual void init(zi::scheduler_action &sa)
	{
		sa.wait_read();
	}

	virtual bool read(
		zi::scheduler_action &sa, bool out_of_band, bool priority
	)
	{
		return false;
	}

	void conn_finished(scheduler_action &&sa)
	{
		sa.resume_read(acceptor_tk);
	}

private:
	zi::address_family const *af;
	zi::scheduler_token acceptor_tk;
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
	std::vector<server_actor> srv_v;

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
		srv_v.emplace_back(dp.second);
		sched.imbue(std::move(dp.first), srv_v.back());
	}

	return 0;
}

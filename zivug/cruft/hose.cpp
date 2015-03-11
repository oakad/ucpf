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
#include <zivug/io/terminating_actor.hpp>
#include <zivug/arch/io/splice_output_actor.hpp>

#include <zivug/io/scheduler.hpp>

extern "C" {

#include <sys/socket.h>

}

namespace mi = ucpf::mina;
namespace ms = ucpf::mina::store;
namespace zc = ucpf::zivug::config;
namespace zi = ucpf::zivug::io;

struct connector_actor : zi::actor {
	connector_actor(zi::actor &act_)
	: act(act_)
	{}

	virtual void init(zi::scheduler_action &&sa, bool new_desc)
	{
		printf("--1- init\n");
		sa.wait_write();
	}

	virtual bool hang_up(zi::scheduler_action &&sa, bool read_only)
	{
		printf("--1- hang_up\n");
		sa.set_actor(t_act);
		return true;
	}

	virtual bool error(zi::scheduler_action &&sa, bool priority)
	{
		int err(0);
		::socklen_t optlen(sizeof(err));

		if (!::getsockopt(
			sa.get_descriptor().native(), SOL_SOCKET, SO_ERROR,
			&err, &optlen
		))
			printf("sock err %d, %s\n", err, ::strerror(err));

		printf("--1- error\n");
		sa.set_actor(t_act);
		return true;
	}

	virtual bool write(
		zi::scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
		printf("--1- write\n");
		sa.set_actor(act);
		return true;
	}

private:
	zi::actor &act;
	zi::terminating_actor t_act;
};

namespace ucpf { namespace zivug { namespace config {

template <typename Alloc>
struct io_client {
	template <typename Packager>
	void mina_pack(Packager &p)
	{
		p(MINA_NPP(epoll, protocol, address));
	}

	io_client(Alloc const &a = Alloc())
	: protocol(a), address(a)
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

	string<Alloc> protocol;
	string<Alloc> address;
};

}}}

int main(int argc, char **argv)
{
	typedef std::allocator<void> a_type;
	a_type a;

	ms::gdbm gs("hose.cfg");

	mi::np_packager<
		mi::text_store_adaptor<decltype(gs), a_type>
	> pack(gs, a);

	zc::io_client<a_type> c_cfg(a);
	pack.restore({"client"}, c_cfg);

	zi::rr_scheduler<a_type> sched(c_cfg, a);

	auto dp(zi::address_family::make_descriptor(
		std::begin(c_cfg.protocol).base(),
		std::end(c_cfg.protocol).base()
	));

	zi::splice_output_actor s_actor(zi::descriptor([]() -> int {
		return ::dup(STDIN_FILENO);
	}), 256);
	connector_actor c_actor(s_actor);

	printf("--0- %d - %p\n", dp.first.native(), &dp.second);

	dp.second->connect(
		dp.first, std::begin(c_cfg.address).base(),
		std::end(c_cfg.address).base()
	);

	printf("--8- %p\n", &c_actor);
	sched.imbue(std::move(dp.first), c_actor);
	printf("--9-\n");
	sched.wait();
	printf("-10-\n");
	return 0;
}

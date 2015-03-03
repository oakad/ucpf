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

struct connector_actor : zi::actor {
};

struct splice_writer_actor : zi::actor {
	splice_writer_actor(int src_fd_)
	: src_fd(src_fd_)
	{}

	virtual void init(zi::scheduler_action &&sa)
	{
		printf("xx init %p\n", this);
		sa.wait_write();
	}

	virtual void write(
		zi::scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
		printf("--1- write\n");

		auto rv(::splice(
			src_fd, nullptr, sa.get_descriptor().native(),
			nullptr, std::size_t(1) << 8,
			SPLICE_F_MOVE | SPLICE_F_NONBLOCK
		));

		printf("splice %zd, %d\n", rv, errno);

		if (rv > 0)
			sa.resume_write();
		else if (rv < 0) {
			if (errno == EAGAIN)
				sa.wait_write();
			else
				sa.release();
		} else
			sa.release();

		printf("--4- write out\n");
	}

	virtual void error(zi::scheduler_action &&sa, bool priority)
	{
		printf("--1- error\n");
	}

private:
	int src_fd;
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
	splice_writer_actor c_actor(STDIN_FILENO);

	auto dp(zi::address_family::make_descriptor(
		std::begin(c_cfg.protocol).base(),
		std::end(c_cfg.protocol).base()
	));

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

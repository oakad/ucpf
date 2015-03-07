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

#include <zivug/io/scheduler.hpp>

extern "C" {

#include <sys/socket.h>

}

namespace mi = ucpf::mina;
namespace ms = ucpf::mina::store;
namespace zc = ucpf::zivug::config;
namespace zi = ucpf::zivug::io;

struct splice_writer_actor : zi::actor {
	splice_writer_actor(int src_fd_)
	: reader(*this), src_fd(src_fd_), pipe_fd{-1, -1}, pipe_fill(0)
	{
	}

	virtual ~splice_writer_actor()
	{
		mark_close(pipe_fd[0]);
		mark_close(pipe_fd[1]);
	}

	virtual void init(zi::scheduler_action &&sa, bool new_desc)
	{
		zi::descriptor in_d([src_fd = src_fd]() -> int {
			auto rv(dup(src_fd));
			if (0 > rv)
				throw std::system_error(
					errno, std::system_category()
				);

			auto flags(::fcntl(rv, F_GETFL, 0));
			if (0 > ::fcntl(rv, F_SETFL, flags | O_NONBLOCK))
				throw std::system_error(
					errno, std::system_category()
				);

			return rv;
		});

		if (0 > ::pipe2(pipe_fd, O_NONBLOCK))
			throw std::system_error(
				errno, std::system_category()
			);
		printf("pipe %d -> %d\n", pipe_fd[1], pipe_fd[0]);
		sa.get_scheduler().imbue(std::move(in_d), reader);
	}

	virtual bool write(
		zi::scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
		int err(0);

		if (pipe_fill) {
			auto rv(::splice(
				pipe_fd[0], nullptr,
				sa.get_descriptor().native(),
				nullptr, std::size_t(1) << 8,
				SPLICE_F_MOVE | SPLICE_F_NONBLOCK
			));

			printf(
				"pipe read %zd, %d (%s)\n", rv, errno,
				::strerror(errno)
			);
			if (rv >= 0)
				pipe_fill -= rv;
			else
				err = errno;
		}

		if (err) {
			if (err == EAGAIN) {
				if (pipe_fill)
					sa.wait_write();
				else {
					writer_tk = sa.suspend();
					sa.resume_read(std::move(reader_tk));
				}
			} else {
				mark_close(pipe_fd[0]);
				sa.release(std::move(reader_tk));
				sa.release();
			}
		} else if (!pipe_fill) {
			writer_tk = sa.suspend();
			sa.resume_read(std::move(reader_tk));
		} else
			sa.wait_write();

		return true;
	}

	virtual bool error(zi::scheduler_action &&sa, bool priority)
	{
		printf("--2- error\n");
		mark_close(pipe_fd[0]);
		sa.release(std::move(reader_tk));
		sa.release();
		return true;
	}

	virtual bool hang_up(zi::scheduler_action &&sa, bool read_only)
	{
		printf("--2- hang_up\n");
		mark_close(pipe_fd[0]);
		sa.release(std::move(reader_tk));
		sa.release();
		return true;
	}

private:
	struct reader_actor : zi::actor {
		reader_actor(splice_writer_actor &parent_)
		: parent(parent_)
		{}

		virtual void init(zi::scheduler_action &&sa, bool new_desc)
		{
		}

		virtual bool read(
			zi::scheduler_action &&sa, bool out_of_band,
			bool priority
		)
		{
			if (parent.pipe_fd[0] < 0) {
				sa.release();
				mark_close(parent.pipe_fd[1]);
				return true;
			}

			auto rv(::splice(
				sa.get_descriptor().native(), nullptr,
				parent.pipe_fd[1], nullptr, std::size_t(1) << 8,
				SPLICE_F_MOVE | SPLICE_F_NONBLOCK
			));

			printf(
				"pipe write %zd, %d (%s)\n", rv, errno,
				::strerror(errno)
			);

			int err(rv >= 0 ? 0 : errno);
			parent.pipe_fill += (rv > 0) ? rv : 0;

			if (parent.pipe_fill)
				sa.resume_write(std::move(parent.writer_tk));

			if (err) {
				if (err == EAGAIN) {
					if (parent.pipe_fill)
						parent.reader_tk = sa.suspend();
					else
						sa.wait_read();
				} else {
					mark_close(parent.pipe_fd[1]);
					sa.resume_write(std::move(
						parent.writer_tk
					));
					sa.release();
				}
			}

			return true;
		}

		splice_writer_actor &parent;
	} reader;

	static void mark_close(int &fd)
	{
		if (fd >= 0) {
			::close(fd);
			fd = -1;
		}
	}

	int src_fd;
	int pipe_fd[2];
	int pipe_fill;
	zi::scheduler_token reader_tk;
	zi::scheduler_token writer_tk;
};

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
		return act.write(
			std::forward<zi::scheduler_action>(sa),
			out_of_band, priority
		);
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

	splice_writer_actor s_actor(STDIN_FILENO);
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

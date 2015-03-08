/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_2E6D5E1121195905417362EE3E001124)
#define HPP_2E6D5E1121195905417362EE3E001124

#include <zivug/io/scheduler.hpp>

extern "C" {

#include <fcntl.h>

}

namespace ucpf { namespace zivug { namespace io {

struct splice_channel_actor : actor {
	splice_channel_actor(descriptor &&src_d_)
	: reader(*this), src_d(std::move(src_d_)), pipe_fd{-1, -1},
	  pipe_fill(0)
	{}

	virtual ~splice_channel_actor()
	{
		mark_close(pipe_fd[0]);
		mark_close(pipe_fd[1]);
	}

	virtual void init(scheduler_action &&sa, bool new_desc)
	{
		auto flags(::fcntl(src_d.native(), F_GETFL, 0));
		if (0 > ::fcntl(src_d.native(), F_SETFL, flags | O_NONBLOCK))
			throw std::system_error(
				errno, std::system_category()
			);

		if (0 > ::pipe2(pipe_fd, O_NONBLOCK))
			throw std::system_error(
				errno, std::system_category()
			);
		printf("pipe %d -> %d\n", pipe_fd[1], pipe_fd[0]);
		sa.get_scheduler().imbue(std::move(src_d), reader);
		writer_tk = sa.suspend();
	}

	virtual bool write(
		scheduler_action &&sa, bool out_of_band, bool priority
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

	virtual bool error(scheduler_action &&sa, bool priority)
	{
		printf("--2- writer error\n");
		mark_close(pipe_fd[0]);
		sa.release(std::move(reader_tk));
		sa.release();
		return true;
	}

	virtual bool hang_up(scheduler_action &&sa, bool read_only)
	{
		printf("--2- writer hang_up\n");
		mark_close(pipe_fd[0]);
		sa.release(std::move(reader_tk));
		sa.release();
		return true;
	}

private:
	struct reader_actor : actor {
		reader_actor(splice_channel_actor &parent_)
		: parent(parent_)
		{}

		virtual void init(scheduler_action &&sa, bool new_desc)
		{
			sa.wait_read();
		}

		virtual bool read(
			scheduler_action &&sa, bool out_of_band,
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

		virtual bool error(scheduler_action &&sa, bool priority)
		{
			printf("--2- reader error\n");
			mark_close(parent.pipe_fd[1]);
			sa.resume_write(std::move(parent.writer_tk));
			sa.release();
			return true;
		}

		virtual bool hang_up(scheduler_action &&sa, bool read_only)
		{
			printf("--2- reader hang_up\n");
			mark_close(parent.pipe_fd[1]);
			sa.resume_write(std::move(parent.writer_tk));
			sa.release();
			return true;
		}

		splice_channel_actor &parent;
	} reader;

	static void mark_close(int &fd)
	{
		if (fd >= 0) {
			::close(fd);
			fd = -1;
		}
	}

	descriptor src_d;
	int pipe_fd[2];
	int pipe_fill;
	scheduler_token reader_tk;
	scheduler_token writer_tk;
};

}}}
#endif

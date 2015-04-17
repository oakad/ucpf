/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_E764CA8CAFAA830571F29FD51CE28A9E)
#define HPP_E764CA8CAFAA830571F29FD51CE28A9E

#include <zivug/io/scheduler.hpp>

extern "C" {

#include <fcntl.h>
#include <sys/stat.h>

}

namespace ucpf { namespace zivug { namespace io {

struct splice_input_actor : actor {
	splice_input_actor(descriptor &&dst_d_, std::size_t block_size_)
	: writer(*this), dst_d(std::move(dst_d_)), pipe_fd{-1, -1},
	  pipe_fill(0), block_size(block_size_), blocking_dst(false)
	{}

	virtual ~splice_input_actor()
	{
		mark_close(pipe_fd[0]);
		mark_close(pipe_fd[1]);
	}

	virtual void init(scheduler_action &&sa, bool new_desc)
	{
		struct ::stat fs{0};
		if (0 > ::fstat(dst_d.native(), &fs))
			throw std::system_error(
				errno, std::system_category()
			);

		auto flags(::fcntl(dst_d.native(), F_GETFL, 0));

		if (S_ISREG(fs.st_mode) || !(flags & O_NONBLOCK))
			blocking_dst = true;

		if (0 > ::pipe2(pipe_fd, O_NONBLOCK))
			throw std::system_error(
				errno, std::system_category()
			);
		printf("pipe %d -> %d\n", pipe_fd[1], pipe_fd[0]);
		sa.get_scheduler().imbue(std::move(dst_d), writer);
		sa.wait_read();
	}

	virtual bool read(
		scheduler_action &&sa, bool out_of_band,
		bool priority
	)
	{
		if (pipe_fd[0] < 0) {
			sa.release();
			mark_close(pipe_fd[1]);
			return true;
		}

		auto rv(::splice(
			sa.get_descriptor().native(), nullptr,
			parent.pipe_fd[1], nullptr, block_size,
			SPLICE_F_MOVE | SPLICE_F_NONBLOCK
		));

		printf(
			"pipe write %zd, %d (%s)\n", rv, errno,
			::strerror(errno)
		);

		int err(rv >= 0 ? 0 : errno);
		pipe_fill += (rv > 0) ? rv : 0;

		if (pipe_fill)
			sa.resume_write(std::move(parent.writer_tk));

		if (err) {
			if (err == EAGAIN) {
				if (pipe_fill)
					reader_tk = sa.suspend();
				else
					sa.wait_read();
			} else {
				mark_close(pipe_fd[1]);
				sa.resume_write(std::move(parent.writer_tk));
				sa.release();
			}
		}

		return true;
	}

	virtual bool error(scheduler_action &&sa, bool priority)
	{
		printf("--2- reader error\n");
		mark_close(pipe_fd[1]);
		sa.resume_write(std::move(parent.writer_tk));
		sa.release();
		return true;
	}

	virtual bool hang_up(scheduler_action &&sa, bool read_only)
	{
		printf("--2- reader hang_up\n");
		mark_close(pipe_fd[1]);
		sa.resume_write(std::move(parent.writer_tk));
		sa.release();
		return true;
	}

private:
	struct writer_actor : actor {
		writer_actor(splice_input_actor &parent_)
		: parent(parent_)
		{}

		virtual void init(scheduler_action &&sa, bool new_desc)
		{
			sa.wait_write();
		}


		virtual bool write(
			scheduler_action &&sa, bool out_of_band, bool priority
		)
		{
			int err(0);

			if (parent.pipe_fill) {
				auto rv(::splice(
					parent.pipe_fd[0], nullptr,
					sa.get_descriptor().native(),
					nullptr, parent.block_size,
					SPLICE_F_MOVE | SPLICE_F_NONBLOCK
				));

				printf(
					"pipe read %zd, %d (%s)\n", rv, errno,
					::strerror(errno)
				);
				if (rv >= 0)
					parent.pipe_fill -= rv;
				else
					err = errno;
			}

			if (err) {
				if (err == EAGAIN) {
					if (parent.pipe_fill)
						sa.wait_write();
					else {
						writer_tk = sa.suspend();
						sa.resume_read(std::move(
							parent.reader_tk
						));
					}
				} else {
					mark_close(parent.pipe_fd[0]);
					sa.release(std::move(reader_tk));
					sa.release();
				}
			} else if (!pipe_fill) {
				parent.writer_tk = sa.suspend();
				sa.resume_read(std::move(parent.reader_tk));
			} else
				sa.wait_write();

			return true;
		}

		virtual bool error(scheduler_action &&sa, bool priority)
		{
			printf("--2- writer error\n");
			mark_close(parent.pipe_fd[0]);
			sa.release(std::move(parent.reader_tk));
			sa.release();
			return true;
		}

		virtual bool hang_up(scheduler_action &&sa, bool read_only)
		{
			printf("--2- writer hang_up\n");
			mark_close(parent.pipe_fd[0]);
			sa.release(std::move(parent.reader_tk));
			sa.release();
			return true;
		}

		splice_input_actor &parent;
	} writer;

	static void mark_close(int &fd)
	{
		if (fd >= 0) {
			::close(fd);
			fd = -1;
		}
	}

	descriptor dst_d;
	int pipe_fd[2];
	int pipe_fill;
	std::size_t block_size;
	bool blocking_dst;
	scheduler_token reader_tk;
	scheduler_token writer_tk;
};

}}}
#endif

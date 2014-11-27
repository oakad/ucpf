/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_ZIVUG_SOCKET_20141126T1500)
#define UCPF_ZIVUG_SOCKET_20141126T1500

extern "C" {

#include <unistd.h>
#include <sys/epoll.h>

}

#include <system_error>

namespace ucpf { namespace zivug { namespace socket {

struct descriptor {
	descriptor()
	: fd(-1)
	{}

	
	int fd;
};

struct event {
	enum {
		READ = 0,
		WRITE = 1,
		PRIORITY_READ = 2,
		HANG_UP = 3,
		READ_HANG_UP = 4,
		ERROR = 5
	};
};

struct notification {
	virtual ~notification()
	{}

	virtual void read_ready(bool out_of_band, bool priority)
	{}

	virtual void write_ready(bool out_of_band, bool priority)
	{}

	virtual void error(bool priority)
	{}

	virtual void hang_up(bool read_only)
	{}
};

template <std::size_t EventCount>
struct event_dispatcher {
	event_dispatcher()
	: fd(::epoll_create1(0)), timeout(-1)
	{
		if (fd < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	~event_dispatcher()
	{
		if (fd >= 0)
			::close(fd);
	}

	void reset(descriptor d, notification &n)
	{
		::epoll_event ev{
			.events = all_events_mask | one_shot_event_mask,
			.data = { .ptr = &n }
		};

		auto rv(epoll_ctl(fd, EPOLL_CTL_MOD, d.fd, &ev));

		if ((rv < 0) && (errno == ENOENT))
			rv = epoll_ctl(fd, EPOLL_CTL_ADD, d.fd, &ev);

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	void remove(descriptor d)
	{
		epoll_ctl(fd, EPOLL_CTL_DEL, d.fd, nullptr);
	}

	bool process_next()
	{
		auto rv(::epoll_wait(fd, ev_buf, EventCount, timeout));

		for (auto c(0); c < rv; ++c) {
			auto n_ptr(reinterpret_cast<notification *>(
				ev_buf[c].data.ptr
			));

			if (ev_buf[c].events & read_event_mask)
				n_ptr->read_ready(
					ev_buf[c].events
					& out_of_band_event_mask,
					ev_buf[c].events
					& priority_event_mask
				);
			else if (ev_buf[c].events & write_event_mask)
				n_ptr->write_ready(
					ev_buf[c].events
					& out_of_band_event_mask,
					ev_buf[c].events
					& priority_event_mask
				);
			else if (ev_buf[c].events & hang_up_event_mask)
				n_ptr->hang_up(
					ev_buf[c].events
					& read_hang_up_event_mask
				);
			else if (ev_buf[c].events & error_event_mask)
				n_ptr->error(
					ev_buf[c].events
					& priority_event_mask
				);
		}

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);

		return rv > 0;
	}

private:
	constexpr static uint32_t read_event_mask
	= EPOLLIN | EPOLLRDNORM | EPOLLRDBAND;
	constexpr static uint32_t write_event_mask
	= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
	constexpr static uint32_t hang_up_event_mask
	= EPOLLHUP | EPOLLRDHUP;
	constexpr static uint32_t error_event_mask
	= EPOLLERR;
	constexpr static uint32_t all_events_mask
	= read_event_mask | write_event_mask | hang_up_event_mask
	  | error_event_mask;

	constexpr static uint32_t out_of_band_event_mask
	= EPOLLRDBAND | EPOLLWRBAND;
	constexpr static uint32_t priority_event_mask = EPOLLPRI;
	constexpr static uint32_t read_hang_up_event_mask = EPOLLRDHUP;
	constexpr static uint32_t one_shot_event_mask
	= EPOLLONESHOT | EPOLLET;

	int fd;
	int timeout;
	::epoll_event ev_buf[EventCount];
};

}}}

#endif

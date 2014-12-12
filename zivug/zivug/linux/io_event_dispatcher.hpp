/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_E9C0D279EF6DE6EE0217C75F7640D97C)
#define HPP_E9C0D279EF6DE6EE0217C75F7640D97C

extern "C" {

#include <unistd.h>
#include <sys/epoll.h>

}

#include <system_error>

namespace ucpf { namespace zivug { namespace io {

struct descriptor {
	descriptor()
	: fd(-1)
	{}

	template <typename OpenFunc>
	descriptor(OpenFunc &&func)
	: fd(func())
	{}

	descriptor(descriptor &&other)
	: fd(-1)
	{
		std::swap(fd, other.fd);
	}

	descriptor(descriptor const &other) = delete;
	descriptor &operator=(descriptor const &other) = delete;

	~descriptor()
	{
		if (fd >= 0)
			close(fd);
	}

	int native() const
	{
		return fd;
	}

private:
	int fd;
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

struct event_dispatcher {
	template <typename ConfigType>
	event_dispatcher(ConfigType const &config)
	: fd(::epoll_create1(0)), timeout(config.epoll.timeout_ms),
	  event_count(config.epoll.event_count)
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

	void set(descriptor const &d, notification &n)
	{
		::epoll_event ev{
			.events = all_events_mask | edge_triggered_event_mask,
			.data = { .ptr = &n }
		};

		auto rv(epoll_ctl(fd, EPOLL_CTL_ADD, d.native(), &ev));

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	void reset(descriptor const &d, notification &n)
	{
		::epoll_event ev{
			.events = all_events_mask | one_shot_event_mask,
			.data = { .ptr = &n }
		};

		auto rv(epoll_ctl(fd, EPOLL_CTL_MOD, d.native(), &ev));

		if ((rv < 0) && (errno == ENOENT))
			rv = epoll_ctl(fd, EPOLL_CTL_ADD, d.native(), &ev);

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	void reset_read(descriptor const &d, notification &n)
	{
		::epoll_event ev{
			.events = read_events_mask | one_shot_event_mask,
			.data = { .ptr = &n }
		};

		auto rv(epoll_ctl(fd, EPOLL_CTL_MOD, d.native(), &ev));

		if ((rv < 0) && (errno == ENOENT))
			rv = epoll_ctl(fd, EPOLL_CTL_ADD, d.native(), &ev);

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	void reset_write(descriptor const &d, notification &n)
	{
		::epoll_event ev{
			.events = write_events_mask | one_shot_event_mask,
			.data = { .ptr = &n }
		};

		auto rv(epoll_ctl(fd, EPOLL_CTL_MOD, d.native(), &ev));

		if ((rv < 0) && (errno == ENOENT))
			rv = epoll_ctl(fd, EPOLL_CTL_ADD, d.native(), &ev);

		if (rv < 0)
			throw std::system_error(
				errno, std::system_category()
			);

	}

	void remove(descriptor const &d)
	{
		epoll_ctl(fd, EPOLL_CTL_DEL, d.native(), nullptr);
	}

	bool process_next()
	{
		::epoll_event ev_buf[event_count];
		auto rv(::epoll_wait(fd, ev_buf, event_count, timeout));

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

			if (ev_buf[c].events & write_event_mask)
				n_ptr->write_ready(
					ev_buf[c].events
					& out_of_band_event_mask,
					ev_buf[c].events
					& priority_event_mask
				);

			if (ev_buf[c].events & hang_up_event_mask)
				n_ptr->hang_up(
					ev_buf[c].events
					& read_hang_up_event_mask
				);

			if (ev_buf[c].events & error_event_mask)
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

	constexpr static uint32_t read_events_mask
	= read_event_mask | hang_up_event_mask
	  | error_event_mask;
	constexpr static uint32_t write_events_mask
	= write_event_mask | hang_up_event_mask
	  | error_event_mask;
	constexpr static uint32_t all_events_mask
	= read_event_mask | write_event_mask | hang_up_event_mask
	  | error_event_mask;

	constexpr static uint32_t out_of_band_event_mask
	= EPOLLRDBAND | EPOLLWRBAND;
	constexpr static uint32_t priority_event_mask = EPOLLPRI;
	constexpr static uint32_t read_hang_up_event_mask = EPOLLRDHUP;
	constexpr static uint32_t edge_triggered_event_mask
	= EPOLLET;
	constexpr static uint32_t one_shot_event_mask
	= EPOLLONESHOT | EPOLLET;

	int fd;
	int timeout;
	int event_count;
};

}}}

#endif

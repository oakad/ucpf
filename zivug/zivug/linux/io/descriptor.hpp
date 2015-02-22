/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_E370D7302C2FA10CE27C25A0FC60795E)
#define HPP_E370D7302C2FA10CE27C25A0FC60795E

extern "C" {

#include <unistd.h>

}

#include <utility>

namespace ucpf { namespace zivug { namespace io {

struct descriptor {
	descriptor()
	: fd(-1), ctx(nullptr)
	{}

	template <typename OpenFunc>
	descriptor(OpenFunc &&func)
	: fd(func()), ctx(nullptr)
	{}

	template <typename OpenFunc, typename ContextType>
	descriptor(OpenFunc &&func, ContextType const *ctx_)
	: fd(func()), ctx(ctx_)
	{}

	descriptor(descriptor &&other)
	: fd(-1), ctx(nullptr)
	{
		std::swap(fd, other.fd);
		std::swap(ctx, other.ctx);
	}

	descriptor(descriptor const &other) = delete;
	descriptor &operator=(descriptor const &other) = delete;

	~descriptor()
	{
		if (fd >= 0)
			::close(fd);
	}

	int native() const
	{
		return fd;
	}

	template <typename ContextType>
	ContextType const *context() const
	{
		return reinterpret_cast<ContextType const *>(ctx);
	}

	explicit operator bool() const
	{
		return fd >= 0;
	}

private:
	int fd;
	void const *ctx;
};

}}}
#endif

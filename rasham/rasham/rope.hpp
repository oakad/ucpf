/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_ROPE_HPP)
#define _RASHAM_ROPE_HPP

#include <rasham/internal/rope_base.hpp>
#include <rasham/internal/rope_ops.hpp>

#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>

namespace rasham
{

typedef rope<char> crope;
typedef rope<wchar_t> wrope;

template <
	typename char_type,
	typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>,
	typename param_type = rope_default_params
> struct rope_file_reader {
	typedef rope<char_type, traits_type, alloc_type, param_type> rope_type;

	rope_file_reader(char const *name, alloc_type a = alloc_type())
	: c(allocate_counted<context>(a, name))
	{}

	rope_file_reader(int fd, bool owned, alloc_type a = alloc_type())
	: c(allocate_counted<context>(a, fd, owned))
	{}

	~rope_file_reader()
	{}

	void operator()(char_type *buf, size_t count, size_t offset)
	{
		c->read(buf, count, offset);
	}

	operator rope_type ()
	{
		return rope_type(
			*this, c->size(),
			*c.template get_allocator<alloc_type>()
		);
	}

private:
	struct context {
		context(char const *name)
		: fd(open(name, O_RDONLY)), owned(true)
		{}

		context(int fd_, bool owned_)
		: fd(fd_), owned(owned_)
		{}

		context(context &&other)
		: fd(other.fd), owned(other.owned)
		{
			other.owned = false;
		}

		context(context const &) = delete;
		context &operator=(context const &) = delete;

		~context()
		{
			if (owned)
				close(fd);
		}

		void read(char_type *buf, size_t count, size_t offset)
		{
			auto b_count(count * sizeof(char_type));

			auto rv(pread(
				fd, buf, b_count, offset * sizeof(char_type)
			));

			if (rv < ssize_t(b_count)) {
				if (rv < 0)
					rv = 0;
				memset(buf + rv, 0, b_count - size_t(rv));
			}
		}

		size_t size() const
		{
			struct stat s;
			auto rv(fstat(fd, &s));

			if (rv < 0)
				return 0;

			return s.st_size / sizeof(char_type);
		}

	private:
		int fd;
		bool owned;
	};

	counted_ptr<context> c;
};

typedef rope_file_reader<char> crope_file_reader;
typedef rope_file_reader<wchar_t> wrope_file_reader;

}

namespace std
{

template<>
struct hash<rasham::crope>
{
	size_t operator()(rasham::crope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

template<>
struct hash<rasham::wrope>
{
	size_t operator()(rasham::wrope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

}

#endif

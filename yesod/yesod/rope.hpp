/*
 * Copyright (c) 2012 - 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ROPE_OCT_31_2013_1840)
#define UCPF_YESOD_ROPE_OCT_31_2013_1840

#include <yesod/detail/rope_base.hpp>
#include <yesod/detail/rope_ops.hpp>

#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>

namespace ucpf { namespace yesod {

struct default_rope_policy {
	enum {
		/* We allocate leaf data in increments of
		 * 1 << alloc_granularity_shift.
		 */
		alloc_granularity_shift = 3,

		/* Iterators will store path_cache_len rope node
		 * references.
		 */
		path_cache_len = 5,

		/* Iterators will cache iterator_buf_len characters
		 * from non-leaf rope nodes.
		 */
		iterator_buf_len = 16,

		/* For strings shorter than max_copy, we copy to
		 * concatenate.
		 */
		max_copy = 23,

		/* When dumping internal structure, string printouts
		 * are cut to max_printout_len characters.
		 */
		max_printout_len = 40,

		/* Maximal rope tree depth. */
		max_rope_depth = 45,

		/* For substrings longer than lazy_threshold, we create
		 * substring nodes.
		 */
		lazy_threshold = 128
	};
};

typedef rope<
	char, std::char_traits<char>, std::allocator<char>, default_rope_policy
> crope;
typedef rope<
	wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>,
	default_rope_policy
> wrope;

template <
	typename CharType,
	typename TraitsType = std::char_traits<CharType>,
	typename AllocType = std::allocator<CharType>,
	typename Policy = default_rope_policy
> struct rope_file_reader {
	typedef rope<CharType, TraitsType, AllocType, Policy> rope_type;

	rope_file_reader(char const *name, AllocType a = AllocType())
	: c(allocate_counted<context>(a, name))
	{}

	rope_file_reader(int fd, bool owned, AllocType a = AllocType())
	: c(allocate_counted<context>(a, fd, owned))
	{}

	~rope_file_reader()
	{}

	void operator()(CharType *buf, size_t count, size_t offset)
	{
		c->read(buf, count, offset);
	}

	operator rope_type ()
	{
		return rope_type(
			*this, c->size(),
			*c.template get_allocator<AllocType>()
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

		void read(CharType *buf, size_t count, size_t offset)
		{
			auto b_count(count * sizeof(CharType));

			auto rv(pread(
				fd, buf, b_count, offset * sizeof(CharType)
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

			return s.st_size / sizeof(CharType);
		}

	private:
		int fd;
		bool owned;
	};

	counted_ptr<context> c;
};

typedef rope_file_reader<char> crope_file_reader;
typedef rope_file_reader<wchar_t> wrope_file_reader;

}}

namespace std
{

template<>
struct hash<ucpf::yesod::crope>
{
	size_t operator()(ucpf::yesod::crope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

template<>
struct hash<ucpf::yesod::wrope>
{
	size_t operator()(ucpf::yesod::wrope const &str) const
	{
		size_t size(str.size());
		if (!size)
			return 0;
		return 13 * str[0] + 5 * str[size - 1] + size;
	}
};

}
#endif

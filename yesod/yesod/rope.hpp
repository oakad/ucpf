/*
 * Copyright (c) 2012-2014 Alex Dubov <oakad@yahoo.com>
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
#include <cstdio>

namespace ucpf { namespace yesod {

struct rope_default_policy {
	/* Maximal rope tree depth. */
	static constexpr int max_rope_depth = 45;

	/* We allocate leaf data in increments of
	 * 1 << alloc_granularity_shift.
	 */
	static constexpr int alloc_granularity_shift = 3;

	/* Iterators will store path_cache_len rope node
	 * references.
	 */
	static constexpr int path_cache_len = 5;

	/* Iterators will cache iterator_buf_len characters
	 * from non-leaf rope nodes.
	 */
	static constexpr int iterator_buf_len = 16;

	/* For strings shorter than max_copy, we copy to
	 * concatenate.
	 */
	static constexpr size_t max_copy = 23;

	/* When dumping internal structure, string printouts
	 * are cut to max_printout_len characters.
	 */
	static constexpr int max_printout_len = 40;

	/* For substrings longer than lazy_threshold, we create
	 * substring nodes.
	 */
	static constexpr int lazy_threshold = 128;
};

typedef rope<char, rope_default_policy> crope;
typedef rope<wchar_t, rope_default_policy> wrope;

template <
	typename CharType, typename Policy = rope_default_policy
> struct rope_file_reader {
	typedef rope<CharType, Policy>          rope_type;

	rope_file_reader(char const *name)
	: c(make_counted<context>(name))
	{}

	template <typename Alloc>
	rope_file_reader(char const *name, Alloc const &a)
	: c(allocate_counted<context>(a, name))
	{}

	rope_file_reader(int fd, bool owned)
	: c(make_counted<context>(fd, owned))
	{}

	template <typename Alloc>
	rope_file_reader(int fd, bool owned, Alloc const &a)
	: c(allocate_counted<context>(a, fd, owned))
	{}

	~rope_file_reader()
	{}

	void operator()(CharType *buf, size_t count, size_t offset)
	{
		c->read(buf, count, offset);
	}

	operator rope_type()
	{
		wrapper w = {
			.reader = this
		};
		c.access_allocator(&w);
		return w.rv;
	}

private:
	struct wrapper {
		rope_file_reader *reader;
		rope_type r;
	};

	struct context {
		context(char const *name)
		: fd(fopen(name, "rb")), owned(true)
		{}

		context(FILE *fd_, bool owned_)
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
				fclose(fd);
		}

		void read(CharType *buf, size_t count, size_t offset)
		{
			fseek(fd, count * sizeof(CharType), SEEK_SET);

			auto rv(fread(
				buf, sizeof(CharType), count, fd
			));

			if (rv < ssize_t(count)) {
				if (rv < 0)
					rv = 0;
				std::fill_n(buf + rv, count - rv, 0);
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

		template <typename Alloc>
		static void access_allocator(
			Alloc &a, context const *p, void *data
		)
		{
			auto w(reinterpret_cast<wrapper *>(data));
			w->rv = rope_type(
				*(w->reader), w->reader.c->size(), a
			);
		}

	private:
		FILE *fd;
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

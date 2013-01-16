/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_VALIDATING_TEST_STREAM_HPP)
#define _RASHAM_VALIDATING_TEST_STREAM_HPP

#include <rasham/rope_stream.hpp>
#include <iostream>

namespace rasham
{

template <
	typename char_type, typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>,
	typename param_type = rope_default_params
> struct validating_test_stream;

typedef validating_test_stream<char> validating_test_cstream;
typedef validating_test_stream<wchar_t> validating_test_wstream;

template <
	typename char_type_, typename traits_type_, typename alloc_type,
	typename param_type
> struct validating_test_stream
: public std::basic_ostream<char_type_, traits_type_> {
	typedef char_type_                                 char_type;
	typedef traits_type_                               traits_type;
	typedef alloc_type                                 allocator_type;
	typedef typename traits_type::int_type             int_type;
	typedef typename traits_type::pos_type             pos_type;
	typedef typename traits_type::off_type             off_type;

	typedef rope<
		char_type, traits_type, allocator_type, param_type
	> rope_type;
	typedef rope_streambuf<
		char_type, traits_type, allocator_type, param_type
	> streambuf_type;
	typedef rope_file_reader<
		char_type, traits_type, allocator_type, param_type
	> reader_type;
	typedef std::basic_ostream<char_type, traits_type> ostream_type;

	explicit validating_test_stream(
		std::string const &pattern_file,
		std::string const &log_file = std::string(),
		std::ios_base::openmode m = std::ios_base::out
	) : ostream_type(), streambuf(m | std::ios_base::out), flush_pos(0),
	    pattern(std::move(reader_type(pattern_file.c_str()))),
	    log_fd(!log_file.empty() ? creat(log_file.c_str(), 0644) : -1)
	{
		this->init(&streambuf);
	}

	~validating_test_stream()
	{
		flush_log();
		if (log_fd > -1)
			close(log_fd);
	}

	streambuf_type *rdbuf() const
	{
		return const_cast<streambuf_type *>(&streambuf);
	}

	bool match_pattern()
	{
		rope_type const &s_rope(streambuf.get_rope());
		printf("xxx\n");
		rope_type t_range(
			s_rope.cbegin() + flush_pos, s_rope.cend(),
			s_rope.get_allocator()
		);
		printf("yyy\n");
		rope_type p_range(
			pattern.cbegin() + flush_pos,
			pattern.cbegin() + s_rope.size(),
			pattern.get_allocator()
		);

		auto t_pos(t_range.cbegin()), p_pos(p_range.cbegin());
		auto t_end(t_range.cend()), p_end(p_range.cend());
		typename rope_type::size_type m_count(0);

		for (; (t_pos != t_end) && (p_pos != p_end); ++t_pos, ++p_pos)
			if (*t_pos == *p_pos)
				++m_count;
			else
				break;

		bool rv(m_count > 0);
		if (rv) {
			if ((t_pos != t_end) || (p_pos != p_end))
				rv = false;
		}

		flush_log();
		return rv;
	}

private:
	void flush_log()
	{
		rope_type const &s_rope(streambuf.get_rope());

		typename rope_type::size_type sz(s_rope.size() - flush_pos);

		if (log_fd > -1 && sz) {
			std::vector<char_type, allocator_type> buf(
				sz, char_type(0), s_rope.get_allocator()
			);
			s_rope.copy(&buf.front(), sz, flush_pos);
			write(log_fd, &buf.front(), sz);
		}

		flush_pos += sz;
	}

	streambuf_type streambuf;
	typename rope_type::size_type flush_pos;
	rope_type pattern;
	int log_fd;
};


}
#endif

/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_ROPE_STREAM_HPP)
#define _RASHAM_ROPE_STREAM_HPP

#include <rasham/internal/rope_base.hpp>
#include <rasham/internal/rope_ops.hpp>

namespace rasham
{

template <
	typename char_type, typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>
> struct rope_streambuf;

template <
	typename char_type, typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>
> struct rope_istream;

template <
	typename char_type, typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>
> struct rope_ostream;

template <
	typename char_type, typename traits_type = std::char_traits<char_type>,
	typename alloc_type = std::allocator<char_type>
> struct rope_stream;

typedef rope_istream<char> crope_istream;
typedef rope_ostream<char> crope_ostream;
typedef rope_stream<char>  crope_stream;

typedef rope_istream<wchar_t> wrope_istream;
typedef rope_ostream<wchar_t> wrope_ostream;
typedef rope_stream<wchar_t>  wrope_stream;

template <typename char_type_, typename traits_type_, typename alloc_type>
struct rope_streambuf : public std::basic_streambuf<char_type_, traits_type_> {
	typedef char_type_                                 char_type;
	typedef traits_type_                               traits_type;
	typedef alloc_type                                 allocator_type;
	typedef typename traits_type::int_type             int_type;
	typedef typename traits_type::pos_type             pos_type;
	typedef typename traits_type::off_type             off_type;

	typedef std::basic_streambuf<char_type_, traits_type_> streambuf_type;
	typedef rope<char_type_, traits_type_, alloc_type>     rope_type;
	typedef typename rope_type::size_type                  size_type;

	explicit rope_streambuf(
		std::ios_base::openmode m = std::ios_base::in
					    | std::ios_base::out
	) : streambuf_type(), mode(m), b_rope(), get_pos(0)
	{}

	explicit rope_streambuf(
		rope_type const &r_,
		std::ios_base::openmode m = std::ios_base::in
					    | std::ios_base::out
	) : streambuf_type(), mode(m), b_rope(r_), get_pos(0)
	{}

	rope_type get_rope() const
	{
		return b_rope;
	}

	void set_rope(rope_type const &r)
	{
		b_rope = r;
	}

protected:
	virtual std::streamsize showmanyc()
	{
		return std::streamsize(b_rope.size()) - get_pos;
	}

	virtual std::streamsize xsgetn(char_type *s, std::streamsize count)
	{
		auto rv(b_rope.copy(s, count, get_pos));
		get_pos += rv;
		return rv;
	}

	virtual std::streamsize xsputn(char_type const *s,
				       std::streamsize count)
	{
		b_rope.append(s, count);
		return count;
	}

	std::ios_base::openmode mode;
	rope_type b_rope;
	size_type get_pos;
};

template <typename char_type_, typename traits_type_, typename alloc_type>
struct rope_istream : public std::basic_istream<char_type_, traits_type_> {
	typedef char_type_                                 char_type;
	typedef traits_type_                               traits_type;
	typedef alloc_type                                 allocator_type;
	typedef typename traits_type::int_type             int_type;
	typedef typename traits_type::pos_type             pos_type;
	typedef typename traits_type::off_type             off_type;

	typedef rope<char_type, traits_type, alloc_type>   rope_type;
	typedef rope_streambuf<
		char_type, traits_type, alloc_type
	> streambuf_type;
	typedef std::basic_istream<char_type, traits_type> istream_type;

	explicit rope_istream(std::ios_base::openmode m = std::ios_base::in)
	: istream_type(), streambuf(m | std::ios_base::in)
	{
		this->init(&streambuf);
	}

	explicit rope_istream(
		rope_type const &r,
		std::ios_base::openmode m = std::ios_base::in
	) : istream_type(), streambuf(r, m | std::ios_base::in)
	{
		this->init(&streambuf);
	}

	~rope_istream()
	{}

	streambuf_type *rdbuf() const
	{
		return const_cast<streambuf_type *>(&streambuf);
	}

	rope_type get_rope() const
	{
		return streambuf.get_rope();
	}

	void set_rope(rope_type const &r)
	{
		streambuf.set_rope(r);
	}

private:
	streambuf_type streambuf;
};

template <typename char_type_, typename traits_type_, typename alloc_type>
struct rope_ostream : public std::basic_ostream<char_type_, traits_type_> {
	typedef char_type_                                 char_type;
	typedef traits_type_                               traits_type;
	typedef alloc_type                                 allocator_type;
	typedef typename traits_type::int_type             int_type;
	typedef typename traits_type::pos_type             pos_type;
	typedef typename traits_type::off_type             off_type;

	typedef rope<char_type, traits_type, alloc_type>   rope_type;
	typedef rope_streambuf<
		char_type, traits_type, alloc_type
	> streambuf_type;
	typedef std::basic_ostream<char_type, traits_type> ostream_type;

	explicit rope_ostream(std::ios_base::openmode m = std::ios_base::out)
	: ostream_type(), streambuf(m | std::ios_base::out)
	{
		this->init(&streambuf);
	}

	explicit rope_ostream(
		rope_type const &r,
		std::ios_base::openmode m = std::ios_base::out
	) : ostream_type(), streambuf(r, m | std::ios_base::out)
	{
		this->init(&streambuf);
	}

	~rope_ostream()
	{}

	streambuf_type *rdbuf() const
	{
		return const_cast<streambuf_type *>(&streambuf);
	}

	rope_type get_rope() const
	{
		return streambuf.get_rope();
	}

	void set_rope(rope_type const &r)
	{
		streambuf.set_rope(r);
	}

private:
	streambuf_type streambuf;
};

template <typename char_type_, typename traits_type_, typename alloc_type>
struct rope_stream : public std::basic_iostream<char_type_, traits_type_> {
	typedef char_type_                                  char_type;
	typedef traits_type_                                traits_type;
	typedef alloc_type                                  allocator_type;
	typedef typename traits_type::int_type              int_type;
	typedef typename traits_type::pos_type              pos_type;
	typedef typename traits_type::off_type              off_type;

	typedef rope<char_type, traits_type, alloc_type>    rope_type;
	typedef rope_streambuf<
		char_type, traits_type, alloc_type
	> streambuf_type;
	typedef std::basic_iostream<char_type, traits_type> iostream_type;

	explicit rope_stream(
		std::ios_base::openmode m = std::ios_base::out
					    | std::ios_base::in
	) : iostream_type(), streambuf(m)
	{
		this->init(&streambuf);
	}

	explicit rope_stream(
		rope_type const &r,
		std::ios_base::openmode m = std::ios_base::out
					    | std::ios_base::in
	) : iostream_type(), streambuf(r, m)
	{
		this->init(&streambuf);
	}

	~rope_stream()
	{}

	streambuf_type *rdbuf() const
	{
		return const_cast<streambuf_type *>(&streambuf);
	}

	rope_type get_rope() const
	{
		return streambuf.get_rope();
	}

	void set_rope(rope_type const &r)
	{
		streambuf.set_rope(r);
	}

private:
	streambuf_type streambuf;
};

}
#endif

/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_VALIDATING_TEST_STREAM_HPP)
#define _RASHAM_VALIDATING_TEST_STREAM_HPP

#include <rasham/internal/log_item.hpp>
#include <rasham/rope_stream.hpp>

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

	typedef typename rope_type::size_type size_type;

	template <typename input_iter_t>
	struct pattern_set {
		~pattern_set()
		{
			stmt_names.for_each(
				[](
					std::basic_string<char_type> s,
					log_item::value_set_base<
						input_iter_t, char_type
					> *v
				) -> void {
					delete v;
				}
			);
		}

		template <typename container_type>
		void add(
			std::basic_string<char_type> name,
			typename log_item::value_set<
				container_type, input_iter_t, char_type
			>::parser_type const &parser
		)
		{
			auto v_base(stmt_names.find(name));
			if (!v_base) {
				auto v(
					new log_item::value_set<
						container_type, input_iter_t,
						char_type
					>(parser));
				stmt_names.add(name, v);
			}
		}

		bool consume(
			log_item::stmt<char_type> const &s, input_iter_t &begin,
			input_iter_t const &end
		)
		{
			auto v_base(stmt_names.find(s.name));
			if (v_base)
				return (*v_base)->consume(s, begin, end);
			else
				return true;
		}

		template <typename container_type>
		container_type *get(
			std::basic_string<char_type> stmt_name,
			std::basic_string<char_type> var_name
   		)
		{
			auto v_base(stmt_names.find(stmt_name));
			if (v_base) {
				auto v(
					dynamic_cast<
						log_item::value_set<
							container_type,
							input_iter_t,
							char_type
						> *
					>(*v_base)
				);
				if (v)
					return v->get(var_name);
			}
			return nullptr;
		}

	private:
		log_item::qi::symbols<
			char_type,
			log_item::value_set_base<input_iter_t, char_type> *
		> stmt_names;
	};

	explicit validating_test_stream(
		std::string const &pattern_file,
		std::string const &log_file = std::string(),
		char_type escape_char = '%',
		std::ios_base::openmode m = std::ios_base::out
	) : ostream_type(), streambuf(m | std::ios_base::out),
	    s_flush_pos(0), p_flush_pos(0), s_failed_pos(0),
	    pattern(std::move(reader_type(pattern_file.c_str()))),
	    p_scanner(escape_char),
	    log_fd(!log_file.empty() ? creat(log_file.c_str(), 0644) : -1),
	    failed(false), e_scan(true)
	{
		this->init(&streambuf);
	}

	~validating_test_stream()
	{
		failed = true;
		rope_type const &s_rope(streambuf.get_rope());
		flush_log(s_rope.size() - s_flush_pos);

		if (log_fd > -1)
			close(log_fd);
	}

	streambuf_type *rdbuf() const
	{
		return const_cast<streambuf_type *>(&streambuf);
	}

	pattern_set<typename rope_type::const_iterator> &capture()
	{
		return p_set;
	}

	bool match_pattern()
	{
		rope_type const &s_rope(streambuf.get_rope());
		auto s_left(s_rope.size() - s_flush_pos);

		if (failed) {
			flush_log(s_left);
			return false;
		}

		for (; s_left; s_left = s_rope.size() - s_flush_pos)
			if (!match_extended())
				return false;

		return true;
	}

private:
	bool match_stmt(
		log_item::range<
			typename rope_type::const_iterator
		> const &p_range
	)
	{
		rope_type const &s_rope(streambuf.get_rope());
		auto s_pos(s_rope.cbegin() + s_flush_pos);

		switch (p_range.type) {
		case log_item::type::INVALID:
			return false;
		case log_item::type::ESCAPE:
			failed = *(p_range.item_match.begin() + 1) != *s_pos;
			flush_log(1, p_range.item_match.size());
			return !failed;
		case log_item::type::CAPTURE: {
			auto s_orig(s_pos);
			failed = !p_set.consume(
				p_range.stmt, s_pos, s_rope.cend()
			);
			flush_log(s_pos - s_orig, p_range.item_match.size());
			return !failed;
		}};
	}

	bool match_extended()
	{
		using boost::spirit::qi::parse;

		rope_type const &s_rope(streambuf.get_rope());
		log_item::range<typename rope_type::const_iterator> p_range;

		if (e_scan)
			e_scan = parse(
				pattern.cbegin() + p_flush_pos, pattern.cend(),
				p_scanner, p_range
			);

		auto p_prefix(
			e_scan ? p_range.item_match.begin()
				 - p_range.extent.begin()
			       : pattern.size() - p_flush_pos
		);

		auto s_left(s_rope.size() - s_flush_pos);

		if (!match_simple(std::min(s_left, p_prefix)))
			return false;

		if (e_scan)
			return match_stmt(p_range);
		else {
			if (!p_prefix && s_left) {
				failed = true;
				flush_log(s_left);
				return false;
			}
		}
		return true;
	}

	bool match_simple(size_type count)
	{
		rope_type const &s_rope(streambuf.get_rope());

		auto s_pos(s_rope.cbegin() + s_flush_pos);
		auto p_pos(pattern.cbegin() + p_flush_pos);
		size_type c(0);

		for (; c < count; ++c)
			if (*s_pos == *p_pos) {
				++s_pos;
				++p_pos;
			} else
				break;

		s_failed_pos = s_flush_pos + c;

		flush_log(c);

		if (c == count)
			return true;
		else {
			failed = true;
			flush_log(count - c);
			return false;
		}
	}

	void flush_log(size_type count)
	{
		flush_log(count, count);
	}

	void flush_log(size_type s_count, size_type p_count)
	{
		rope_type const &s_rope(streambuf.get_rope());

		if (log_fd > -1 && s_count) {
			write_buf.resize(s_count, char_type(0));
			s_rope.copy(&write_buf.front(), s_count, s_flush_pos);
			write(log_fd, &write_buf.front(), s_count);
		}

		s_flush_pos += s_count;
		if (!failed)
			p_flush_pos += std::min(
				pattern.size() - p_flush_pos, p_count
			);
	}

	streambuf_type streambuf;
	size_type s_flush_pos, p_flush_pos, s_failed_pos;
	rope_type pattern;
	log_item::scanner<typename rope_type::const_iterator> p_scanner;
	int log_fd;
	bool failed, e_scan;
	std::vector<char_type, allocator_type> write_buf;
	pattern_set<typename rope_type::const_iterator> p_set;
};

}
#endif

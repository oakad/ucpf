/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F5B061E5D859AF9ED545F3614F19D749)
#define HPP_F5B061E5D859AF9ED545F3614F19D749

#include <yesod/string_utils/charseq_adaptor.hpp>
#include <yesod/string_utils/ascii_hex_formatter.hpp>

#include <iostream>
#include <experimental/string_view>

namespace ucpf::yesod::test {

typedef std::basic_string<uint8_t> ustring;
typedef std::experimental::basic_string_view<uint8_t> ustring_view;

constexpr uint8_t const *operator ""_us(char const *s, size_t len)
{
	return reinterpret_cast<uint8_t const *>(s);
}

std::ostream &operator<<(std::ostream &os, ustring const &s)
{
	os << reinterpret_cast<char const *>(s.c_str());
	return os;
}

template <typename CharSeq>
std::ostream &print_hex(std::ostream &os, CharSeq &&seq_)
{
	auto seq(string_utils::charseq_adaptor<CharSeq>::apply(
		std::forward<CharSeq>(seq_)
	));
	std::ostream_iterator<char> iter(os);
	string_utils::ascii_hex_formatter<
		decltype(iter), decltype(seq)
	>::apply(iter, seq);
	return os;
}

template <typename CharSeq>
std::ostream &print_hex(std::ostream &os, CharSeq &&seq_, size_t length)
{
	auto seq(string_utils::charseq_adaptor<CharSeq>::apply(
		std::forward<CharSeq>(seq_), length
	));
	std::ostream_iterator<char> iter(os);
	string_utils::ascii_hex_formatter<
		decltype(iter), decltype(seq)
	>::apply(iter, std::move(seq));
	return os;
}

}

namespace boost::test_tools::tt_detail {

template <>
struct print_log_value<ucpf::yesod::test::ustring> {
	void operator()(std::ostream &os, ucpf::yesod::test::ustring const &s)
	{
		os.write(
			reinterpret_cast<char const *>(s.data()), s.size()
		);
	}
};

template <>
struct print_log_value<ucpf::yesod::test::ustring_view> {
	void operator()(
		std::ostream &os, ucpf::yesod::test::ustring_view const &s
	)
	{
		os.write(
			reinterpret_cast<char const *>(s.data()), s.size()
		);
	}
};

}
#endif

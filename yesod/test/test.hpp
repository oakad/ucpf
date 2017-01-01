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

namespace ucpf::yesod::test {

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
#endif


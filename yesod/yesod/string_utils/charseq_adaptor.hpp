/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_352F7C4CCB9E3B97BAE3AA7985D87019)
#define HPP_352F7C4CCB9E3B97BAE3AA7985D87019

#include <string>
#include <experimental/string_view>

namespace ucpf::yesod::string_utils {

template <typename CharSeq>
struct charseq_adaptor {};

template <typename CharType, typename Traits, typename Alloc>
struct charseq_adaptor<std::basic_string<CharType, Traits, Alloc>> {
	typedef std::basic_string<CharType, Traits, Alloc> result_type;

	static auto apply(
		std::basic_string<CharType, Traits, Alloc> const &str
	)
	{
		return str;
	}
};

template <typename CharType, typename Traits>
struct charseq_adaptor<
	std::experimental::basic_string_view<CharType, Traits
>> {
	typedef std::experimental::basic_string_view<
		CharType, Traits
	> result_type;

	static auto apply(std::experimental::basic_string_view<
		CharType, Traits
	> const &str)
	{
		return str;
	}
};

template <>
struct charseq_adaptor<char const *> {
	typedef std::experimental::string_view result_type;

	static auto apply(char const *str)
	{
		return std::experimental::string_view(str);
	}
};

template <>
struct charseq_adaptor<uint8_t const * (&)> {
	typedef std::experimental::string_view result_type;

	static auto apply(uint8_t const * &str, size_t length)
	{
		return std::experimental::string_view(
			reinterpret_cast<char const *>(str), length
		);
	}
};

template <>
struct charseq_adaptor<uint8_t * (&)> {
	typedef std::experimental::string_view result_type;

	static auto apply(uint8_t * &str, size_t length)
	{
		return std::experimental::string_view(
			reinterpret_cast<char const *>(str), length
		);
	}
};

template <size_t N>
struct charseq_adaptor<char const (&) [N]> {
	typedef std::experimental::string_view result_type;

	static auto apply(char const (&str)[N])
	{
		return std::experimental::string_view(str, N ? N - 1 : 0);
	}
};

template <size_t N>
struct charseq_adaptor<uint8_t (&) [N]> {
	typedef std::experimental::string_view result_type;

	static auto apply(uint8_t (&str)[N])
	{
		return std::experimental::string_view(
			reinterpret_cast<char *>(str), N
		);
	}
};

template <>
struct charseq_adaptor<void * (&)> {
	typedef std::experimental::string_view result_type;

	static auto apply(void * &str, size_t length)
	{
		return std::experimental::string_view(
			reinterpret_cast<char *>(str), length
		);
	}
};

}
#endif

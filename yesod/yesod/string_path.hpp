/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_36A4C68630035F9450192329A4B78968)
#define HPP_36A4C68630035F9450192329A4B78968

#include <yesod/detail/string_path_impl.hpp>
#include <yesod/string_utils/charseq_adaptor.hpp>
#include <yesod/string_utils/u8string_tokenizer_sce.hpp>

namespace ucpf::yesod {

struct string_path {
	typedef typename detail::string_path_impl<>::size_type size_type;

	struct element {
	private:
		detail::ptr_or_data data;
	};

	struct element_iter {
	};

	template <typename SeqParser>
	string_path(SeqParser &&parser)
	{
		auto str_sz(parser.char_count());
		if (detail::string_path_impl<0>::size_limit >= str_sz)
			detail::string_path_impl<0>::init(
				data, str_sz, std::forward<SeqParser>(parser)
			);
		else
			detail::string_path_impl<2>::init(
				data, str_sz, std::forward<SeqParser>(parser)
			);
	}

	template <typename SeqParser>
	string_path(SeqParser &&parser, pmr::memory_resource *mem_alloc)
	{
		auto str_sz(parser.char_count());
		if (detail::string_path_impl<1>::size_limit >= str_sz)
			detail::string_path_impl<1>::init(
				data, str_sz, std::forward<SeqParser>(parser),
				mem_alloc
			);
		else
			detail::string_path_impl<3>::init(
				data, str_sz, std::forward<SeqParser>(parser),
				mem_alloc
			);
	}

	string_path(string_path &&other);

	string_path(string_path const &other);

	template <typename CharSeq>
	static string_path from_posix_like_string(CharSeq &&seq);

	template <typename CharSeq>
	static string_path from_posix_like_string(
		CharSeq &&seq, pmr::memory_resource *mem_alloc
	);

	~string_path()
	{
		detail::string_path_impl<>::select(data).deallocate(data);
	}

	element at(std::size_t pos) const;

	element_iter begin() const;

	element_iter end() const;

	template <typename ...PathType>
	string_path cat(PathType &&...other) const;

	string_path sub_head(string_path const &other) const;

	string_path sub_tail(string_path const &other) const;

	size_type size() const
	{
		return detail::string_path_impl<>::select(
			data
		).element_count(data);
	}

	size_type byte_count() const;

	size_type common_head_size(string_path const &other) const;

	size_type common_tail_size(string_path const &other) const;

private:
	detail::ptr_or_data data;
};

template <typename CharSeq>
string_path string_path::from_posix_like_string(CharSeq &&seq)
{
	typedef string_utils::charseq_adaptor<CharSeq> adaptor_type;

	return string_path(string_utils::u8string_tokenizer_sce<
		typename adaptor_type::result_type
	>(adaptor_type::apply(std::forward<CharSeq>(seq)), '/', '\\'));
}

template <typename CharSeq>
string_path string_path::from_posix_like_string(
	CharSeq &&seq, pmr::memory_resource *mem_alloc
)
{
	typedef string_utils::charseq_adaptor<CharSeq> adaptor_type;

	return string_path(string_utils::u8string_tokenizer_sce<
		typename adaptor_type::result_type
	>(
		adaptor_type::apply(std::forward<CharSeq>(seq)), '/', '\\'
	), mem_alloc);
}

}
#endif

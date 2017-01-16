/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_36A4C68630035F9450192329A4B78968)
#define HPP_36A4C68630035F9450192329A4B78968

#include <yesod/iterator/facade.hpp>
#include <yesod/detail/string_path_impl.hpp>
#include <yesod/string_utils/charseq_adaptor.hpp>
#include <yesod/string_utils/u8string_tokenizer_sce.hpp>

namespace ucpf::yesod {

struct string_path {
	typedef typename detail::string_path_impl<>::size_type size_type;
	typedef typename detail::string_path_impl<>::value_type value_type;
	typedef typename detail::string_path_impl<>::const_iterator const_iterator;

	string_path()
	{
		detail::string_path_impl<0>::common_init(data, 0);
	}

	string_path(string_path &&other)
	{
		data.copy_from(other.data);
		other.data.reset();
	}

	string_path(string_path const &other)
	{
		detail::string_path_impl<>::select(other.data).copy(
			data, other.data
		);
	}

	template <typename SeqParser>
	string_path(
		SeqParser &&parser, pmr::memory_resource *mem_alloc = nullptr
	)
	{
		auto str_sz(parser.char_count());
		if (detail::string_path_impl<0>::size_limit >= str_sz)
			detail::string_path_impl<0>::init_p(
				data, str_sz, std::forward<SeqParser>(parser)
			);
		else
			detail::string_path_impl<1>::init_p(
				data, str_sz, mem_alloc,
				std::forward<SeqParser>(parser)
			);
	}

	~string_path()
	{
		detail::string_path_impl<>::select(data).deallocate(data);
	}

	const_iterator begin() const
	{
		return detail::string_path_impl<>::select(
			data
		).cbegin(data);
	}

	const_iterator cbegin() const
	{
		return detail::string_path_impl<>::select(
			data
		).cbegin(data);
	}

	const_iterator end() const
	{
		return detail::string_path_impl<>::select(
			data
		).cend(data);
	}

	const_iterator cend() const
	{
		return detail::string_path_impl<>::select(
			data
		).cend(data);
	}

	size_type size() const
	{
		return detail::string_path_impl<>::select(
			data
		).value_count(data);
	}

	size_type byte_count() const
	{
		return detail::string_path_impl<>::select(
			data
		).byte_count(data);
	}

	auto at(size_type pos) const
	{
		return *(cbegin() + pos);
	}

	size_type common_head_size(string_path const &other) const
	{
		return detail::string_path_impl<>::common_head_size(
			cbegin(), other.cbegin()
		);
	}

	size_type common_tail_size(string_path const &other) const
	{
		return detail::string_path_impl<>::common_tail_size(
			cend(), other.cend()
		);
	}

	string_path head(
		size_type count, pmr::memory_resource *mem_alloc = nullptr
	) const
	{
		return sub_range(cbegin(), cbegin() + count, mem_alloc);
	}

	string_path tail(
		size_type count, pmr::memory_resource *mem_alloc = nullptr
	) const
	{
		return sub_range(cend(), cend() - count, mem_alloc);
	}

	string_path sub_range(
		const_iterator const &first, const_iterator const &last,
		pmr::memory_resource *mem_alloc = nullptr
	) const
	{
		auto str_sz(detail::string_path_impl<>::byte_distance(
			first, last
		));
		string_path rv;

		if (detail::string_path_impl<0>::size_limit >= str_sz)
			detail::string_path_impl<0>::init_i(
				rv.data, str_sz, first, last
			);
		else
			detail::string_path_impl<1>::init_i(
				rv.data, str_sz, mem_alloc, first, last
			);

		return rv;
	}

	bool operator==(string_path const &other) const
	{
		return detail::string_path_impl<>::equals(
			cbegin(), other.cbegin()
		);
	}

	bool operator!=(string_path const &other) const
	{
		return !detail::string_path_impl<>::equals(
			cbegin(), other.cbegin()
		);
	}

	template <typename... StringPathType>
	static string_path cat(
		pmr::memory_resource *mem_alloc, StringPathType &&...other
	)
	{
		auto str_sz((size_type(0) + ... + other.byte_count()));
		string_path rv;

		if (detail::string_path_impl<0>::size_limit >= str_sz)
			detail::string_path_impl<0>::init_c(
				rv.data, str_sz,
				std::forward<StringPathType>(other)...
			);
		else
			detail::string_path_impl<1>::init_c(
				rv.data, str_sz, mem_alloc,
				std::forward<StringPathType>(other)...
			);

		return rv;
	}

	template <typename... StringPathType>
	static string_path cat(
		StringPathType &&...other
	)
	{
		return cat(
			static_cast<pmr::memory_resource *>(nullptr),
			std::forward<StringPathType>(other)...
		);
	}

	template <typename CharSeq>
	static string_path from_posix_like_string(
		CharSeq &&seq, pmr::memory_resource *mem_alloc = nullptr
	)
	{
		typedef string_utils::charseq_adaptor<CharSeq> adaptor_type;

		return string_path(string_utils::u8string_tokenizer_sce<
			typename adaptor_type::result_type
		>(
			adaptor_type::apply(std::forward<CharSeq>(seq)),
			'/', '\\'
		), mem_alloc);
	}
private:
	detail::ptr_or_data data;
};

}
#endif

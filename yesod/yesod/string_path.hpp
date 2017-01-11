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
	typedef typename detail::string_path_impl<>::element value_type;

	struct const_iterator : iterator::facade<
		const_iterator, value_type const,
		std::random_access_iterator_tag, value_type const
	> {
		const_iterator()
		: data_ptr(nullptr), pos(0) {}

	private:
		friend struct string_path;
		friend struct iterator::core_access;

		const_iterator(
			detail::ptr_or_data const *data_ptr_, size_type pos_
		) : data_ptr(data_ptr_), pos(pos_) {}

		reference dereference() const
		{
			return detail::string_path_impl<>::select(
				*data_ptr
			).element_at(*data_ptr, pos);
		}

		bool equal(const_iterator const &other) const
		{
			return (data_ptr == other.data_ptr)
				&& (pos == other.pos);
		}

		void increment()
		{
			++pos;
		}

		void decrement()
		{
			--pos;
		}

		void advance(difference_type count)
		{
			if (count >= 0)
				pos += count;
			else
				pos -= count;
		}

		difference_type distance_to(const_iterator const &other) const
		{
			return difference_type(other.pos) - pos;
		}

		detail::ptr_or_data const *data_ptr;
		size_type pos;
	};

	template <typename SeqParser>
	string_path(
		SeqParser &&parser, pmr::memory_resource *mem_alloc = nullptr
	)
	{
		auto str_sz(parser.char_count());
		if (detail::string_path_impl<0>::size_limit >= str_sz)
			detail::string_path_impl<0>::init(
				data, str_sz, std::forward<SeqParser>(parser)
			);
		else
			detail::string_path_impl<1>::init(
				data, str_sz, std::forward<SeqParser>(parser),
				mem_alloc
			);
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

	template <typename CharSeq>
	static string_path from_posix_like_string(
		CharSeq &&seq, pmr::memory_resource *mem_alloc = nullptr
	);

	~string_path()
	{
		detail::string_path_impl<>::select(data).deallocate(data);
	}

	auto at(size_type pos) const
	{
		return detail::string_path_impl<>::select(data).element_at(
			data, pos
		);
	}

	const_iterator begin() const
	{
		return const_iterator(&data, 0);
	}

	const_iterator cbegin() const
	{
		return const_iterator(&data, 0);
	}

	const_iterator end() const
	{
		return const_iterator(&data, size());
	}

	const_iterator cend() const
	{
		return const_iterator(&data, size());
	}

	template <typename ...PathType>
	string_path cat(
		PathType &&...other, pmr::memory_resource *mem_alloc = nullptr
	) const;

	string_path sub_head(
		string_path const &other,
		pmr::memory_resource *mem_alloc = nullptr
	) const;

	string_path sub_tail(
		string_path const &other,
		pmr::memory_resource *mem_alloc = nullptr
	) const;

	size_type size() const
	{
		return detail::string_path_impl<>::select(
			data
		).element_count(data);
	}

	size_type byte_count() const
	{
		return detail::string_path_impl<>::select(
			data
		).byte_count(data);
	}

	size_type common_head_size(string_path const &other) const;

	size_type common_tail_size(string_path const &other) const;

private:
	detail::ptr_or_data data;
};

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

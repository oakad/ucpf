/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_MBP_UNPACK_20140608T2300)
#define UCPF_MINA_MBP_UNPACK_20140608T2300

#include <mina/mbp/raw_encoding.hpp>

namespace ucpf { namespace mina { namespace mbp { namespace detail {

template <typename T, int Kind>
struct unpack_helper;

template <typename T>
struct unpack_helper<T, 0> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		return custom<
			typename std::remove_reference<T>::type
		>::unpack(first, last, std::forward<T>(v));
	}
};

template <typename T>
struct unpack_helper<T, kind_flags::sequence> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!advance_skip(first, last))
				return false;
		}
		if (*first != detail::tuple_start_code)
			return false;
		++first;

		constexpr auto next_kind(detail::classify<
			typename T::value_type, 0,
			yesod::is_sequence<typename T::value_type>::value
		>::value);

		while (first != last) {
			if (*first == detail::byte_skip_code) {
				if (!advance_skip(first, last))
					return false;
			}

			if (*first == detail::tuple_end_code) {
				++first;
				return true;
			}

			typename T::value_type xv;
			if (unpack_helper<T, next_kind>::apply(
				first, last, xv
			))
				v.push_back(xv);
			else
				return false;
		}
		return false;
	}
};

template <typename T>
struct unpack_helper<T, kind_flags::integral> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!advance_skip(first, last))
				return false;
		}

		uint8_t xv(*first);
		auto f_class(field_class<>::from_header(xv));

		typedef typename std::remove_reference<T>::type Tr;
		constexpr bool signed_(std::is_signed<Tr>::value);

		if ((
			f_class.numeric_type_r == numeric_type_rank::n_signed
		) != signed_)
			return false;

		auto s_rank(scalar_rank<>::from_type<Tr>());
		if (s_rank < f_class.scalar_r)
			return false;

		if (f_class.list_size_r != list_size_rank::l3)
			return false;

		if (f_class.scalar_r == scalar_rank<>::i5) {
			v = xv & small_int_mask;
			if (signed_)
				v -= small_int_code_offset;

			++first;
			return true;
		}

		if (xv & small_list_size_mask)
			return false;

		if (!advance_n(first, last, 1))
			return false;

		auto x_first(first);
		if (!advance_n(
			first, last, scalar_rank<>::order[f_class.scalar_r] / 8
		))
			return false;

		v = unpack_integral<signed_, Tr>(x_first, first);
		return true;
	}
};

template <typename T>
struct unpack_helper<T, kind_flags::float_t> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!advance_skip(first, last))
				return false;
		}

		uint8_t xv(*first);
		auto f_class(field_class<>::from_header(xv));

		if (f_class.numeric_type_r != numeric_type_rank::n_float)
			return false;

		if (f_class.list_size_r != list_size_rank::l3)
			return false;

		typedef typename yesod::fp_adapter_type<
			typename std::remove_reference<T>::type
		>::type Tw;
		typedef typename Tw::storage_type Tr;
		constexpr auto s_rank(scalar_rank<>::from_type<Tr>());

		if (s_rank != f_class.scalar_r)
			return false;

		if (xv & small_list_size_mask)
			return false;

		if (!advance_n(first, last, 1))
			return false;

		auto x_first(first);
		if (!advance_n(
			first, last, scalar_rank<>::order[f_class.scalar_r] / 8
		))
			return false;

		v = Tw(unpack_integral<false, Tr>(x_first, first));
		return true;
	}
};

template <typename T>
struct unpack_helper<T, kind_flags::integral | kind_flags::sequence> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!advance_skip(first, last))
				return false;
		}

		uint8_t xv(*first);
		auto f_class(field_class<>::from_header(xv));
		typedef typename std::remove_reference<T>::type::value_type Tr;
		constexpr bool signed_(std::is_signed<Tr>::value);

		if ((
			f_class.numeric_type_r == numeric_type_rank::n_signed
		) != signed_)
			return false;

		auto s_rank(scalar_rank<>::from_type<Tr>());
		if (s_rank < f_class.scalar_r)
			return false;

		size_t list_size(0);
		if (!advance_n(first, last, 1))
			return false;

		auto x_first(first);
		switch (f_class.list_size_r) {
		case list_size_rank::le:
			return false;
		case list_size_rank::l3:
			if (f_class.scalar_r == scalar_rank<>::i5) {
				v.push_back(Tr(
					xv & small_int_mask
				) - small_int_code_offset);
				return true;
			} else
				list_size = (xv & small_list_size_mask) + 1;

			break;
		default:
			if (!advance_n(first, last, f_class.list_size_r))
				return false;

			list_size = unpack_integral<false, size_t>(
				x_first, first
			) + 1;
		}

		if (!list_size)
			return false;

		x_first = first;
		auto x_step(scalar_rank<>::order[f_class.scalar_r] / 8);
		if (!advance_n(first, last, x_step * list_size))
			return false;

		for (; list_size; --list_size) {
			auto x_last(x_first);
			std::advance(x_last, x_step);
			v.push_back(
				unpack_integral<signed_, Tr>(x_first, x_last)
			);
			x_first = x_last;
		}

		return true;
	}
};

template <typename T>
struct unpack_helper<T, kind_flags::float_t | kind_flags::sequence> {
	template <typename ForwardIterator>
	static bool apply(ForwardIterator &first, ForwardIterator last, T &&v)
	{
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!advance_skip(first, last))
				return false;
		}

		uint8_t xv(*first);
		auto f_class(field_class<>::from_header(xv));
		typedef typename yesod::fp_adapter_type<
			typename std::remove_reference<T>::type::value_type
		>::type Tw;
		typedef typename Tw::storage_type Tr;

		if (f_class.numeric_type_r != numeric_type_rank::n_float)
			return false;

		auto s_rank(scalar_rank<>::from_type<Tr>());
		if (s_rank != f_class.scalar_r)
			return false;

		size_t list_size(0);
		++first;
		auto x_first(first);
		switch (f_class.list_size_r) {
		case list_size_rank::le:
			return false;
		case list_size_rank::l3:
			list_size = (xv & small_list_size_mask) + 1;
			break;
		default:
			if (!advance_n(first, last, f_class.list_size_r))
				return false;

			list_size = unpack_integral<false, size_t>(
				x_first, first
			) + 1;
		}

		if (!list_size)
			return false;

		x_first = first;
		auto x_step(scalar_rank<>::order[f_class.scalar_r] / 8);
		if (!advance_n(first, last, x_step * list_size))
			return false;

		for (; list_size; --list_size) {
			auto x_last(x_first);
			std::advance(x_last, x_step);
			v.push_back(
				Tw(unpack_integral<false, Tr>(x_first, x_last))
			);
			x_first = x_last;
		}

		return true;
	}
};

template <typename ForwardIterator, typename T>
bool unpack(ForwardIterator &first, ForwardIterator last, T &&v)
{
	typedef typename std::remove_reference<T>::type Tr;

	return unpack_helper<T, detail::classify<
		Tr, 0, yesod::is_sequence<Tr>::value
	>::value>::apply(first, last, std::forward<T>(v));
}

template <typename ForwardIterator, typename T, typename ...Tn>
bool unpack(ForwardIterator &first, ForwardIterator last, T &&v, Tn &&...vn)
{
	return unpack(first, last, std::forward<T>(v))
	       && unpack(first, last, std::forward<Tn>(vn)...);
}

}}}}
#endif

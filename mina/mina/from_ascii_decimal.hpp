/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_FROM_ASCII_DECIMAL_20140815T2300)
#define UCPF_MINA_FROM_ASCII_DECIMAL_20140815T2300

#include <mina/detail/from_ascii_decimal_f.hpp>

namespace ucpf { namespace mina {
namespace detail {

template <typename T, bool IsFloat = false>
struct from_ascii_decimal_converter {
	template <typename ForwardIterator>
	static void parse_negative_tail(
		ForwardIterator &first, ForwardIterator last, T &v
	)
	{
		constexpr static auto v_min(std::numeric_limits<T>::min());

		while (first != last) {
			if (!std::isdigit(*first))
				return;

			if ((v_min / 10) > v) {
				v = v_min;
				break;
			}

			auto v_next(v * 10);
			if ((v_min + (*first - '0')) > v_next) {
				v = v_min;
				break;
			}

			v = v_next - (*first - '0');
			++first;
		}

		while ((first != last) && std::isdigit(*first))
			++first;
	}

	template <typename ForwardIterator>
	static void parse_positive_tail(
		ForwardIterator &first, ForwardIterator last, T &v
	)
	{
		constexpr static auto v_max(std::numeric_limits<T>::max());

		while (first != last) {
			if (!std::isdigit(*first))
				return;

			if ((v_max / 10) < v) {
				v = v_max;
				break;
			}

			auto v_next(v * 10);
			if ((v_max - (*first - '0')) < v_next) {
				v = v_max;
				break;
			}

			v = v_next + (*first - '0');
			++first;
		}

		while ((first != last) && std::isdigit(*first))
			++first;
	}

	template <typename ForwardIterator>
	static bool parse_unsigned(
		ForwardIterator &first, ForwardIterator last, T &v
	)
	{
		auto x_first(first);

		if (x_first == last)
			return false;

		if (*x_first == '+')
			++x_first;

		if ((x_first == last) || !std::isdigit(*x_first))
			return false;

		v = *x_first - '0';
		first = x_first;
		++first;
		parse_positive_tail(first, last, v);
		return true;
	}

	template <typename ForwardIterator>
	static bool parse_signed(
		ForwardIterator &first, ForwardIterator last, T &v
	)
	{
		auto x_first(first);

		if (x_first == last)
			return false;

		bool sign(*x_first == '-');

		if (sign || (*x_first == '+'))
			++x_first;

		if ((x_first == last) || !std::isdigit(*x_first))
			return false;

		if (!sign)
			v = *x_first - '0';
		else
			v = -(*x_first - '0');

		first = x_first;
		++first;

		if (!sign)
			parse_positive_tail(first, last, v);
		else
			parse_negative_tail(first, last, v);

		return true;
	}

	template <typename ForwardIterator, typename Alloc>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, T &v,
		Alloc const &a
	)
	{
		return std::is_signed<T>::value
		       ? parse_signed(first, last, v)
		       : parse_unsigned(first, last, v);
	}
};

template <typename T>
struct from_ascii_decimal_converter<T, true> {
	template <typename ForwardIterator, typename Alloc>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, T &v,
		Alloc const &a
	)
	{
		from_ascii_decimal_f<T> cv(first, last, a);
		if (cv.valid)
			v = cv.value;

		return cv.valid;
	}
};

}

template <
	typename ForwardIterator, typename T,
	typename Alloc = std::allocator<void>
>  bool from_ascii_decimal(
	ForwardIterator &first, ForwardIterator last, T &v,
	Alloc const &a = Alloc()
)
{
	return detail::from_ascii_decimal_converter<
		T, std::is_floating_point<T>::value
	>::apply(first, last, v, a);
}

}}

#endif


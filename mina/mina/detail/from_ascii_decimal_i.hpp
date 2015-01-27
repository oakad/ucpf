/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_42631AAF068DCF4D3FAB9C3C98E847BE)
#define HPP_42631AAF068DCF4D3FAB9C3C98E847BE

namespace ucpf { namespace mina { namespace detail {

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_decimal_u(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	constexpr static auto v_max(std::numeric_limits<T>::max());

	if (!std::isdigit(*first))
		return false;

	v = *first - '0';
	++first;

	while (first != last) {
		if (!std::isdigit(*first))
			return true;

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

	return true;
}

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_decimal_neg(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	constexpr static auto v_min(std::numeric_limits<T>::min());

	if (!std::isdigit(*first))
		return false;

	v = -(*first - '0');
	++first;

	while (first != last) {
		if (!std::isdigit(*first))
			return true;

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

	return true;
}

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_decimal_s(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	auto x_first(first);

	if (x_first == last)
		return false;

	bool sign(*x_first == '-');

	if (sign || (*x_first == '+'))
		++x_first;

	if (x_first == last)
		return false;

	if (
		sign ? from_ascii_decimal_neg(v, x_first, last)
		     : from_ascii_decimal_u(v, x_first, last)
	) {
		first = x_first;
		return true;
	}
	else
		return false;
}

}}}
#endif

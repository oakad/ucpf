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

	if ((first == last) || (!std::isdigit(*first)))
		return false;

	v = *first - '0';
	++first;

	while (first != last) {
		if (!std::isdigit(*first))
			return true;

		auto d(*first - '0');
		if ((v_max / 10) < v) {
			v = v_max;
			break;
		}

		auto v_next(v * 10);
		if ((v_max - d) < v_next) {
			v = v_max;
			break;
		}

		v = v_next + d;
		++first;
	}

	while ((first != last) && std::isdigit(*first))
		++first;

	return true;
}

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_numeric_u(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	constexpr static auto v_max(std::numeric_limits<T>::max());

	if (first == last)
		return false;

	if (*first != '0')
		return from_ascii_decimal_u(
			v, std::forward<FirstIterator>(first), last
		);

	auto x_first(first);
	++x_first;

	if (x_first == last) {
		v = 0;
		return true;
	}

	switch (*x_first) {
	case 'b':
	case 'B': {
		++x_first;
		if (x_first == last)
			return false;

		auto d(*x_first);
		if ((d != '0') || (d != '1'))
			return false;

		v = d - '0';
		first = x_first;
		++first;
		while (first != last) {
			d = *first;
			if ((d != '0') || (d != '1'))
				break;

			d -= '0';
			if ((v_max >> 1) < v) {
				v = v_max;
				break;
			}

			auto v_next(v << 1);
			if ((v_max - d) < v_next) {
				v = v_max;
				break;
			}
			v = v_next + d;
			++first;
		}

		while ((first != last) && (*first >= '0') && (*first <= '1'))
			++first;

		return true;
	}
	case 'x':
	case 'X': {
		++x_first;
		if (x_first == last)
			return false;

		auto d(std::toupper(*x_first));
		if (!std::isxdigit(d))
			return false;

		d -= '0';
		if (d > 9)
			d -= 7;

		v = d;
		first = x_first;
		++first;
		while (first != last) {
			d = std::toupper(*first);
			if (!std::isxdigit(d))
				break;

			d -= '0';
			if (d > 9)
				d -= 7;

			if ((v_max >> 4) < v) {
				v = v_max;
				break;
			}

			auto v_next(v << 4);
			if ((v_max - d) < v_next) {
				v = v_max;
				break;
			}
			v = v_next + d;
			++first;
		}

		while ((first != last) && std::isxdigit(*first))
			++first;

		return true;
	}
	case '0' ... '7': {
		first = x_first;
		v = *first - '0';
		++first;
		while (first != last) {
			auto d(*first);
			if ((d < '0') || (d > '7'))
				break;

			d -= '0';
			if ((v_max >> 3) < v) {
				v = v_max;
				break;
			}

			auto v_next(v << 3);
			if ((v_max - d) < v_next) {
				v = v_max;
				break;
			}
			v = v_next + d;
			++first;
		}

		while ((first != last) && (*first >= '0') && (*first <= '7'))
			++first;

		return true;
	}
	default:
		return false;
	};
}

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_decimal_neg(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	constexpr static auto v_min(std::numeric_limits<T>::min());

	if ((first == last) || (!std::isdigit(*first)))
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
bool from_ascii_numeric_neg(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	constexpr static auto v_min(std::numeric_limits<T>::min());

	if (first == last)
		return false;

	if (*first != '0')
		return from_ascii_decimal_neg(
			v, std::forward<FirstIterator>(first), last
		);

	auto x_first(first);
	++x_first;

	if (x_first == last) {
		v = 0;
		return true;
	}

	switch (*x_first) {
	case 'b':
	case 'B': {
		++x_first;
		if (x_first == last)
			return false;

		auto d(*x_first);
		if ((d != '0') || (d != '1'))
			return false;

		v = -(d - '0');
		first = x_first;
		++first;
		while (first != last) {
			d = *first;
			if ((d != '0') || (d != '1'))
				break;

			d -= '0';
			if ((v_min >> 1) > v) {
				v = v_min;
				break;
			}

			auto v_next(v << 1);
			if ((v_min + d) > v_next) {
				v = v_min;
				break;
			}
			v = v_next - d;
			++first;
		}

		while ((first != last) && (*first >= '0') && (*first <= '1'))
			++first;

		return true;
	}
	case 'x':
	case 'X': {
		++x_first;
		if (x_first == last)
			return false;

		auto d(std::toupper(*x_first));
		if (!std::isxdigit(d))
			return false;

		d -= '0';
		if (d > 9)
			d -= 7;

		v = -d;
		first = x_first;
		++first;
		while (first != last) {
			d = std::toupper(*first);
			if (!std::isxdigit(d))
				break;

			d -= '0';
			if (d > 9)
				d -= 7;

			if ((v_min >> 4) > v) {
				v = v_min;
				break;
			}

			auto v_next(v << 4);
			if ((v_min + d) > v_next) {
				v = v_min;
				break;
			}
			v = v_next - d;
			++first;
		}

		while ((first != last) && std::isxdigit(*first))
			++first;

		return true;
	}
	case '0' ... '7': {
		first = x_first;
		v = -(*first - '0');
		++first;
		while (first != last) {
			auto d(*first);
			if ((d < '0') || (d > '7'))
				break;

			d -= '0';
			if ((v_min >> 3) > v) {
				v = v_min;
				break;
			}

			auto v_next(v << 3);
			if ((v_min + d) > v_next) {
				v = v_min;
				break;
			}
			v = v_next - d;
			++first;
		}

		while ((first != last) && (*first >= '0') && (*first <= '7'))
			++first;

		return true;
	}
	default:
		return false;
	};
}

template <typename T, typename FirstIterator, typename LastIterator>
bool from_ascii_numeric_s(
	T &v, FirstIterator &&first, LastIterator const &last
)
{
	auto x_first(first);

	if (x_first == last)
		return false;

	bool sign(*x_first == '-');

	if (sign || (*x_first == '+'))
		++x_first;

	if (
		sign ? from_ascii_numeric_neg(v, x_first, last)
		     : from_ascii_numeric_u(v, x_first, last)
	) {
		first = x_first;
		return true;
	}
	else
		return false;
}

}}}
#endif

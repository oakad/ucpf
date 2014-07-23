/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_F_20140721T2300)
#define UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_F_20140721T2300

#include <mina/detail/float.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct from_ascii_decimal_f;

template <>
struct from_ascii_decimal_f<double> {
	typedef typename yesod::fp_adapter_type<double>::type wrapper_type;
	typedef float_t<wrapper_type::bit_size> adapter_type;
	typedef typename wrapper_type::storage_type storage_type;
	typedef to_ascii_decimal_f_traits<double> traits_type;


	template <typename InputIterator>
	static bool parse_special(
		double &v, InputIterator &first, InputIterator last
	)
	{
		return false;
	}

	template <typename InputIterator>
	static bool parse_value(
		storage_type &m, int32_t &exp_10, bool &sign, bool &exact,
		InputIterator &first, InputIterator last
	)
	{
		auto x_first(first);
		m = 0;
		exp_10 = 0;
		exact = true;
		sign = *x_first == '-';

		if (sign || (*x_first == '+'))
			++x_first;

		bool has_int((x_first != last) && std::isdigit(*x_first));
		while ((x_first != last) && std::isdigit(*x_first)) {
			if ((*x_first == '0') && !m) {
				++first;
				continue;
			}
			m *= 10;
			m += *x_first - '0';
			++exp_10;
			++x_first;
		}

		if (has_int)
			first = x_first;

		if ((x_first == last) || (*x_first != '.')) {
			return has_int;
		}

		++x_first;
		bool has_frac((x_first != last) && std::isdigit(*x_first));
		while ((x_first != last) && std::isdigit(*x_first)) {
			m *= 10;
			m += *first - '0';
			--exp_10;
			++x_first;
		}

		if (has_frac)
			first = x_first;

		return has_int || has_frac;
	}

	template <typename InputIterator>
	static void parse_exponent(
		int32_t &exp_10, InputIterator &first, InputIterator last
	)
	{
		if (first == last)
			return;

		if (std::tolower(*first) != 'e')
			return;

		auto x_first(first);
		++x_first;
		if (x_first == last)
			return;

		bool exp_sign(*x_first == '-');
		if (exp_sign || (*x_first == '+')) {
			++x_first;
			if (x_first == last)
				return;
		}
		int32_t x_exp(0);
		bool rv((x_first != last) && std::isdigit(*x_first));
		while ((x_first != last) && std::isdigit(*x_first)) {
			x_exp *= 10;
			x_exp += *x_first - '0';
			++x_first;
		}

		if (rv) {
			first = x_first;
			if (exp_sign)
				exp_10 -= x_exp;
			else
				exp_10 += x_exp;
		}
	}

	template <typename InputIterator, typename Alloc>
	from_ascii_decimal_f(
		InputIterator &first, InputIterator last, Alloc const &a
	) : value(std::numeric_limits<T>::quiet_NaN()), valid(false)
	{
		valid = parse_special(value, first, last);
		if (valid)
			return;

		bool exact(true);
		bool sign(false);
		storage_type m(0);
		int32_t exp_10(0);

		valid = parse_value(m, exp_10, sign, exact, first, last);
		if (!valid)
			return;

		parse_exponent(exp_10, first, last);

		if (!m) {
			value = sign ? -double(0) : double(0);
			return;
		}

		storage_type error(exact ? 0 : 4);
		adapter_type xv(m, 0);
		xv.normalize();
		error <<= -xv.exp;

		auto exp_bd(binary_pow_10<storage_type>::lookup_exp_2(exp_10));
		if (exp_bd.exp_5 != exp_10) {
			auto adj_bd(binary_pow_10<
				storage_type
			>::lookup_exp_10_rem(exp_10 - exp_bd.exp_5));
			adapter_type adj_v(adj_bd.m, adj_bd.exp_2);
			xv *= adj_v;
		}

		{
			adapter_type adj_v(exp_bd.m, exp_bd.exp_2);
			xv *= adj_v;
		}
	}

	double value;
	bool valid;
};

}}}
#endif

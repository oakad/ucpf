/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_F_20140721T2300)
#define UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_F_20140721T2300

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct from_ascii_decimal_f;

template <>
struct from_ascii_decimal_f<double> {
	typedef yesod::float_t<64> wrapper_type;
	typedef typename wrapper_type::storage_type mantissa_type;

	template <typename InputIterator>
	static bool parse_special(
		double &v, InputIterator &first, InputIterator last
	)
	{
		return false;
	}

	template <typename InputIterator>
	static bool parse_value(
		mantissa_type &m, int32_t &exp_10, bool &sign,
		InputIterator &first, InputIterator last
	)
	{
		auto x_first(first);
		m = 0;
		exp_10 = 0;
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

		bool sign(false);
		mantissa_type m(0);
		int32_t exp_10(0);

		valid = parse_value(m, exp_10, sign, first, last);
		if (!valid)
			return;

		parse_exponent(exp_10, first, last);

		
	}

	double value;
	bool valid;
};

}}}
#endif

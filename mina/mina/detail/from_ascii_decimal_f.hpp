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
#include <mina/detail/binary_pow_10.hpp>
#include <mina/detail/from_ascii_decimal_u.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct from_ascii_decimal_f;

template <>
struct from_ascii_decimal_f<double> {
	typedef double value_type;
	typedef typename yesod::fp_adapter_type<value_type>::type wrapper_type;
	typedef float_t<wrapper_type::bit_size> adapter_type;
	typedef typename wrapper_type::storage_type storage_type;
	typedef to_ascii_decimal_f_traits<value_type> traits_type;
	constexpr static size_t mantissa_digits = 16;
	constexpr static size_t exponent_digits = 3;

	struct m_int_policy {
		constexpr static bool skip_leading_zeros = true;
		constexpr static bool consume_trailing_digits = true;
		constexpr static bool round_tail = true;

		constexpr size_t max_digits() const
		{
			return mantissa_digits;
		}
	};

	struct m_frac_policy {
		constexpr static bool skip_leading_zeros = false;
		constexpr static bool consume_trailing_digits = true;
		constexpr static bool round_tail = true;

		size_t max_digits() const
		{
			return max_digit_cnt;
		}

		m_frac_policy(size_t max_digit_cnt_)
		: max_digit_cnt(max_digit_cnt_)
		{}

		size_t max_digit_cnt;
	};

	struct m_exp_policy {
		constexpr static bool skip_leading_zeros = true;
		constexpr static bool consume_trailing_digits = false;
		constexpr static bool round_tail = false;

		constexpr size_t max_digits() const
		{
			return exponent_digits;
		}
	};

	template <typename InputIterator>
	static bool parse_special(
		value_type &v, InputIterator &first, InputIterator last
	)
	{
		return false;
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

		from_ascii_decimal_u<
			int32_t, m_exp_policy
		> exp(x_first, last);

		if (exp.consumed()) {
			if (exp_sign)
				exp_10 -= exp.value;
			else
				exp_10 += exp.value;

			first = x_first;
		}
	}

	template <typename InputIterator, typename Alloc>
	from_ascii_decimal_f(
		InputIterator &first, InputIterator last, Alloc const &a
	) : value(std::numeric_limits<value_type>::quiet_NaN()), valid(false)
	{
		constexpr static int32_t significand_size(
			8 * sizeof(storage_type)
		);
		constexpr static int32_t mantissa_bits(
			wrapper_type::traits_type::mantissa_bits
		);
		constexpr static int32_t denormal_exponent(
			2 - wrapper_type::traits_type::exponent_bias
			- mantissa_bits
		);

		auto x_first(first);
		bool sign(*x_first == '-');

		if (!sign && (*x_first == '+'))
			++x_first;

		valid = parse_special(value, first, last);
		if (valid)
			return;

		storage_type m(0);

		from_ascii_decimal_u<
			storage_type, m_int_policy
		> m_int(x_first, last);

		if (!m_int.consumed()) {
			if (*x_first != '.')
				return;
		}

		first = x_first;
		m = m_int.value;
		valid = true;
		bool inexact(m_int.tail_cnt);
		int32_t exp_10(0);

		if (*x_first == '.') {
			++x_first;
			from_ascii_decimal_u<
				storage_type, m_frac_policy
			> m_frac(
				x_first, last, m, m_frac_policy(
					(m_int.converted < 16)
					? (16 - m_int.converted)
					: 0
				)
			);

			if (m_frac.consumed()) {
				m = m_frac.value;
				exp_10 -= m_frac.converted;
				first = x_first;
				inexact |= m_frac.tail_cnt && !m_frac.zero_tail;
			}
		} else
			exp_10 += m_int.tail_cnt;

		parse_exponent(exp_10, first, last);

		if (!m) {
			value = sign ? -value_type(0) : value_type(0);
			return;
		}

		int32_t error(inexact ? 4 : 0);
		adapter_type xv(m, 0);
		printf("--1- %016zX, %d\n", xv.m, xv.exp);
		xv.normalize();
		printf("--2- %016zX, %d\n", xv.m, xv.exp);
		error <<= -xv.exp;

		auto exp_bd(binary_pow_10<storage_type>::lookup_exp_2(exp_10));
		if (exp_bd.exp_5 != exp_10) {
			printf("--3- ref %d, exp %d\n", exp_bd.exp_5, exp_10);
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

		error += 8 + (error ? 1 : 0);
		printf("--4- %016zX, %d err %d, %d\n", xv.m, xv.exp, error, denormal_exponent);

		auto m_size(xv.exp + significand_size);
		if (m_size >= (denormal_exponent + mantissa_bits))
			m_size = mantissa_bits;
		else if (m_size <= denormal_exponent)
			m_size = 0;
		else
			m_size -= denormal_exponent;

		m_size = significand_size - m_size;
		if ((m_size + 3) >= significand_size) {
			auto shift((m_size + 3) - significand_size + 1);
			xv.m >>= shift;
			xv.exp += shift;
			error = (error >> shift) + 1 + 8;
			m_size -= shift;
		}
		printf("--5- %016zX, %d err %d m_size %d\n", xv.m, xv.exp, error, m_size);
		auto half(storage_type(1) << (m_size - 1));
		auto m_mask((storage_type(1) << m_size) - 1);
		auto x_m(xv.m & m_mask);
		printf("--6- bm %016zX b %016zX half %016zX\n", m_mask, x_m, half);
		half <<= 3;
		x_m <<= 3;
		xv.m >>= m_size;
		xv.exp += m_size;
		if (x_m >= (half + error))
			xv.m += 1;

		
	}

	value_type value;
	bool valid;
};

}}}
#endif

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
#include <mina/detail/bigint.hpp>
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
	constexpr static auto limb_digits_10 = bigint::limb_digits_10;
	constexpr static size_t mantissa_digits = 19;
	constexpr static size_t exponent_digits = 3;

	template <typename V>
	struct int_policy {
		constexpr static bool skip_leading_zeros = true;
		constexpr static bool consume_trailing_digits = true;
		constexpr static bool round_tail = true;

		int_policy(V &digits_, size_t pos_)
		: digits(digits_), pos(pos_)
		{}

		constexpr size_t max_digits() const
		{
			return mantissa_digits;
		}

		void push_digit(int c)
		{
			if (!(pos % limb_digits_10))
				digits.push_back(0);

			digits.back() *= 10;
			digits.back() += c & 0xf;
			++pos;
		}

		V &digits;
		size_t pos;
	};

	template <typename V>
	struct frac_policy : int_policy<V> {
		constexpr static bool skip_leading_zeros = false;
		constexpr static bool consume_trailing_digits = true;
		constexpr static bool round_tail = true;

		frac_policy(V &digits_, size_t pos_, size_t max_digit_cnt_)
		: int_policy<V>(digits_, pos_), max_digit_cnt(max_digit_cnt_)
		{}

		size_t max_digits() const
		{
			return max_digit_cnt;
		}

		void push_digit(int c)
		{
			int_policy<V>::push_digit(c);
		}

		size_t max_digit_cnt;
	};

	struct exp_policy {
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

		from_ascii_decimal_u<int32_t, exp_policy> exp(x_first, last);

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

		typedef std::vector<
			bigint::limb_type,
			typename std::allocator_traits<
				Alloc
			>::template rebind_alloc<bigint::limb_type>
		> bigint_type;

		auto x_first(first);
		bool sign(*x_first == '-');

		if (sign || (*x_first == '+'))
			++x_first;

		valid = parse_special(value, first, last);
		if (valid)
			return;

		storage_type m(0);
		bigint_type digits(a);
		int_policy<bigint_type> int_p(digits, 0);

		from_ascii_decimal_u<storage_type, decltype(int_p)> m_int(
			x_first, last,
			std::forward<decltype(int_p)>(int_p), 0
		);

		if (!m_int.consumed()) {
			if (*x_first != '.')
				return;
		}

		first = x_first;
		m = m_int.value;
		valid = true;
		bool inexact(m_int.tail_cnt);
		int32_t exp_10(0), d_exp_10(0);
		auto digit_cnt(m_int.converted);
		auto full_digit_cnt(int_p.pos);

		if (*x_first == '.') {
			++x_first;
			frac_policy<bigint_type> frac_p(
				digits, int_p.pos,
				(m_int.converted < mantissa_digits)
				? (mantissa_digits - m_int.converted) : 0
			);
			from_ascii_decimal_u<
				storage_type, decltype(frac_p)
			> m_frac(
				x_first, last,
				std::forward<decltype(frac_p)>(frac_p), m
			);
			full_digit_cnt = frac_p.pos;

			if (m_frac.consumed()) {
				if (!inexact) {
					m = m_frac.value;
					exp_10 -= m_frac.converted;
					d_exp_10 -= m_frac.consumed();
					inexact = m_frac.tail_cnt
						  && !m_frac.zero_tail;
					digit_cnt += m_frac.converted;
				} else
					exp_10 += m_int.tail_cnt;

				first = x_first;
			}
		} else {
			exp_10 += m_int.tail_cnt;
			d_exp_10 = exp_10;
		}

		d_exp_10 -= exp_10;
		parse_exponent(exp_10, first, last);

		if (!m) {
			value = sign ? -value_type(0) : value_type(0);
			return;
		}

		int32_t error(inexact ? 4 : 0);
		adapter_type xv(m, 0);
		printf("--1- %016zX, %d, exp_10 %d\n", xv.m, xv.exp, exp_10);
		xv.normalize();
		printf("--2- %016zX, %d\n", xv.m, xv.exp);
		error <<= -xv.exp;

		auto exp_bd(binary_pow_10<storage_type>::lookup_exp_2(exp_10));
		printf("--3- ref %d, exp %d\n", exp_bd.exp_5, exp_10);
		if (exp_bd.exp_5 != exp_10) {
			auto adj_bd(binary_pow_10<
				storage_type
			>::lookup_exp_10_rem(exp_10 - exp_bd.exp_5));
			adapter_type adj_v(adj_bd.m, adj_bd.exp_2);
			xv *= adj_v;
			printf("-x3- %016zX, %d\n", xv.m, xv.exp);
			if (
				(mantissa_digits - digit_cnt)
				< (exp_10 - exp_bd.exp_5)
			)
				error += 4;
		}

		{
			adapter_type adj_v(exp_bd.m, exp_bd.exp_2);
			printf("-y3- %016zX, %d * %016zX, %d, err %d\n", xv.m, xv.exp, adj_v.m, adj_v.exp, error);
			xv *= adj_v;
			auto x_exp(xv.exp);
			xv.normalize();
			error += 8 + (error ? 1 : 0);
			printf("-z3- %d, %d, %d\n", error, x_exp, xv.exp);
			error <<= x_exp - xv.exp;
		}

		printf("--4- %016zX, %d err %d\n", xv.m, xv.exp, error);

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

		printf("--7- %016zX, %d, h %016zX, err %d\n", xv.m, xv.exp, half, error);
		value = value_type(xv);
		if (((half - error) < x_m) && ((half + error) > x_m)) {
			auto c_exp_10(0);
			if (
				(digits.size() > 1)
				&& (full_digit_cnt % limb_digits_10)
			) {
				c_exp_10 = limb_digits_10
					   - full_digit_cnt % limb_digits_10;
				digits.back() *= small_power_10[c_exp_10];
			}

			printf("aa %zd ", full_digit_cnt);
			for (auto d: digits)
				printf("%zu ", d);
			printf("\n");

			bigint::dec_to_bin(digits);

			printf("bb %zd ", full_digit_cnt);
			for (auto d: digits)
				printf("%zu ", d);
			printf("\n");

			auto upper(xv);
			upper.m <<= 1;
			upper.m += 1;
			upper.exp -= 1;

			bigint_type x_upper;
			bigint::assign_scalar(
				x_upper, upper.m//, yesod::clz(upper.m)
			);
			printf("-x7- exp_10 %d, exp_2 %d, d_exp_10 %d\n", exp_10, upper.exp, d_exp_10);

			d_exp_10 += exp_10;

			printf("dig1 %s\n", bigint::to_ascii_hex(digits).data());
			printf("upp1 %s\n", bigint::to_ascii_hex(x_upper).data());
			if ((d_exp_10 - c_exp_10) >= 0)
				bigint::multiply_pow10(
					digits, d_exp_10 - c_exp_10
				);
			else
				bigint::multiply_pow10(
					x_upper, c_exp_10 - d_exp_10
				);

			printf("dig2 %s\n", bigint::to_ascii_hex(digits).data());
			printf("upp2 %s\n", bigint::to_ascii_hex(x_upper).data());
			if (upper.exp >= 0)
				bigint::shift_left(x_upper, upper.exp);
			else
				bigint::shift_left(digits, -upper.exp);

			auto ld(yesod::clz(digits.back()));
			auto lu(yesod::clz(x_upper.back()));
			if (ld >= lu)
				bigint::shift_left(digits, ld - lu);
			else
				bigint::shift_left(x_upper, lu - ld);

			printf("dig3 %s\n", bigint::to_ascii_hex(digits).data());
			printf("upp3 %s\n", bigint::to_ascii_hex(x_upper).data());
			bigint::shift_left(digits, yesod::clz(digits.back()));
			bigint::shift_left(x_upper, yesod::clz(x_upper.back()));
			
			int c(bigint::compare(digits, x_upper));
			printf("--8- %016zX, %d, c %d\n", upper.m, upper.exp, c);

			if ((c > 0) || (!c && (wrapper_type(value).get_mantissa() & 1))) {
				//xv.m += sign ? -1 : 1;
				xv.m += 1;
				value = value_type(xv);
			}
		}

		if (sign)
			value = -value;
	}

	value_type value;
	bool valid;
};

}}}
#endif

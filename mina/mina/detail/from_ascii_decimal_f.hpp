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
	constexpr static auto limb_bits = bigint::limb_bits;
	constexpr static auto limb_digits_10 = bigint::limb_digits_10;
	/* Loose upper limit for decimal exponent */
	constexpr static auto exponent_bound = small_power_10_estimate(
		1 << wrapper_type::traits_type::exponent_bits
	);

	template <typename Vector>
	struct num_reader {
		num_reader(Vector &digits_)
		: digits(digits_), pos(0), z_tail(0)
		{}

		template <typename InputIterator>
		size_t append(InputIterator &first, InputIterator last)
		{
			size_t rv(0);
			while ((first != last) && std::isdigit(*first)) {
				if (!(pos % limb_digits_10))
					digits.push_back(0);

				digits.back() *= 10;
				if (*first != '0') {
					digits.back() += *first - '0';
					z_tail = 0;
				} else
					++z_tail;
				++pos;
				++first;
				++rv;
			}
			return rv;
		}

		template <typename InputIterator>
		size_t append_trailing(
			InputIterator &first, InputIterator last
		)
		{
			size_t rv(0);
			while ((first != last) && std::isdigit(*first)) {
				if (*first != '0')
					return rv + append(first, last);

				++first;
				++rv;
			}
			return rv;
		}

		void adjust_tail()
		{
			if (digits.empty())
				return;

			bool multi_limb(digits.size() > 1);

			if (!digits.back()) {
				auto diff(
					(pos % limb_digits_10)
					? (pos % limb_digits_10)
					: limb_digits_10
				);
				pos -= diff;
				z_tail -= diff;

				digits.pop_back();

				while (!digits.back()) {
					pos -= limb_digits_10;
					z_tail -= limb_digits_10;
					digits.pop_back();
				}

				if (digits.empty())
					return;
			} else {
				if (pos % limb_digits_10) {
					digits.back() *= small_power_10[
						limb_digits_10
						- pos % limb_digits_10
					];
					z_tail += limb_digits_10
						  - pos % limb_digits_10;
				}
			}

			if (multi_limb)
				pos -= z_tail;
		}

		Vector &digits;
		size_t pos;
		size_t z_tail;
	};


	template <typename InputIterator>
	static bool parse_special(
		value_type &v, InputIterator &first, InputIterator last
	)
	{
		return false;
	}

	template <typename Vector, typename InputIterator>
	static void parse_exponent(
		Vector &digits, bool &sign, InputIterator &first,
		InputIterator last
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

		sign = *x_first == '-';
		if (sign || (*x_first == '+')) {
			++x_first;
			if (x_first == last)
				return;
		}

		num_reader<Vector> r(digits);
		if (r.append_trailing(x_first, last))
			first = x_first;
	}

	template <typename Vector, typename U>
	static std::pair<size_t, bool> bigdec_to_scalar(U &dst, Vector &src)
	{
		constexpr static auto dst_digits_10(
			std::numeric_limits<U>::digits10
		);

		if (src.empty())
			return std::make_pair(0, false);

		auto dd(dst_digits_10);
		auto iter(src.begin());
		auto r_val(small_power_10[limb_digits_10] >> 1);
		size_t rv(0);
		dst = 0;

		if (dst_digits_10 > limb_digits_10) {
			while (iter != src.end()) {
				dst *= small_power_10[limb_digits_10];
				dst += *iter++;
				dd -= limb_digits_10;
				rv += limb_digits_10;
				if (dd < limb_digits_10)
					break;
			}

			dst *= small_power_10[dd];

			if (iter == src.end())
				return std::make_pair(rv, true);

			if (!dd) {
				if (*iter >= r_val)
					++dst;

				return std::make_pair(rv, false);
			}
		}

		if (dd < limb_digits_10) {
			printf("--a %d, %zd, %zd\n", dd, src.size(), src.front());
			r_val = small_power_10[limb_digits_10 - dd] >> 1;
			auto rem(*iter % small_power_10[limb_digits_10 - dd]);
			dst += *iter / small_power_10[limb_digits_10 - dd];
			rv += dd;

			if (rem) {
				if (rem >= r_val)
					++dst;
				return std::make_pair(rv, false);
			} else {
				++iter;
				return std::make_pair(rv, iter == src.end());
			}
		} else {
			printf("---zz %zd\n", *iter);
			dst += *iter++;
			rv += limb_digits_10;
			if (iter != src.end()) {
				if (*iter >= r_val)
					++dst;
				return std::make_pair(rv, false);
			} else
				return std::make_pair(rv, true);
		}
	}

	template <typename Vector>
	static void bigdec_to_bigint(Vector &v)
	{
		Vector dst(v.get_allocator());
		dst.reserve(v.size() + v.size() / limb_bits + 1);
		dst.push_back(v.front());

		for (auto iter(++v.begin()); iter != v.end(); ++iter) {
			bigint::multiply(dst, small_power_10[limb_digits_10]);
			bigint::add(dst, *iter);
		}

		v.swap(dst);
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

		bigint_type digits(a);
		num_reader<bigint_type> m_r(digits);
		if (!m_r.append_trailing(x_first, last)) {
			if ((x_first != last) && *x_first != '.')
				return;
		} else {
			first = x_first;
			valid = true;
		}

		bool check_for_exp(true);
		int32_t exp_10(0);
		auto int_pos(m_r.pos);
		printf("aaa %zd\n", int_pos);

		if ((x_first != last) && (*x_first == '.')) {
			++x_first;
			if (!int_pos) {
				auto f_sz(m_r.append_trailing(x_first, last));
				if (!f_sz)
					check_for_exp = false;
				else {
					if (f_sz > m_r.pos)
						exp_10 -= f_sz - m_r.pos;

					first = x_first;
					valid = true;
				}
			} else if (m_r.append(x_first, last)) {
				first = x_first;
				valid = true;
			} else
				check_for_exp = false;
		}

		if (!valid)
			return;

		printf("bbb %zd, %d\n", m_r.pos, exp_10);
		storage_type m(0);
		m_r.adjust_tail();
		auto m_cnt(bigdec_to_scalar(m, digits));
		int32_t error(m_cnt.second ? 0 : 4);

		exp_10 -= m_cnt.first - int_pos;

		if (check_for_exp) {
			bigint_type exp_digits(a);
			bool exp_sign(false);
			parse_exponent(exp_digits, exp_sign, first, last);
			if (!exp_digits.empty()) {
				if (
					(exp_digits.size() > 1)
					|| (exp_digits.back() >= exponent_bound)
				) {
					if (exp_sign)
						value = value_type(0);
					else
						value = std::numeric_limits<
							value_type
						>::infinity();

					if (sign)
						value = -value;

					return;
				}
				if (exp_sign)
					exp_10 -= exp_digits.back();
				else
					exp_10 += exp_digits.back();
			}
		}

		printf("ccc %zd\n", m_r.pos);

		if (!m_r.pos) {
			value = sign ? -value_type(0) : value_type(0);
			return;
		}

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
			if ((
				std::numeric_limits<
					decltype(m)
				>::digits10 - m_cnt.first
			) < (exp_10 - exp_bd.exp_5))
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

			printf("aa %zd ", m_r.pos);
			for (auto d: digits)
				printf("%zu ", d);
			printf("\n");

			bigdec_to_bigint(digits);

			printf("bb %zd ", m_r.pos);
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
			int32_t d_exp_10(0);
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

			if ((c > 0) || (
				!c && (wrapper_type(value).get_mantissa() & 1)
			)) {
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

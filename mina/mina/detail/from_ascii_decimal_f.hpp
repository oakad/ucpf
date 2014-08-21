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
struct from_ascii_decimal_f {
	typedef T value_type;
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
	constexpr static int32_t significand_size = 8 * sizeof(storage_type);
	constexpr static int32_t mantissa_bits
	= wrapper_type::traits_type::mantissa_bits;
	constexpr static int32_t denormal_exponent
	= 2 - wrapper_type::traits_type::exponent_bias - mantissa_bits;

	template <typename Vector>
	struct num_reader {
		num_reader(Vector &digits_)
		: digits(digits_), pos(0)
		{}

		template <typename ForwardIterator>
		size_t append(ForwardIterator &first, ForwardIterator last)
		{
			size_t rv(0);
			while ((first != last) && std::isdigit(*first)) {
				if (!(pos % limb_digits_10))
					digits.push_back(0);

				digits.back() *= 10;
				digits.back() += *first - '0';

				++pos;
				++first;
				++rv;
			}
			return rv;
		}

		template <typename ForwardIterator>
		size_t append_trailing(
			ForwardIterator &first, ForwardIterator last
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

		int32_t adjust_tail()
		{
			int32_t tail_exp(0);

			if (digits.empty())
				return tail_exp;

			if (pos % limb_digits_10) {
				tail_exp = (
					pos % limb_digits_10
				) - limb_digits_10;
				digits.back() *= small_power_10[-tail_exp];
			}

			return tail_exp;
		}

		Vector &digits;
		size_t pos;
	};

	template <typename ForwardIterator>
	bool parse_special(ForwardIterator &first, ForwardIterator last)
	{
		auto x_first(first);

		if ((x_first == last) || (*x_first != '1'))
			return false;
		++x_first;

		if ((x_first == last) || (*x_first != '.'))
			return false;
		++x_first;

		if ((x_first == last) || (*x_first != '#'))
			return false;
		++x_first;

		if (x_first == last)
			return false;

		if (*x_first == 'i') {
			++x_first;

			if ((x_first == last) || (*x_first != 'n'))
				return false;
			++x_first;

			if ((x_first == last) || (*x_first != 'f'))
				return false;
			++x_first;
			first = x_first;
			value = std::numeric_limits<value_type>::infinity();
			return true;
		}

		bool s_nan(false);
		if (*x_first == 's')
			s_nan = true;
		else if (*x_first != 'q')
			return false;

		++x_first;

		if ((x_first == last) || (*x_first != '('))
			return false;
		++x_first;

		storage_type sig(0);
		while (x_first != last) {
			if (std::isdigit(*x_first)) {
				sig *= 10;
				sig += *x_first - '0';
			} else if (*x_first == ')') {
				++x_first;
				break;
			} else
				return false;

			++x_first;
		}

		if (!s_nan)
			value = std::numeric_limits<value_type>::quiet_NaN();
		else
			value = wrapper_type::make_nan(sig);

		first = x_first;
		return true;
	}

	template <typename Vector, typename ForwardIterator>
	static void parse_exponent(
		Vector &digits, bool &sign, ForwardIterator &first,
		ForwardIterator last
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

	template <typename U, typename Vector>
	static size_t bigdec_to_scalar(U &dst, Vector &src)
	{
		constexpr static auto dst_digits_10(
			std::numeric_limits<U>::digits10
		);

		if (src.empty())
			return 0;

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
				return rv;

			if (!dd) {
				if (*iter >= r_val)
					++dst;

				return rv;
			}
		}

		if (dd < limb_digits_10) {
			r_val = small_power_10[limb_digits_10 - dd] >> 1;
			auto rem(*iter % small_power_10[limb_digits_10 - dd]);
			dst += *iter / small_power_10[limb_digits_10 - dd];
			rv += dd;

			if (rem) {
				if (rem >= r_val)
					++dst;
				return rv;
			} else {
				++iter;
				return rv;
			}
		} else {
			dst += *iter++;
			rv += limb_digits_10;
			if (iter != src.end()) {
				if (*iter >= r_val)
					++dst;
				return rv;
			} else
				return rv;
		}
	}

	template <typename Vector>
	static int32_t bigdec_to_bigint(Vector &v)
	{
		Vector dst(v.get_allocator());
		dst.reserve(v.size() + v.size() / limb_bits + 1);
		dst.push_back(v.front());

		for (auto iter(++v.begin()); iter != v.end(); ++iter) {
			bigint::multiply(dst, small_power_10[limb_digits_10]);
			bigint::add(dst, *iter);
		}

		auto ld(small_power_10_estimate(yesod::clz(dst.back())));
		if (ld)
			bigint::multiply(dst, small_power_10[ld]);
		
		v.swap(dst);
		return ld;
	}

	static void scale_guess(
		adapter_type &xv, int32_t &error, int32_t exp_10
	)
	{
		if (exp_10 >= 0) {
			while (exp_10 > binary_pow_10::exp_5_range.second) {
				auto exp_bd(binary_pow_10::lookup_exp_2<
					storage_type
				>(binary_pow_10::exp_5_range.second));
				adapter_type adj_v(exp_bd.m, exp_bd.exp_2);
				xv *= adj_v;
				auto x_exp(xv.exp);
				xv.normalize();
				error += 9;
				error <<= x_exp - xv.exp;
				exp_10 -= binary_pow_10::exp_5_range.second;
			}
		} else {
			while (exp_10 < binary_pow_10::exp_5_range.first) {
				auto exp_bd(binary_pow_10::lookup_exp_2<
					storage_type
				>(binary_pow_10::exp_5_range.first));
				adapter_type adj_v(exp_bd.m, exp_bd.exp_2);
				xv *= adj_v;
				auto x_exp(xv.exp);
				xv.normalize();
				error += 9;
				error <<= x_exp - xv.exp;
				exp_10 -= binary_pow_10::exp_5_range.first;
			}
		}

		auto exp_bd(binary_pow_10::lookup_exp_2<storage_type>(exp_10));
		if (exp_bd.exp_5 != exp_10) {
			auto adj_bd(binary_pow_10::lookup_exp_10_rem<
				storage_type
			>(exp_10 - exp_bd.exp_5));
			adapter_type adj_v(adj_bd.m, adj_bd.exp_2);
			xv *= adj_v;
			error += 9;
		}

		adapter_type adj_v(exp_bd.m, exp_bd.exp_2);
		xv *= adj_v;
		auto x_exp(xv.exp);
		xv.normalize();
		error += 9;
		error <<= x_exp - xv.exp;
	}

	bool compute_guess(adapter_type &xv, int32_t &error, int32_t exp_10)
	{
		xv.normalize();
		error <<= -xv.exp;

		scale_guess(xv, error, exp_10);

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
			error = (error >> shift) + 9;
			m_size -= shift;
		}

		auto half(storage_type(1) << (m_size - 1));
		auto m_mask((storage_type(1) << m_size) - 1);
		auto x_m(xv.m & m_mask);

		half <<= 3;
		x_m <<= 3;
		xv.m >>= m_size;
		xv.exp += m_size;
		if (x_m >= (half + error))
			xv.m += 1;

		value = value_type(xv);
		wrapper_type wv(value);

		if (wv.is_special()) {
			value = std::numeric_limits<value_type>::infinity();
			return true;
		}

		return !(((half - error) < x_m) && ((half + error) > x_m));
	}

	template <typename Vector>
	void adjust_guess(
		adapter_type &xv, Vector &digits, int32_t exp_10
	)
	{
		exp_10 -= bigdec_to_bigint(digits);

		auto upper(xv);
		upper.m <<= 1;
		upper.m += 1;
		upper.exp -= 1;

		Vector x_upper;
		bigint::assign_scalar(x_upper, upper.m);
		auto lu(yesod::clz(x_upper.back()));
		bigint::shift_left(x_upper, lu);
		upper.exp -= lu;

		if (exp_10 >= 0)
			bigint::multiply_pow10(digits, exp_10);
		else
			bigint::multiply_pow10(x_upper, -exp_10);

		if (upper.exp >= 0)
			bigint::shift_left(x_upper, upper.exp);
		else
			bigint::shift_left(digits, -upper.exp);

		int c(bigint::compare(digits, x_upper));

		wrapper_type wv(value);
		if ((c > 0) || (!c && (wv.get_mantissa() & 1))) {
			xv.m += 1;
			value = value_type(xv);
		}
	}

	template <typename ForwardIterator, typename Alloc>
	from_ascii_decimal_f(
		ForwardIterator &first, ForwardIterator last, Alloc const &a
	) : value(std::numeric_limits<value_type>::quiet_NaN()), valid(false)
	{
		typedef std::vector<
			bigint::limb_type, typename std::allocator_traits<
				Alloc
			>::template rebind_alloc<bigint::limb_type>
		> bigint_type;

		if (first == last)
			return;

		auto x_first(first);
		bool sign(*x_first == '-');

		if (sign || (*x_first == '+'))
			++x_first;

		valid = parse_special(x_first, last);
		if (valid) {
			if (sign)
				value = -value;

			first = x_first;
			return;
		}

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

		storage_type m(0);
		auto tail_exp(m_r.adjust_tail());

		auto m_cnt(bigdec_to_scalar(m, digits));
		int32_t m_exp(
			std::numeric_limits<decltype(m)>::digits10 - m_cnt
		);
		int32_t error(4);

		auto d_exp_10(exp_10);
		exp_10 -= m_cnt - int_pos;

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
				if (exp_sign) {
					d_exp_10 -= exp_digits.back();
					exp_10 -= exp_digits.back();
				} else {
					d_exp_10 += exp_digits.back();
					exp_10 += exp_digits.back();
				}
			}
		}

		if (!m_r.pos) {
			value = sign ? -value_type(0) : value_type(0);
			return;
		}

		adapter_type xv(m, 0);
		if (!compute_guess(xv, error, exp_10 - m_exp)) {
			if (int_pos > m_r.pos)
				d_exp_10 += int_pos - m_r.pos;
			else
				d_exp_10 -= m_r.pos - int_pos;

			d_exp_10 += tail_exp;

			adjust_guess(xv, digits, d_exp_10);
		}

		if (sign)
			value = -value;
	}

	value_type value;
	bool valid;
};

}}}
#endif

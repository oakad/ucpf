/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_674AA4CF202FEB22F46159B96D612245)
#define HPP_674AA4CF202FEB22F46159B96D612245

#include <mina/detail/float.hpp>
#include <mina/detail/bigint.hpp>
#include <mina/detail/binary_pow_10.hpp>

namespace ucpf { namespace mina { namespace detail {

template <
	typename T, typename FirstIterator, typename LastIterator,
	typename Alloc
> struct from_ascii_numeric_f_helper {
	typedef T value_type;
	typedef typename yesod::fp_adapter_type<value_type>::type wrapper_type;
	typedef float_t<wrapper_type::bit_size> adapter_type;
	typedef typename wrapper_type::storage_type storage_type;
	typedef to_ascii_decimal_f_traits<value_type> traits_type;

	constexpr static auto limb_bits = bigint::limb_bits;
	constexpr static auto limb_digits_10 = bigint::limb_digits_10;
	/* Loose upper limit for binary and decimal exponent */
	constexpr static auto bin_exponent_bound
	= int32_t(1) << wrapper_type::traits_type::exponent_bits;
	constexpr static auto exponent_bound = small_power_10_estimate(
		bin_exponent_bound
	);
	constexpr static int32_t significand_size = 8 * sizeof(storage_type);
	constexpr static int32_t mantissa_bits
	= wrapper_type::traits_type::mantissa_bits;
	constexpr static int32_t denormal_exponent
	= 2 - wrapper_type::traits_type::exponent_bias - mantissa_bits;
	typedef std::vector<
		bigint::limb_type, typename std::allocator_traits<
			Alloc
		>::template rebind_alloc<bigint::limb_type>
	> bigint_type;


	template <typename Vector>
	struct num_reader {
		num_reader(Vector &digits_)
		: digits(digits_), pos(0)
		{}

		size_t append(FirstIterator &first, LastIterator const &last)
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

		size_t append_trailing(
			FirstIterator &first, LastIterator const &last
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

	static bool parse_special(
		value_type &value, FirstIterator &first,
		LastIterator const &last
	)
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

	static bool parse_hex(
		value_type &value, FirstIterator &first,
		LastIterator const &last
	)
	{
		enum {
			LEAD_ZERO = 0,
			LEAD_EX,
			M_INT_BEGIN,
			M_INT_SKIP_ZEROES,
			M_INT_CONT,
			M_FRAC_BEGIN,
			M_FRAC_SKIP_ZEROES,
			M_FRAC_SKIP_ALL,
			M_FRAC_CONT,
			FINISHED
		} state(LEAD_ZERO);

		storage_type m(0);
		int32_t exp_2(0), exp_ind(0);
		int bit_pos(mantissa_bits);

		auto align_first = [&bit_pos](storage_type &m, uint32_t d) {
			bit_pos -= yesod::fls(d) + 1;
			m = d;
			m <<= bit_pos;
		};

		auto align_last = [&bit_pos](
			storage_type &m, int32_t &exp, uint32_t d
		) {
			m |= storage_type(d) >> (4 - bit_pos);
			m += storage_type(1) & (d >> (3 - bit_pos));
			if (m & (storage_type(1) << mantissa_bits)) {
				m >>= 1;
				exp += 1;
			}
			bit_pos = -1;
		};

		auto x_first(first);
		for (; (x_first != last) && (state != FINISHED); ++x_first) {
			uint32_t d(*x_first);
			switch (state) {
			case LEAD_ZERO:
				if (d != '0')
					return false;

				state = LEAD_EX;
				break;
			case LEAD_EX:
				if (std::toupper(d) != 'X')
					return false;

				state = M_INT_BEGIN;
				break;
			case M_INT_BEGIN:
				if (!std::isxdigit(d)) {
					value = std::numeric_limits<
						value_type
					>::quiet_NaN();
					first = x_first;
					return true;
				}

				if (d == '0') {
					state = M_INT_SKIP_ZEROES;
					break;
				}

				d = (d <= '9') ? (d - '0') : (
					std::toupper(d) - 'A'  + 10
				);

				align_first(m, d);
				exp_2 = mantissa_bits - bit_pos - 1;
				state = M_INT_CONT;
				break;
			case M_INT_SKIP_ZEROES:
				switch (d) {
				case '0':
					break;
				case 'a' ... 'f':
					d -= 'a' - 'A';
				case 'A' ... 'F':
					d -= 'A' - '9' - 1;
				case '1' ... '9':
					d -= '0';
					align_first(m, d);
					exp_2 = mantissa_bits - bit_pos - 1;
					state = M_INT_CONT;
					break;
				case '.':
					state = M_FRAC_BEGIN;
					break;
				case 'P':
				case 'p':
					exp_ind = parse_exponent(
						x_first, last
					);
					state = FINISHED;
					break;
				default:
					state = FINISHED;
				}
				break;
			case M_INT_CONT:
				switch (d) {
				case 'a' ... 'f':
					d -= 'a' - 'A';
				case 'A' ... 'F':
					d -= 'A' - '9' - 1;
				case '0' ... '9':
					d -= '0';

					if (bit_pos < 0)
						exp_2 += 4;
					else if (bit_pos >= 4) {
						bit_pos -= 4;
						m |= storage_type(d) << bit_pos;
						exp_2 += 4;
					} else
						align_last(m, exp_2, d);

					break;
				case '.':
					state = M_FRAC_BEGIN;
					break;
				case 'P':
				case 'p':
					exp_ind = parse_exponent(
						x_first, last
					);
					state = FINISHED;
					break;
				default:
					state = FINISHED;
				}
				break;
			case M_FRAC_BEGIN:
				if (!std::isxdigit(d))
					state = FINISHED;
				else if (bit_pos < 0)
					state = M_FRAC_SKIP_ALL;
				else if (!exp_2) {
					if (d == '0') {
						exp_2 -= 4;
						state = M_FRAC_SKIP_ZEROES;
					} else {
						d = (d <= '9') ? (d - '0') : (
							std::toupper(d)
							- 'A'  + 10
						);

						align_first(m, d);
						exp_2 = bit_pos + 1
							- mantissa_bits;

						state = M_FRAC_CONT;
					}
				} else {
					d = (d <= '9') ? (d - '0') : (
						std::toupper(d) - 'A'  + 10
					);
					if (bit_pos >= 4) {
						bit_pos -= 4;
						m |= storage_type(d) << bit_pos;
					} else if (bit_pos >= 0)
						align_last(m, exp_2, d);

					state = M_FRAC_CONT;
				}
				break;
			case M_FRAC_SKIP_ZEROES:
				switch (d) {
				case '0':
					exp_2 -= 4;
					break;
				case 'a' ... 'f':
					d -= 'a' - 'A';
				case 'A' ... 'F':
					d -= 'A' - '9' - 1;
				case '1' ... '9':
					d -= '0';
					align_first(m, d);
					exp_2 = bit_pos + 1 - mantissa_bits;
					state = M_FRAC_CONT;
					break;
				case 'P':
				case 'p':
					exp_ind = parse_exponent(
						x_first, last
					);
					state = FINISHED;
					break;
				default:
					state = FINISHED;
				}
				break;
			case M_FRAC_SKIP_ALL:
				if (!std::isxdigit(d)) {
					if (std::toupper(d) == 'P')
						exp_ind = parse_exponent(
							x_first, last
						);

					state = FINISHED;
				}
				break;
			case M_FRAC_CONT:
				if (std::isxdigit(d)) {
					d = (d <= '9') ? (d - '0') : (
						std::toupper(d) - 'A'  + 10
					);
					if (bit_pos >= 4) {
						bit_pos -= 4;
						m |= storage_type(d) << bit_pos;
					} else if (bit_pos >= 0)
						align_last(m, exp_2, d);
				} else if (std::toupper(d) == 'P') {
					exp_ind = parse_exponent(
						x_first, last
					);
					state = FINISHED;
				} else
					state = FINISHED;

				break;
			case FINISHED:
				break;
			}
		}

		printf("--1- m %08x, exp_2 %d, exp_ind %d\n", m, exp_2, exp_ind);
		exp_ind += exp_2 + wrapper_type::traits_type::exponent_bias;
		printf("-x1- exp_ind %d\n", exp_ind);
		if (!m)
			value = value_type(0);
		else if (exp_ind <= 0) {
			if (mantissa_bits > (-exp_ind)) {
				m >>= -exp_ind;
				value = wrapper_type(m).get();
			} else
				value = value_type(0);
		} else if (exp_ind >= (bin_exponent_bound - 1)) {
			value = std::numeric_limits<value_type>::infinity();
		} else {
			m ^= storage_type(1) << (mantissa_bits - 1);

			printf("--2- m %08x, exp %d (%x)\n", m, exp_ind, exp_ind);
			m |= storage_type(exp_ind) << (mantissa_bits - 1);
			printf("--3- m %08x\n", m);
			value = wrapper_type(m).get();
		}

		first = x_first;
		return true;
	}

	static int32_t parse_exponent(
		FirstIterator &first, LastIterator const &last
	)
	{
		bool neg(false);
		bool valid(false);
		int32_t rv(0);

		auto x_first(first);
		++x_first;
		if (x_first != last) {
			auto d(*x_first);
			if (std::isdigit(d)) {
				rv = d - '0';
				valid = true;
			} else if (d == '-')
				neg = true;
			else if (d != '+')
				return rv;
		} else
			return rv;

		++x_first;
		if (x_first != last) {
			auto d(*x_first);
			if (std::isdigit(d)) {
				valid = true;
				d -= '0';
				rv *= 10;
				if (neg)
					rv -= d;
				else
					rv += d;
			} else {
				if (valid)
					first = x_first;

				return rv;
			}
		} else {
			if (valid)
				first = x_first;

			return rv;
		}

		first = x_first;
		++first;
		if (neg) {
			constexpr static auto v_min(
				std::numeric_limits<int32_t>::min()
			);

			while (first != last) {
				auto d(*first);
				if (!std::isdigit(d))
					return rv;

				if ((v_min / 10) > rv) {
					rv = v_min;
					break;
				}

				d -= '0';
				auto v_next(rv * 10);
				if ((v_min + d) > v_next) {
					rv = v_min;
					break;
				}

				rv = v_next - d;
				++first;
			}
		} else {
			constexpr static auto v_max(
				std::numeric_limits<int32_t>::max()
			);

			while (first != last) {
				auto d(*first);
				if (!std::isdigit(d))
					return rv;

				if ((v_max / 10) < rv) {
					rv = v_max;
					break;
				}

				d -= '0';
				auto v_next(rv * 10);
				if ((v_max - d) < v_next) {
					rv = v_max;
					break;
				}

				rv = v_next + d;
				++first;
			}
		}

		while ((first != last) && std::isdigit(*first))
			++first;

		return rv;
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
				if (dd < int(limb_digits_10))
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

		if (dd < int(limb_digits_10)) {
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

	static bool compute_guess(
		value_type &value, adapter_type &xv, int32_t &error,
		int32_t exp_10
	)
	{
		xv.normalize();
		error <<= -xv.exp;

		if (exp_10 < binary_pow_10<>::pow_10_range.first) {
			value = value_type(0);
			return true;
		} else if (exp_10 > binary_pow_10<>::pow_10_range.second) {
			value = std::numeric_limits<value_type>::infinity();
			return true;
		}

		auto exp_bd(
			binary_pow_10<>::lookup_pow_2<storage_type>(exp_10)
		);
		if (exp_bd.pow_10 != exp_10) {
			auto adj_bd(binary_pow_10<>::lookup_pow_10_rem<
				storage_type
			>(exp_10 - exp_bd.pow_10));
			adapter_type adj_v(adj_bd.m, adj_bd.pow_2);
			xv *= adj_v;
			error += 9;
		}

		adapter_type adj_v(exp_bd.m, exp_bd.pow_2);
		xv *= adj_v;
		auto x_exp(xv.exp);
		xv.normalize();
		error += 9;
		error <<= x_exp - xv.exp;

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
	static void adjust_guess(
		value_type &value, adapter_type &xv, Vector &digits,
		int32_t exp_10
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

	static bool apply(
		value_type &value, FirstIterator &&first,
		LastIterator const &last, Alloc const &a
	)
	{
		if (first == last)
			return false;

		value = std::numeric_limits<value_type>::quiet_NaN();
		auto x_first(first);
		bool sign(*x_first == '-');

		if (sign || (*x_first == '+'))
			++x_first;

		auto valid(
			parse_special(value, x_first, last)
			|| parse_hex(value, x_first, last)
		);
		if (valid) {
			if (sign)
				value = -value;

			first = x_first;
			return true;
		}

		bigint_type digits(a);
		num_reader<bigint_type> m_r(digits);
		if (!m_r.append_trailing(x_first, last)) {
			if ((x_first != last) && *x_first != '.')
				return false;
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
			return valid;

		storage_type m(0);
		auto tail_exp(m_r.adjust_tail());

		auto m_cnt(bigdec_to_scalar(m, digits));
		int32_t m_exp(
			std::numeric_limits<decltype(m)>::digits10 - m_cnt
		);
		int32_t error(4);

		auto d_exp_10(exp_10);
		exp_10 -= m_cnt - int_pos;

		if (
			check_for_exp && (first != last)
			&& (std::toupper(*first) == 'E')
		) {
			auto exp_ind(parse_exponent(first, last));

			if (abs(exp_ind) > exponent_bound) {
				if (exp_ind < 0)
					value = value_type(0);
				else
					value = std::numeric_limits<
						value_type
					>::infinity();

				if (sign)
					value = -value;

				return valid;
			}

			d_exp_10 += exp_ind;
			exp_10 += exp_ind;
		}

		if (!m_r.pos) {
			value = sign ? -value_type(0) : value_type(0);
			return valid;
		}

		adapter_type xv(m, 0);
		if (!compute_guess(value, xv, error, exp_10 - m_exp)) {
			if (int_pos > m_r.pos)
				d_exp_10 += int_pos - m_r.pos;
			else
				d_exp_10 -= m_r.pos - int_pos;

			d_exp_10 += tail_exp;

			adjust_guess(value, xv, digits, d_exp_10);
		}

		if (sign)
			value = -value;

		return valid;
	}
};

template <
	typename T, typename FirstIterator, typename LastIterator,
	typename Alloc
> bool from_ascii_numeric_f(
	T &v, FirstIterator &&first, LastIterator const &last, Alloc const &a
)
{
	return from_ascii_numeric_f_helper<
		T, FirstIterator, LastIterator, Alloc
	>::apply(v, std::forward<FirstIterator>(first), last, a);
}

}}}
#endif

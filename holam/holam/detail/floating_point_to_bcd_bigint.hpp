/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_6D2483D3EAFB4C4D3352EAF74173BFA9)
#define HPP_6D2483D3EAFB4C4D3352EAF74173BFA9

#include <holam/detail/floating_point_traits.hpp>
#include <cmath>
#include <cstdio>

namespace ucpf { namespace holam { namespace detail {

template <typename T>
struct floating_point_to_bcd_bigint {
#if defined(_GLIBCXX_USE_INT128)
	typedef bigint_calc_traits<64> calc_traits;
#else
	typedef bigint_calc_traits<32> calc_traits;
#endif
	typedef fp_value_traits<T> value_traits;
	typedef typename calc_traits::limb_type limb_type;

	static void apply(
		uint8_t *m_out, std::size_t &m_len, int32_t &exp_out,
		fp_value_t<T> const &val
	) {
		constexpr static int32_t inv_log2_10(646456993);
		int64_t acc(val.exp + int32_t(value_traits::mantissa_bits) - 1);
		int32_t exp10(acc ? ((acc * inv_log2_10) >> 31) + 1 : 0);
		int disc(exp10 < 0 ? 2 : 1);
		if (val.exp >= 0)
			disc = 0;

		disc = (disc << 1) | (val.is_normal_pow2() ? 1 : 0);
		switch (disc) {
		case 0: {
			auto limb_cnt(limb_count_estimate(val.exp + 1));
			limb_type limbs[3 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));
			setup_high<false>(limbs, limb_cnt, exp10, val);
			produce_digits_high<false>(
				limbs, limb_cnt, val, exp10, m_out, m_len,
				exp_out
			);
			break;
		}
		case 1: {
			auto limb_cnt(limb_count_estimate(val.exp + 2));
			limb_type limbs[3 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));
			setup_high<true>(limbs, limb_cnt, exp10, val);
			produce_digits_high<true>(
				limbs, limb_cnt, val, exp10, m_out, m_len,
				exp_out
			);
			break;
		}
		case 2: {
			auto limb_cnt(limb_count_estimate(1 - val.exp));
			limb_type limbs[3 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));

			setup_mid<false>(limbs, limb_cnt, exp10, val);
			produce_digits_high<false>(
				limbs, limb_cnt, val, exp10, m_out, m_len,
				exp_out
			);
			break;
		}
		case 3: {
			auto limb_cnt(limb_count_estimate(2 - val.exp));
			limb_type limbs[3 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));

			setup_mid<true>(limbs, limb_cnt, exp10, val);
			produce_digits_high<true>(
				limbs, limb_cnt, val, exp10, m_out, m_len,
				exp_out
			);
			break;
		}
		case 4: {
			auto limb_cnt(limb_count_estimate(1 - val.exp));
			int32_t denom_shift(1 - val.exp);
			limb_type limbs[2 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));

			setup_low<false>(limbs, limb_cnt, exp10, val);
			produce_digits_low<false>(
				limbs, limb_cnt, val, exp10, denom_shift,
				m_out, m_len, exp_out
			);
			break;
		}
		case 5: {
			auto limb_cnt(limb_count_estimate(2 - val.exp));
			int32_t denom_shift(2 - val.exp);
			limb_type limbs[2 * limb_cnt];
			__builtin_memset(limbs, 0, sizeof(limbs));

			setup_low<true>(limbs, limb_cnt, exp10, val);
			produce_digits_low<true>(
				limbs, limb_cnt, val, exp10, denom_shift,
				m_out, m_len, exp_out
			);
			break;
		}}
	}

private:
	template <bool Pow2>
	static void setup_high(
		limb_type *limbs, std::size_t limb_cnt, int32_t exp10,
		fp_value_t<T> const &val
	) {
		auto num(limbs);
		auto bound(num + limb_cnt);
		auto denom(bound + limb_cnt);

		assign_pow2(bound, limb_cnt, val.exp);
		assign_pow5(denom, limb_cnt, exp10);
		if (Pow2) {
			shift_left(denom, limb_cnt, exp10 + 2);
			assign_pow2(
				num, limb_cnt,
				val.exp + value_traits::mantissa_bits + 2
			);
		} else {
			shift_left(denom, limb_cnt, exp10 + 1);
			assign_mantissa(num, limb_cnt, val.m);
			shift_left(num, limb_cnt, val.exp + 1);
		}
	}

	template <bool Pow2>
	static void setup_mid(
		limb_type *limbs, std::size_t &limb_cnt, int32_t exp10,
		fp_value_t<T> const &val
	) {
		auto num(limbs);
		auto bound(num + limb_cnt);
		auto denom(bound + limb_cnt);

		bound[0] = 1;
		assign_pow5(denom, limb_cnt, exp10);
		if (Pow2) {
			shift_left(denom, limb_cnt, exp10 + 2 - val.exp);
			assign_pow2(
				num, limb_cnt, 2 + value_traits::mantissa_bits
			);
		} else {
			shift_left(denom, limb_cnt, exp10 + 1 - val.exp);
			assign_mantissa(num, limb_cnt, val.m);
			shift_left_near(num, limb_cnt, 1);
		}
	}

	template <bool Pow2>
	static void setup_low(
		limb_type *limbs, std::size_t &limb_cnt, int32_t exp10,
		fp_value_t<T> const &val
	) {
		auto num(limbs);
		auto bound(num + limb_cnt);

		assign_pow5(bound, limb_cnt, -exp10);
		shift_left(bound, limb_cnt, -exp10);
		if (Pow2) {
			assign_pow5(num, limb_cnt, -exp10);
			shift_left(
				num, limb_cnt,
				2 + value_traits::mantissa_bits - exp10
			);
		} else {
			assign_mantissa(num, limb_cnt, val.m);
			multiply_pow5(num, limb_cnt, -exp10);
			shift_left(num, limb_cnt, 1 - exp10);
		}
	}

	template <bool Pow2>
	static void produce_digits_high(
		limb_type *limbs, std::size_t limb_cnt,
		fp_value_t<T> const &val, int32_t exp10,
		uint8_t *m_out, std::size_t &m_len, int32_t &exp_out
	) {
		auto num(limbs);
		auto bound(limbs + limb_cnt);
		auto denom(bound + limb_cnt);

		bool in_range(false);

		if (Pow2) {
			in_range = compare_sum<true>(
				num, bound, denom, limb_cnt
			) >= 0;
		} else {
			if (val.m & 1)
				in_range = compare_sum<false>(
					num, bound, denom, limb_cnt
				) > 0;
			else
				in_range = compare_sum<false>(
					num, bound, denom, limb_cnt
				) >= 0;
		}

		if (in_range)
			++exp10;
		else {
			multiply_10(num, limb_cnt);
			multiply_10(bound, limb_cnt);
		}

		auto denom_msb(find_msb(denom, limb_cnt));
		m_len = 0;
		while (true) {
			limb_type digit(divide_near(
				num, denom, limb_cnt, denom_msb
			));

			int bd_test(0);
			if (Pow2) {
				bd_test |= compare(
					num, bound, limb_cnt
				) <= 0 ? 1 : 0;
				bd_test |= compare_sum<true>(
					num, bound, denom, limb_cnt
				) >= 0 ? 2 : 0;
			} else {
				if (val.m & 1) {
					bd_test |= compare(
						num, bound, limb_cnt
					) < 0 ? 1 : 0;
					bd_test |= compare_sum<false>(
						num, bound, denom, limb_cnt
					) > 0 ? 2 : 0;
				} else {
					bd_test |= compare(
						num, bound, limb_cnt
					) <= 0 ? 1 : 0;
					bd_test |= compare_sum<false>(
						num, bound, denom, limb_cnt
					) >= 0 ? 2 : 0;
				}
			}

			switch (bd_test) {
			case 0:
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				multiply_10(num, limb_cnt);
				multiply_10(bound, limb_cnt);
				break;
			case 1:
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			case 2:
				++digit;
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			case 3:
				bd_test = compare_sum<false>(
					num, num, denom, limb_cnt
				);
				if ((bd_test > 0) || (!bd_test && (digit & 1)))
					++digit;

				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			}
		}
	}

	template <bool Pow2>
	static void produce_digits_low(
		limb_type *limbs, std::size_t limb_cnt,
		fp_value_t<T> const &val, int32_t exp10, int32_t denom_shift,
		uint8_t *m_out, std::size_t &m_len, int32_t &exp_out
	) {
		auto num(limbs);
		auto bound(limbs + limb_cnt);

		bool in_range(false);
		if (Pow2) {
			in_range = compare_sum_pow2<true>(
				num, bound, denom_shift, limb_cnt
			) >= 0;
		} else {
			if (val.m & 1)
				in_range = compare_sum_pow2<false>(
					num, bound, denom_shift, limb_cnt
				) > 0;
			else
				in_range = compare_sum_pow2<false>(
					num, bound, denom_shift, limb_cnt
				) >= 0;
		}

		if (in_range)
			++exp10;
		else {
			multiply_10(num, limb_cnt);
			multiply_10(bound, limb_cnt);
		}

		m_len = 0;
		while (true) {
			limb_type digit(mod_pow2(num, limb_cnt, denom_shift));

			int bd_test(0);
			if (Pow2) {
				bd_test |= compare(
					num, bound, limb_cnt
				) <= 0 ? 1 : 0;
				bd_test |= compare_sum_pow2<true>(
					num, bound, denom_shift, limb_cnt
				) >= 0 ? 2 : 0;
			} else {
				if (val.m & 1) {
					bd_test |= compare(
						num, bound, limb_cnt
					) < 0 ? 1 : 0;
					bd_test |= compare_sum_pow2<false>(
						num, bound, denom_shift,
						limb_cnt
					) > 0 ? 2 : 0;
				} else {
					bd_test |= compare(
						num, bound, limb_cnt
					) <= 0 ? 1 : 0;
					bd_test |= compare_sum_pow2<false>(
						num, bound, denom_shift,
						limb_cnt
					) >= 0 ? 2 : 0;
				}
			}

			switch (bd_test) {
			case 0:
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				multiply_10(num, limb_cnt);
				multiply_10(bound, limb_cnt);
				break;
			case 1:
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			case 2:
				++digit;
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			case 3:
				bd_test = compare_sum_pow2<false>(
					num, num, denom_shift, limb_cnt
				);
				if ((bd_test > 0) || (!bd_test && (digit & 1)))
					++digit;

				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				exp_out = exp10 - m_len;
				return;
			}
		}
	}

	template <typename _Dummy, bool Oversize = false>
	struct assign_mantissa_t {
		static void apply(
			limb_type *val,
			typename value_traits::mantissa_type m
		) {
			val[0] = static_cast<limb_type>(m);
		}
	};

	template <typename _Dummy>
	struct assign_mantissa_t<_Dummy, true> {
		static void apply(
			limb_type *val,
			typename value_traits::mantissa_type m
		) {
			std::size_t pos(0);
			while (m) {
				val[pos] = static_cast<limb_type>(m);
				m >>= calc_traits::limb_bits;
				++pos;
			}
		}	
	};

	static void assign_mantissa(
		limb_type *val, std::size_t limb_cnt,
		typename value_traits::mantissa_type m
	) {
		assign_mantissa_t<
			void,
			(value_traits::mantissa_bits > calc_traits::limb_bits)
		>::apply(val, m);
	}

	static std::size_t limb_count_estimate(int32_t exp2)
	{
		auto bit_count = value_traits::mantissa_bits + exp2 + 6;
		if (bit_count % calc_traits::limb_bits)
			return (bit_count / calc_traits::limb_bits) + 1;
		else
			return bit_count / calc_traits::limb_bits;
	}

	static void shift_left(
		limb_type *val, std::size_t limb_cnt, int32_t shift
	)
	{
		shift_left_near(val, limb_cnt, shift % calc_traits::limb_bits);
		shift /= calc_traits::limb_bits;
		if (!shift)
			return;

		__builtin_memmove(
			val + shift, val,
			(limb_cnt - shift) * sizeof(limb_type)
		);
		__builtin_memset(val, 0, shift * sizeof(limb_type));
	}

	static void shift_left_near(
		limb_type *val, std::size_t limb_cnt, int32_t shift
	) {
		if (!shift)
			return;

		limb_type carry(0);
		limb_type tmp;
		for (std::size_t c(0); c < limb_cnt; ++c) {
			tmp = val[c];
			val[c] <<= shift;
			val[c] |= carry;
			carry = tmp >> (calc_traits::limb_bits - shift);
		}
	}

	static void assign_pow2(
		limb_type *val, std::size_t limb_cnt, int32_t exp2
	) {
		val[
			exp2 / calc_traits::limb_bits
		] |= limb_type(1) << (exp2 % calc_traits::limb_bits);
	}

	static void assign_pow5(
		limb_type *val, std::size_t limb_cnt, int32_t exp5
	)
	{
		multiply_pow5(
			val, limb_cnt,
			exp5 - calc_traits::assign_pow5(val, exp5)
		);
	}

	static void multiply_pow5(
		limb_type *val, std::size_t limb_cnt, uint32_t exp5
	)
	{
		for (
			; exp5 > calc_traits::base5_digits;
			exp5 -= calc_traits::base5_digits
		) {
			limb_type carry(0);
			for (std::size_t c(0); c < limb_cnt; ++c) {
				typename calc_traits::acc_type acc(val[c]);
				acc *= small_power_5[
					calc_traits::base5_digits
				];
				acc += carry;
				val[c] = acc;
				carry = acc >> calc_traits::limb_bits;
			}
		}

		if (exp5) {
			limb_type carry(0);
			for (std::size_t c(0); c < limb_cnt; ++c) {
				typename calc_traits::acc_type acc(val[c]);
				acc *= small_power_5[exp5];
				acc += carry;
				val[c] = acc;
				carry = acc >> calc_traits::limb_bits;
			}
		}
	}

	static void multiply_10(limb_type *val, std::size_t limb_cnt)
	{
		limb_type carry(0);
		for (std::size_t c(0); c < limb_cnt; ++c) {
			typename calc_traits::acc_type acc(val[c]);
			acc *= 10;
			acc += carry;
			val[c] = acc;
			carry = acc >> calc_traits::limb_bits;
		}
	}

	static limb_type mod_pow2(
		limb_type *val, std::size_t limb_cnt, int32_t exp2
	) {
		std::size_t pos(exp2 / calc_traits::limb_bits);
		int32_t shift(exp2 % calc_traits::limb_bits);
		limb_type rv(val[pos]);

		if (shift) {
			rv >>=shift;
			if ((pos + 1) < limb_cnt)
				rv |= val[pos + 1] << (
					calc_traits::limb_bits - shift
				);

			val[pos] &= (limb_type(1) << shift) - 1;
			++pos;
		}

		for (; pos < limb_cnt; ++pos)
			val[pos] = 0;

		return rv;
	}

	static std::size_t find_msb(limb_type *val, std::size_t limb_cnt)
	{
		auto pos(limb_cnt);
		do {
			--pos;
			if (val[pos])
				return (
					pos + 1
				) * calc_traits::limb_bits - support::clz(
					val[pos]
				) - 1;
		} while (pos);
		return 0;
	}

	static limb_type extract_limb(limb_type *val, std::size_t msb_pos) {
		auto msb_lpos(msb_pos / calc_traits::limb_bits);
		auto msb_bpos(msb_pos % calc_traits::limb_bits);

		if (msb_bpos == (calc_traits::limb_bits - 1))
			return val[msb_lpos];

		auto rv(val[msb_lpos] << (
			calc_traits::limb_bits - msb_bpos - 1
		));
		if (msb_lpos)
			rv |= val[msb_lpos - 1] >> (msb_bpos + 1);

		return rv;
	}

	static int compare(limb_type *l, limb_type *r, std::size_t limb_cnt)
	{
		auto pos(limb_cnt);
		do {
			--pos;

			if (r[pos] < l[pos])
				return 1;
			else if (l[pos] < r[pos])
				return -1;
		} while (pos);
		return 0;
	}

	template <bool Scaled>
	static int compare_sum(
		limb_type *l0, limb_type *l1, limb_type *r,
		std::size_t limb_cnt
	) {
		limb_type carry(0);
		limb_type tmp(0);
		int rv(0);
		for (std::size_t pos(0); pos < limb_cnt; ++pos) {
			if (Scaled)
				carry = __builtin_add_overflow(
					(l1[pos] << 1) | tmp, carry, &tmp
				) ? 1 : 0;
			else
				carry = __builtin_add_overflow(
					l1[pos], carry, &tmp
				) ? 1 : 0;

			carry |= __builtin_add_overflow(
				l0[pos], tmp, &tmp
			) ? 1 : 0;

			if (r[pos] < tmp)
				rv = 1;
			else if (tmp < r[pos])
				rv = -1;

			if (Scaled)
				tmp = l1[pos] >> (calc_traits::limb_bits - 1);
		}
		return rv;
	}

	template <bool Scaled>
	static int compare_sum_pow2(
		limb_type *l0, limb_type *l1, int32_t r_shift,
		std::size_t limb_cnt
	) {
		limb_type carry(0);
		limb_type tmp(0);
		int rv(0);
		for (std::size_t pos(0); pos < limb_cnt; ++pos) {
			if (Scaled)
				carry = __builtin_add_overflow(
					(l1[pos] << 1) | tmp, carry, &tmp
				) ? 1 : 0;
			else
				carry = __builtin_add_overflow(
					l1[pos], carry, &tmp
				) ? 1 : 0;

			carry |= __builtin_add_overflow(
				l0[pos], tmp, &tmp
			) ? 1 : 0;

			if (r_shift >= int32_t(calc_traits::limb_bits)) {
				if (tmp)
					rv = 1;
				r_shift -= calc_traits::limb_bits;
			} else if (r_shift >= 0) {
				auto r_val(limb_type(1) << r_shift);
				if (r_val < tmp)
					return 1;
				else if (r_val > tmp)
					rv = -1;

				r_shift = -1;
			} else {
				if (tmp)
					return 1;
			}

			if (Scaled)
				tmp = l1[pos] >> (calc_traits::limb_bits - 1);
		}
		return rv;
	}

	static limb_type divide_near(
		limb_type *l, limb_type *r, std::size_t limb_cnt,
		std::size_t r_msb_pos
	) {
		auto l_msb_pos(find_msb(l, limb_cnt));
		limb_type digit(0);
		if (l_msb_pos >= r_msb_pos) {
			digit = extract_limb(l, l_msb_pos)
				/ extract_limb(r, l_msb_pos);
			digit = subtract_mul(
				l, r, limb_cnt, digit
			);
		}
		return digit;
	}

	static limb_type subtract_mul(
		limb_type *l, limb_type *r, std::size_t limb_cnt,
		limb_type r_mul
	) {
		limb_type mul_carry(0);
		limb_type sub_carry(0);
		std::size_t pos;
		for (pos = 0; pos < limb_cnt; ++pos) {
			typename calc_traits::acc_type acc(r[pos]);
			acc *= r_mul;
			acc += mul_carry;
			sub_carry = __builtin_sub_overflow(
				l[pos], sub_carry, &l[pos]
			) ? 1 : 0;
			sub_carry |= __builtin_sub_overflow(
				l[pos], static_cast<limb_type>(acc), &l[pos]
			) ? 1 : 0;
			mul_carry = acc >> calc_traits::limb_bits;
		}

		return r_mul;
	}

	static void dump(
		char const *label, limb_type *val, std::size_t limb_cnt
	) {
		constexpr char const *format
		= calc_traits::limb_bits > 4 ? "%016lx'" : "%08x'";
		printf("%s: ", label);
		auto pos(limb_cnt);
		do {
			--pos;
			printf(format, val[pos]);
		} while (pos);
		printf("\n");
	}

public:
	static uint64_t c0;
	static uint64_t c1;
};

template <typename T>
uint64_t floating_point_to_bcd_bigint<T>::c0 = 0;
template <typename T>
uint64_t floating_point_to_bcd_bigint<T>::c1 = 0;

}}}
#endif

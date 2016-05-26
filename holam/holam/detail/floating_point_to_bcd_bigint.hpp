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
		limb_type *denom, *num, *bound_low, *bound_high;
		std::size_t limb_cnt;
		int extra_shift(val.is_normal_pow2() ? 1 : 0);
		auto exp10(power_10_estimate(val.exp));

		if (val.exp >= 0) {
			limb_cnt = limb_count_estimate(
				1 + extra_shift + val.exp
			);
			num = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_mantissa(num, limb_cnt, val.m);
			shift_left(num, limb_cnt, val.exp + 1 + extra_shift);

			denom = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_pow5(denom, limb_cnt, exp10);
			shift_left(denom, limb_cnt, exp10 + 1 + extra_shift);

			bound_low = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_pow2(bound_low, limb_cnt, val.exp);
		} else if (exp10 >= 0) {
			limb_cnt = limb_count_estimate(
				1 + extra_shift - val.exp
			);
			num = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_mantissa(num, limb_cnt, val.m);
			shift_left_near(num, limb_cnt, 1 + extra_shift);

			denom = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_pow5(denom, limb_cnt, exp10);
			shift_left(
				denom, limb_cnt,
				exp10 + 1 + extra_shift - val.exp
			);

			bound_low = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_pow2(bound_low, limb_cnt, 0);
		} else {
			limb_cnt = limb_count_estimate(
				1 + extra_shift - val.exp
			);

			num = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_mantissa(num, limb_cnt, val.m);

			denom = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);
			assign_pow2(
				denom, limb_cnt, 1 + extra_shift - val.exp
			);

			bound_low = reinterpret_cast<limb_type *>(
				__builtin_alloca(limb_cnt * sizeof(limb_type))
			);

			assign_pow5(bound_low, limb_cnt, -exp10);
			shift_left(bound_low, limb_cnt, -exp10);
			multiply_pow5(num, limb_cnt, -exp10);
			shift_left(num, limb_cnt, 1 + extra_shift - exp10);
		}

		bound_high = reinterpret_cast<limb_type *>(
			__builtin_alloca(limb_cnt * sizeof(limb_type))
		);
		__builtin_memcpy(
			bound_high, bound_low, limb_cnt * sizeof(limb_type)
		);
		if (extra_shift)
			shift_left_near(bound_high, limb_cnt, 1);

		bool in_range(false);
		if (val.m & 1)
			in_range = compare_sum(
				num, bound_high, denom, limb_cnt
			) > 0;
		else
			in_range = compare_sum(
				num, bound_high, denom, limb_cnt
			) >= 0;

		if (in_range)
			++exp10;
		else {
			multiply_10(num, limb_cnt);
			if (!__builtin_memcmp(
				bound_low, bound_high,
				limb_cnt * sizeof(limb_type)
			)) {
				multiply_10(bound_low, limb_cnt);
				__builtin_memcpy(
					bound_high, bound_low,
					limb_cnt * sizeof(limb_type)
				);
			} else {
				multiply_10(bound_low, limb_cnt);
				multiply_10(bound_high, limb_cnt);
			}
		}
#if 0
		dump("num:   ", num, limb_cnt);
		dump("denom: ", denom, limb_cnt);
		dump("high:  ", bound_high, limb_cnt);
		dump("low:   ", bound_low, limb_cnt);
#endif
		auto denom_msb(find_msb(denom, limb_cnt));
		m_len = 0;
		while (true) {
			auto num_msb(find_msb(num, limb_cnt));
			limb_type digit(0);
			if (num_msb >= denom_msb) {
				digit = extract_limb(num, num_msb)
					/ extract_limb(denom, num_msb);
				digit = subtract_mul(
					num, denom, limb_cnt, digit
				);
			}

			int bd_test(0);
			if (val.m & 1) {
				bd_test |= compare(
					num, bound_low, limb_cnt
				) < 0 ? 1 : 0;
				bd_test |= compare_sum(
					num, bound_high, denom, limb_cnt
				) > 0 ? 2 : 0;
			} else {
				bd_test |= compare(
					num, bound_low, limb_cnt
				) <= 0 ? 1 : 0;
				bd_test |= compare_sum(
					num, bound_high, denom, limb_cnt
				) >= 0 ? 2 : 0;
			}

			switch (bd_test) {
			case 0:
				m_out[m_len / 2] = m_len & 1
					? m_out[m_len / 2] | (digit << 4)
					: digit;
				++m_len;
				multiply_10(num, limb_cnt);
				multiply_10(bound_low, limb_cnt);
				multiply_10(bound_high, limb_cnt);
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
				bd_test = compare_sum(
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

	static int32_t power_10_estimate(int32_t exp2)
	{
		constexpr static double inv_log2_10 = 0.30102999566398114;
		return int32_t(lround(ceil((
			exp2
			+ int32_t(value_traits::mantissa_bits)
			- 1
		) * inv_log2_10 - 1e-10)));
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
		__builtin_memset(val, 0, limb_cnt * sizeof(limb_type));
		assign_mantissa_t<
			void,
			(value_traits::mantissa_bits > calc_traits::limb_bits)
		>::apply(val, m);
	}

	static std::size_t limb_count_estimate(int32_t exp2)
	{
		auto bit_count = value_traits::mantissa_bits + exp2 + 5;
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
		__builtin_memset(val, 0, limb_cnt * sizeof(limb_type));
		val[
			exp2 / calc_traits::limb_bits
		] |= limb_type(1) << (exp2 % calc_traits::limb_bits);
	}

	static void assign_pow5(
		limb_type *val, std::size_t limb_cnt, int32_t exp5
	)
	{
		__builtin_memset(val, 0, limb_cnt * sizeof(limb_type));
		multiply_pow5(
			val, limb_cnt,
			exp5 - calc_traits::assign_pow5(val, exp5)
		);
	}

	static void multiply_pow5(
		limb_type *val, std::size_t limb_cnt, int32_t exp5
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

	static int compare_sum(
		limb_type *l0, limb_type *l1, limb_type *r,
		std::size_t limb_cnt
	) {
		limb_type carry(0);
		limb_type tmp(0);
		int rv(0);
		for (std::size_t pos(0); pos < limb_cnt; ++pos) {
			carry = __builtin_add_overflow(
				l0[pos], carry, &tmp
			) ? 1 : 0;
			carry |= __builtin_add_overflow(
				tmp, l1[pos], &tmp
			) ? 1 : 0;
			if (r[pos] < tmp)
				rv = 1;
			else if (tmp < r[pos])
				rv = -1;
		}
		return rv;
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

		if (sub_carry) {
			sub_carry = 0;
			for (pos = 0; pos < limb_cnt; ++pos) {
				sub_carry = __builtin_add_overflow(
					l[pos], sub_carry, &l[pos]
				) ? 1 : 0;
				sub_carry |= __builtin_add_overflow(
					l[pos], r[pos], &l[pos]
				) ? 1 : 0;
			}
			return r_mul - 1;
		}

		if (0 <= compare(l, r, limb_cnt)) {
			sub_carry = 0;
			for (pos = 0; pos < limb_cnt; ++pos) {
				sub_carry = __builtin_sub_overflow(
					l[pos], sub_carry, &l[pos]
				) ? 1 : 0;
				sub_carry |= __builtin_sub_overflow(
					l[pos], r[pos], &l[pos]
				) ? 1 : 0;
			}
			return r_mul + 1;
		}

		return r_mul;
	}

	static void dump(
		char const *label, limb_type *val, std::size_t limb_cnt
	) {
		constexpr char const *format = calc_traits::limb_bits > 4 ? "%016lx'" : "%08x'";
		printf("%s: ", label);
		auto pos(limb_cnt);
		do {
			--pos;
			printf(format, val[pos]);
		} while (pos);
		printf("\n");
	}
};
}}}
#endif

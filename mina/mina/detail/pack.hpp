/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_DETAIL_PACK_NOV_07_2013_1700)
#define UCPF_MINA_DETAIL_PACK_NOV_07_2013_1700

#include <cstdint>
#include <array>
#include <yesod/is_sequence.hpp>
#include <yesod/compose_bool.hpp>
#include <yesod/float.hpp>

namespace ucpf { namespace mina {

template <typename OutputIterator, typename T>
struct custom {
	static void pack(OutputIterator &&sink, T &&v);
};

namespace detail {

constexpr uint8_t const small_int_mask         = 0x1f;
constexpr uint8_t const small_int_code_offset  = 0x10;
constexpr uint8_t const small_uint_code_offset = 0x30;

struct scalar_rank {
	enum {
		ie   = -1,
		i4   = 0,
		i8   = 1,
		i16  = 2,
		i32  = 3,
		i64  = 4,
		i128 = 5
	};

	constexpr static std::array<int, 6> order = {{
		5, 8, 16, 32, 64, 128
	}};

	template <typename T>
	constexpr static int from_type()
	{
		typedef typename std::remove_reference<T>::type Tr;
		typedef std::integral_constant<
			int, 32 - __builtin_clz(sizeof(Tr))
		> log_type;
		typedef std::integral_constant<
			int,
			((1UL << (log_type::value - 1)) < sizeof(Tr))
			? log_type::value + 1 : log_type::value
		> rv_type;

		return rv_type::value <= i128 ? rv_type::value : ie;
	}

	
	template <typename T>
	static int from_value(T v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		auto s_rank(from_type<Tr>());
		if (!std::is_integral<Tr>::value)
			return s_rank;

		while (s_rank > 0) {
			auto max_val(Tr(1) << order[s_rank - 1]);
			if (std::is_signed<Tr>::value) {
				max_val >>= 1;
				auto min_val = Tr(-1) - max_val;

				if ((v > min_val) && (v < max_val))
					s_rank--;
				else
					return s_rank;
			} else {
				if (v < max_val)
					s_rank--;
				else
					return s_rank;
			}
		}
		return s_rank;
	}
};

constexpr std::array<int, 6> scalar_rank::order;

struct list_size_rank {
	enum {
		le  = -1,
		l3  = 0,
		l8  = 1,
		l16 = 2,
		l24 = 3
	};

	static int from_size(size_t count)
	{
		if (count <= (1 << 24)) {
			if (count <= (1 << 16)) {
				if (count <= (1 << 8))
					return (count <= (1 << 3)) ? l3 : l8;
				else
					return l16;
			} else
				return l24;
		} else
			return le;
	}
};

struct numeric_type_rank {
	enum {
		n_signed = 0,
		n_unsigned = 1,
		n_float = 2
	};
};

constexpr std::array<
	std::array<std::array<uint8_t, 5>, 4>, 3
> list_code = {{
	{{
		{{ 0xe0, 0xc8, 0xb0, 0x98, 0x80 }},
		{{ 0x7d, 0x7a, 0x77, 0x74, 0x71 }},
		{{ 0x6d, 0x6a, 0x67, 0x64, 0x61 }},
		{{ 0x5d, 0x5a, 0x57, 0x54, 0x51 }}
	}},
	{{
		{{ 0xe8, 0xd0, 0xb8, 0xa0, 0x88 }},
		{{ 0x7e, 0x7b, 0x78, 0x75, 0x72 }},
		{{ 0x6e, 0x6b, 0x68, 0x65, 0x62 }},
		{{ 0x5e, 0x5b, 0x58, 0x55, 0x52 }}
	}},
	{{
		{{ 0xf0, 0xd8, 0xc0, 0xa8, 0x90 }},
		{{ 0x7f, 0x7c, 0x79, 0x76, 0x73 }},
		{{ 0x6f, 0x6c, 0x69, 0x66, 0x63 }},
		{{ 0x5f, 0x5c, 0x59, 0x56, 0x53 }}
	}}
}};

constexpr uint8_t tuple_start_code = 0x50;
constexpr uint8_t byte_skip_code = 0x60;
constexpr uint8_t tuple_end_code = 0x70;

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<N == 1, void>::type pack_integral(
	OutputIterator &&sink, T &&v
)
{
	*sink++ = static_cast<uint8_t>(v);
}

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<(N > 1), void>::type pack_integral(
	OutputIterator &&sink, T &&v
)
{
	*sink++ = static_cast<uint8_t>(v & 0xff);
	pack_integral<N - 1>(sink, v >> 8);
}

template <typename OutputIterator, typename T>
void pack_integral(OutputIterator &&sink, T &&v, unsigned int cnt)
{
	for(auto c(cnt); c > 0; --c)
		*sink++ = static_cast<uint8_t>(
			(v >> ((cnt - c) * 8)) & 0xff
		);
}

struct kind_flags {
	enum {
		integral = 1,
		float_t  = 2,
		sequence = 4
	};
};

template <typename T, int Kind, bool IsSequence = false>
struct classify {
	constexpr static int value = ucpf::yesod::compose_bool<
		int, Kind, yesod::is_floating_point<T>::value,
		std::is_integral<T>::value
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

template <typename T, int Kind>
struct classify<T, Kind, true> {
	constexpr static int value = classify<
		typename T::value_type, Kind | kind_flags::sequence, false
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

template <typename OutputIterator, typename T, int Kind>
struct pack_helper;

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, 0> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		custom<OutputIterator, T>::pack(
			std::forward<OutputIterator>(sink),
			std::forward<T>(v)
		);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::sequence> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		if (v.empty())
			return;

		*sink++ = detail::tuple_start_code;

		constexpr auto next_kind(detail::classify<
			typename T::value_type, 0,
			yesod::is_sequence<typename T::value_type>::value
		>::value);

		for (auto xv: v)
			pack_helper<OutputIterator, T, next_kind>(
				std::forward<OutputIterator>(sink),
				std::forward<T>(v)
			);

		*sink++ = detail::tuple_end_code;
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::integral> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		constexpr auto n_rank(
			std::is_signed<Tr>::value
			? numeric_type_rank::n_signed
			: numeric_type_rank::n_unsigned
		);
		auto s_rank(scalar_rank::from_value(v));
		if (s_rank == scalar_rank::ie)
			return;

		if (s_rank > scalar_rank::i4) {
			*sink++ = list_code[n_rank]
					   [list_size_rank::l3]
					   [s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				std::forward<T>(v),
				scalar_rank::order[s_rank] / 8
			);
		} else if (s_rank == scalar_rank::i4)
			*sink++ = (v & small_int_mask) + (
				n_rank == numeric_type_rank::n_signed
				? small_int_code_offset
				: small_uint_code_offset
			);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::float_t> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename yesod::fp_adapter_type<T>::type Tw;
		constexpr auto s_rank(
			scalar_rank::from_type<typename Tw::storage_type>()
		);
		*sink++ = list_code[numeric_type_rank::n_float]
				   [list_size_rank::l3]
				   [s_rank - 1];
		pack_integral<scalar_rank::order[s_rank] / 8>(
			std::forward<OutputIterator>(sink),
			Tw(v).get_storable()
		);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<
	OutputIterator, T, kind_flags::integral | kind_flags::sequence
> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		auto sz(v.size());

		if (!sz)
			return;

		constexpr auto n_rank(
			std::is_signed<typename T::value_type>::value
			? numeric_type_rank::n_signed
			: numeric_type_rank::n_unsigned
		);
		auto l_rank(list_size_rank::from_size(sz));
		constexpr auto s_rank(
			scalar_rank::from_type<typename T::value_type>()
		);

		if (s_rank == scalar_rank::ie)
			return;

		if (l_rank > list_size_rank::l3) {
			*sink++ = list_code[n_rank][l_rank][s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				sz - 1, l_rank
			);
		} else if (l_rank == list_size_rank::l3)
			*sink++ = list_code[n_rank][l_rank][s_rank - 1]
				  | uint8_t(sz - 1);
		else
			return;

		for (auto xv: v)
			pack_integral<scalar_rank::order[s_rank] / 8>(
				std::forward<OutputIterator>(sink), xv
			);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<
	OutputIterator, T, kind_flags::float_t | kind_flags::sequence
> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename yesod::fp_adapter_type<
			typename T::value_type
		>::type Tw;
		auto sz(v.size());

		if (!sz)
			return;

		auto l_rank(list_size_rank::from_size(sz));
		constexpr auto s_rank(
			scalar_rank::from_type<typename Tw::storage_type>()
		);

		if (l_rank > list_size_rank::l3) {
			*sink++ = list_code[numeric_type_rank::n_float]
					   [l_rank]
					   [s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				sz - 1, l_rank
			);
		} else if (l_rank == list_size_rank::l3)
			*sink++ = list_code[numeric_type_rank::n_float]
					   [l_rank]
					   [s_rank - 1]
				  | uint8_t(sz - 1);
		else
			return;

		for (auto xv: v)
			pack_integral<scalar_rank::order[s_rank] / 8>(
				std::forward<OutputIterator>(sink),
				Tw(xv).get_storable()
			);
	}
};

template <typename OutputIterator, typename T>
void pack(OutputIterator &&sink, T &&v)
{
	typedef typename std::remove_reference<T>::type Tr;
	pack_helper<
		OutputIterator, T, detail::classify<
			Tr, 0, yesod::is_sequence<Tr>::value
		>::value
	>(std::forward<OutputIterator>(sink), std::forward<T>(v));
}

template <typename OutputIterator, typename T, typename ...Tn>
void pack(OutputIterator &&sink, T &&v, Tn &&...vn)
{
	pack(std::forward<OutputIterator>(sink), std::forward<T>(v));
	pack(std::forward<OutputIterator>(sink), std::forward<Tn>(vn)...);
}

}
}}
#endif

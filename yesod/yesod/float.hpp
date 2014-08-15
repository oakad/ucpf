/*
 * Copyright (c) 2013-2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_FLOAT_NOV_15_2013_1215)
#define UCPF_YESOD_FLOAT_NOV_15_2013_1215

#include <utility>
#include <yesod/bitops.hpp>
#include <yesod/mpl/at.hpp>
#include <yesod/mpl/map.hpp>
#include <yesod/mpl/has_key.hpp>

#include <yesod/detail/float8.hpp>
#include <yesod/detail/float16.hpp>
#include <yesod/detail/float128.hpp>

namespace ucpf { namespace yesod {
namespace detail {

struct float_tag {};

template <
	typename StorageType, typename MachineType = StorageType,
	uint32_t MantissaBits = std::numeric_limits<MachineType>::digits,
	uint32_t ExponentBits = order_base_2(uint32_t(
		std::numeric_limits<MachineType>::max_exponent
		- std::numeric_limits<MachineType>::min_exponent
	))
> struct storable_float_traits {
	typedef StorageType storage_type;
	typedef MachineType machine_type;
	constexpr static bool is_native = std::is_floating_point<
		machine_type
	>::value;
	constexpr static uint32_t mantissa_bits = MantissaBits;
	constexpr static uint32_t exponent_bits = ExponentBits;
	constexpr static int32_t exponent_bias = ((1 << ExponentBits) - 2) / 2;
};

typedef typename mpl::map<
	mpl::pair<
		mpl::uint_<8>,
		storable_float_traits<uint8_t, float8>
	>,
	mpl::pair<
		mpl::uint_<16>,
		storable_float_traits<uint16_t, float16>
	>,
	mpl::pair<
		mpl::uint_<32>,
		storable_float_traits<uint32_t, float>
	>,
	mpl::pair<
		mpl::uint_<64>,
		storable_float_traits<uint64_t, double>
	>,
	mpl::pair<
		mpl::uint_<128>,
		storable_float_traits<uint128_t, yesod::float128>
	>
>::type fp_storage_types;

template <
	typename T, typename U, bool IsMachineFloatT = true,
	bool IsMachineFloatU = true
>
struct fp_util {
	static T convert(U u)
	{
		return static_cast<T>(u);
	}
};

}

template <size_t N>
struct float_t {
	typedef detail::float_tag tag;
	constexpr static size_t bit_size = N;

	typedef typename mpl::at<
		detail::fp_storage_types,
		mpl::uint_<N>
	>::type traits_type;
	typedef typename traits_type::storage_type storage_type;
	typedef typename traits_type::machine_type machine_type;

	float_t()
	: value(0) {}

	float_t(float_t const &other)
	: value(other.value) {}

	float_t(storage_type const &other)
	: storable(other) {}

	float_t(float_t &&other)
	: value(std::move(other.value)) {}

	template <typename U>
	float_t(U other)
	: value(detail::fp_util<
		machine_type, U, traits_type::is_native,
		std::is_floating_point<U>::value
	>::convert(other)) {};

	machine_type get() const
	{
		return value;
	}

	storage_type get_storable() const
	{
		return storable;
	}

	storage_type get_mantissa() const
	{
		constexpr static storage_type mantissa_mask((
			storage_type(1) << (traits_type::mantissa_bits - 1)
		) - 1);
		return mantissa_mask & storable;
	}

	storage_type get_exponent() const
	{
		return exponent_mask & (
			storable >> (traits_type::mantissa_bits - 1)
		);
	}

	int32_t get_exponent_value() const
	{
		return static_cast<int32_t>(uint32_t(
			get_exponent() ? get_exponent() : storage_type(1)
		)) - traits_type::exponent_bias;
	}

	bool get_sign() const
	{
		return (storable >> (
			traits_type::mantissa_bits + traits_type::exponent_bits
			- 1
		)) ? true : false;
	}

	bool is_special() const
	{
		return get_exponent() == exponent_mask;
	}

	std::pair<storage_type, bool> get_nan() const
	{
		constexpr static storage_type nan_value_mask((
			storage_type(1) << (traits_type::mantissa_bits - 2)
		) - 1);
		constexpr static storage_type nan_type_mask(
			storage_type(1) << (traits_type::mantissa_bits - 2)
		);

		return std::make_pair(
			nan_value_mask & storable,
			nan_type_mask & storable ? true : false
		);
	}

	static machine_type make_nan(storage_type v)
	{
		constexpr static storage_type nan_value_mask((
			storage_type(1) << (traits_type::mantissa_bits - 2)
		) - 1);

		if (!v)
			return std::numeric_limits<machine_type>::quiet_NaN();

		v &= nan_value_mask;
		v |= (
			(storage_type(1) << traits_type::exponent_bits) - 1
		) << (traits_type::mantissa_bits - 1);

		return v;
	}

private:
	constexpr static storage_type exponent_mask = (
		storage_type(1) << traits_type::exponent_bits
	) - 1;

	union {
		machine_type value;
		storage_type storable;
	};
};

template <size_t N>
constexpr typename float_t<N>::storage_type float_t<N>::exponent_mask;

namespace detail {

template <typename T, typename TagT, bool HasTagType = false>
struct has_tag : std::false_type {};

template <typename T, typename TagT>
struct has_tag<T, TagT, true> : std::integral_constant<
	bool, std::is_same<typename T::tag, TagT>::value
> {};

template <typename T>
struct is_floating_point : std::integral_constant<
	bool, std::is_floating_point<T>::value | has_tag<
		T, float_tag, mpl::detail::has_tag<T>::value
	>::value
> {};

typedef typename mpl::map<
	mpl::pair<float8, float_t<8>>,
	mpl::pair<float16, float_t<16>>,
	mpl::pair<float, float_t<32>>,
	mpl::pair<double, float_t<64>>,
	mpl::pair<long double, typename std::conditional<
		(std::numeric_limits<long double>::digits
		 > std::numeric_limits<double>::digits),
		float_t<128>, float_t<64>
	>>,
	mpl::pair<yesod::float128, float_t<128>>
>::type fp_adapter_types;

}

template <typename T>
using is_floating_point = typename detail::is_floating_point<
	typename std::remove_cv<T>::type
>::type;

template <typename T>
using fp_adapter_type = std::conditional<
	mpl::has_key<
		detail::fp_adapter_types, typename std::remove_cv<T>::type
	>::value, typename mpl::at<
		detail::fp_adapter_types, typename std::remove_cv<T>::type
	>::type, T
>;

}}
#endif

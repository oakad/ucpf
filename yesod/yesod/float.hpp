/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_FLOAT_NOV_15_2013_1215)
#define UCPF_YESOD_FLOAT_NOV_15_2013_1215

#include <type_traits>
#include <boost/operators.hpp>
#include <boost/mpl/x11/at.hpp>
#include <boost/mpl/x11/map.hpp>
#include <boost/mpl/x11/has_key.hpp>
#include <yesod/static_log2.hpp>

namespace ucpf { namespace yesod {
namespace detail {

struct float_tag {};

template <
	typename StorageType, typename MachineType = StorageType,
	unsigned int MantissaBits = std::numeric_limits<MachineType>::digits,
	unsigned int ExponentBits = static_log2<
		uint32_t,
		std::numeric_limits<MachineType>::max_exponent
		- std::numeric_limits<MachineType>::min_exponent
	>::value
> struct storable_float_traits {
	typedef StorageType storage_type;
	typedef MachineType machine_type;
	constexpr static bool is_native = std::is_floating_point<
		machine_type
	>::value;
	constexpr static unsigned int mantissa_bits = MantissaBits;
	constexpr static unsigned int exponent_bits = ExponentBits;
};

typedef typename boost::mpl::x11::map<
	boost::mpl::x11::pair<
		boost::mpl::x11::uint_<8>,
		storable_float_traits<uint8_t, uint8_t, 3, 4>
	>,
	boost::mpl::x11::pair<
		boost::mpl::x11::uint_<16>,
		storable_float_traits<uint16_t, uint16_t, 10, 5>
	>,
	boost::mpl::x11::pair<
		boost::mpl::x11::uint_<32>,
		storable_float_traits<uint32_t, float>
	>,
	boost::mpl::x11::pair<
		boost::mpl::x11::uint_<64>,
		storable_float_traits<uint64_t, double>
	>,
	boost::mpl::x11::pair<
		boost::mpl::x11::uint_<128>,
		storable_float_traits<unsigned __int128, __float128>
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

template <unsigned int N>
struct float_t {
	typedef detail::float_tag tag;

	typedef typename boost::mpl::x11::at<
		detail::fp_storage_types,
		boost::mpl::x11::uint_<N>
	>::type traits_type;
	typedef typename traits_type::storage_type storage_type;
	typedef typename traits_type::machine_type machine_type;

	float_t()
	: value(0) {}

	float_t(float_t const &other)
	: value(other.value) {}

	float_t(float_t &&other)
	: value(std::move(other.value)) {}

	template <typename U>
	float_t(U other)
	: value(detail::fp_util<
		machine_type, U, traits_type::is_native,
		std::is_floating_point<U>::value
	>::convert(other)) {};

	storage_type get_storable() const
	{
		return storable;
	}

private:
	union {
		machine_type value;
		storage_type storable;
	};
};

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
		T, float_tag, boost::mpl::x11::detail::has_tag<T>::value
	>::value
> {};

typedef typename boost::mpl::x11::map<
	boost::mpl::x11::pair<float, float_t<32>>,
	boost::mpl::x11::pair<double, float_t<64>>,
	boost::mpl::x11::pair<long double, typename std::conditional<
		(std::numeric_limits<long double>::digits
		 > std::numeric_limits<double>::digits),
		float_t<128>, float_t<64>
	>>,
	boost::mpl::x11::pair<__float128, float_t<128>>
>::type fp_adapter_types;

}

template <typename T>
using is_floating_point = typename detail::is_floating_point<
	typename std::remove_cv<T>::type
>::type;

template <typename T>
using fp_adapter_type = std::conditional<
	boost::mpl::x11::has_key<
		detail::fp_adapter_types, typename std::remove_cv<T>::type
	>::value, typename boost::mpl::x11::at<
		detail::fp_adapter_types, typename std::remove_cv<T>::type
	>::type, T
>;

}}
#endif
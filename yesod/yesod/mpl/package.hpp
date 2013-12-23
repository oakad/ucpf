/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_PACKAGE_DEC_12_2013_1230)
#define UCPF_YESOD_MPL_PACKAGE_DEC_12_2013_1230

#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/next_prior.hpp>
#include <yesod/mpl/sequence_fwd.hpp>
#include <yesod/mpl/iterator_tags.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct package_tag {};

}

template <typename... Tn>
struct package {
	typedef detail::package_tag tag;

	typedef package<Tn...> type;
	typedef type pkg_type;
};

template <typename T, T... Cn>
struct package_c : package<integral_constant<T, Cn>...> {
	typedef package_c type;
	typedef T value_type;
};

template <typename Sequence0, typename Sequence1>
struct join_pack;

template <typename... Tn, typename... Un>
struct join_pack<package<Tn...>, package<Un...>> {
	typedef package<Tn..., Un...> type;
};

template <typename Pack, template <typename... Un> class Op>
struct apply_pack;

template <typename... Tn, template <typename... Un> class Op>
struct apply_pack<package<Tn...>, Op> : Op<Tn...> {};

template <typename T>
struct at_c_always {
	template <typename Index, Index I>
	struct at_c {
		typedef T type;
	};
};

template <
	template <typename IndexOp, IndexOp I> class Op,
	typename Indices = package_c<long>,
	template <typename...> class Sequence = package
> struct at_c_indices;

template <
	typename Index, Index... In,
	template <typename IndexOp, IndexOp I>class Op,
	template <typename...> class Sequence
> struct at_c_indices<
	Op, package_c<Index, In...>, Sequence
> : Sequence<typename Op<Index, In>::type...> {};

namespace detail {

template <typename... Tn>
struct package_iterator {
	typedef forward_iterator_tag category;
};

template <>
struct at_impl<package_tag> {
	template <typename Package, long N>
	struct apply_impl;

	template <typename T0, typename... Tn>
	struct apply_impl<package<T0, Tn...>, 0> {
		typedef T0 type;
	};

	template <typename T0, typename... Tn, long N>
	struct apply_impl<package<T0, Tn...>, N>
	: apply_impl<package<Tn...>, N - 1> {};

	template <typename Package, typename N>
	struct apply : apply_impl<typename Package::pkg_type, N::value> {};
};

template <>
struct begin_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template<typename... Tn>
	struct apply_impl<package<Tn...>> {  
		typedef package_iterator<Tn...> type;
	};

	template <typename Package>
	struct apply : apply_impl<typename Package::pkg_type> {};
};

template <>
struct clear_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template <typename... Tn>
	struct apply_impl<package<Tn...>> {
		typedef package<> type;
	};

	template <typename Package>
	struct apply : apply_impl<Package> {};
};

template <>
struct end_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template <typename... Tn>
	struct apply_impl<package<Tn...>> {  
		typedef package_iterator<> type;
        };

	template <typename Package>
	struct apply : apply_impl<typename Package::pkg_type> {};
};

template <>
struct front_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template <typename T0, typename... Tn>
	struct apply_impl<package<T0, Tn...>> {  
		typedef T0 type;
	};

	template <typename Package>
	struct apply : apply_impl<typename Package::pkg_type> {};
};

template <>
struct pop_front_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template <
		typename... Tn, typename T0,
		template <typename...> class Package
	> struct apply_impl<Package<T0, Tn...>> {
		typedef Package<Tn...> type;
	};

	template <typename Package>
	struct apply : apply_impl<Package> {};
};

template <>
struct push_back_impl<package_tag> {
	template <typename Package, typename Tn>
	struct apply_impl;

	template <
		typename... Tn, typename T0,
		template <typename...> class Package
	> struct apply_impl<Package<Tn...>, T0> {  
		typedef Package<Tn..., T0> type;
	};

	template <typename Package, typename Tn>
	struct apply : apply_impl<Package, Tn> {};
};

template <>
struct push_front_impl<package_tag> {
	template <typename Package, typename T0>
	struct apply_impl;

	template <
		typename... Tn, typename T0,
		template <typename...> class Package
	> struct apply_impl<Package<Tn...>, T0> {  
		typedef Package<T0, Tn...> type;
	};

	template <typename Package, typename T0>
	struct apply : apply_impl<Package, T0> {};
};

template <>
struct size_impl<package_tag> {
	template <typename Package>
	struct apply_impl;

	template <typename... Args>
	struct apply_impl<package<Args...>> : long_<sizeof...(Args)> {};

	template <typename Package>
	struct apply : apply_impl<typename Package::pkg_type> {};
};

}

template <typename T0, typename... Tn>
struct deref<detail::package_iterator<T0, Tn...>> {
	typedef T0 type;
};

template <>
struct deref<detail::package_iterator<>> {
	typedef package<> type;
};

template <typename T0, typename... Tn>
struct next<detail::package_iterator<T0, Tn...>> {
	typedef detail::package_iterator<Tn...> type;
};

}}}

#endif

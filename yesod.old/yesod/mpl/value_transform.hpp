/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_48AD0D0D08D3A29A2CADE01F52132C3D)
#define HPP_48AD0D0D08D3A29A2CADE01F52132C3D

#include <yesod/mpl/value_cast.hpp>
#include <yesod/mpl/package_range_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <
	typename T0, T0 const &v0,
	template <typename T1, T1 const &v1> class Op
> struct value_transform;

template <
	typename U, std::size_t N, U const (&Seq)[N],
	template <typename T, T const &v> class Op
> struct value_transform<U const [N], Seq, Op> {
	typedef U const seq_type[N];

	typedef typename package_range_c<
		std::size_t, 0, N
	>::type index_type;

	typedef value_cast<
		typename apply_pack_c<
			typename package_range_c<std::size_t, 0, N>::type,
			Op<seq_type, Seq>::template apply
		>::type
	> value_type;
};

template <
	typename U, std::size_t N, const std::array<U, N> &Seq,
	template <typename T, T const &v> class Op
> struct value_transform<const std::array<U, N>, Seq, Op> {
	typedef const std::array<U, N> seq_type;

	typedef typename package_range_c<
		std::size_t, 0, N
	>::type index_type;

	typedef value_cast<
		typename apply_pack_c<
			typename package_range_c<std::size_t, 0, N>::type,
			Op<seq_type, Seq>::template apply
		>::type
	> value_type;
};

}}}
#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_INSERT_DEC_16_2013_1400)
#define UCPF_YESOD_MPL_DETAIL_INSERT_DEC_16_2013_1400

#include <yesod/mpl/copy.hpp>
#include <yesod/mpl/joint_view.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Tag>
struct insert_impl {
	template <typename Sequence, typename Pos, typename T = Pos>
	struct apply {
		typedef iterator_range<
			typename begin<Sequence>::type, Pos
		> first_half_;

		typedef iterator_range<
			Pos, typename end<Sequence>::type
		> second_half_;

		typedef typename reverse_fold<
			second_half_,
			typename clear<Sequence>::type,
			push_front<arg<-1>, arg<-1>>
		>::type half_sequence_;

		typedef typename reverse_fold<
			first_half_,
			typename push_front<half_sequence_, T>::type,
			push_front<arg<-1>, arg<-1>>
		>::type type;
	};
};

template <>
struct insert_impl<non_sequence_tag> {};

template <typename Tag>
struct insert_range_impl {
	template <typename Sequence, typename Pos, typename Range>
	struct apply : reverse_copy<
		joint_view<
			iterator_range<typename begin<Sequence>::type, Pos>,
			joint_view<
				Range, iterator_range<
					Pos, typename end<Sequence>::type
				>
			>
		>, front_inserter<typename clear<Sequence>::type>
	> {};
};

template <>
struct insert_range_impl<non_sequence_tag> {};

}}}}

#endif

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_ITERATOR_JOINED_RANGE_ADAPTOR_MAR_27_2014_1630)
#define UCPF_YESOD_ITERATOR_JOINED_RANGE_ADAPTOR_MAR_27_2014_1630

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

template <typename R0, typename... Rn>
struct choose_value_type {
	typedef decltype(std::begin(R0 {})) iterator;
	typedef typename std::iterator_traits<iterator>::value_type type;
};

template <typename R0, typename... Rn>
struct choose_iterator_category {
	typedef decltype(std::begin(R0 {})) iterator;
	typedef typename std::iterator_traits<
		iterator
	>::iterator_category type;
};

template <typename... Rn>
struct joined_range_iterator_base : facade<
	joined_range_iterator_base<typename choose_value_type<Rn...>::type>,
	typename choose_value_type<Rn...>::type,
	typename choose_iterator_category<Rn...>::type
> {
	friend struct core_access;

	template <size_t... Ind>
	joined_range_iterator_base(Range&&... r)
	: slices(std::make_tuple(slice<Range, Ind>(r)...))
	{
	}

	struct slice_base {
		virtual ~slice_base()
		{}
	};

	template <typename Range, size_t N>
	struct slice : slice_base {
		typedef decltype(std::begin(Range {})) iterator;

		slice(Range const &r)
		: first(std::begin(r)), last(std::end(r)), pos(first)
		{}

		iterator first;
		iterator last;
		iterator pos;
	};

	struct slice_pack_type {
		typedef typename mpl::package_range_c<
			size_t, 0, sizeof...(Rn)
		>::type indices;

		template <typename Ix, Ix... Cn>
		struct apply {
			typedef std::tuple<slice<Rn, Cn>...> type;
		};
	};

	joined_range const *container;
	typename mpl::apply_pack_c<
		slice_pack_type::indices, slice_pack_type::template apply
	>::type slices;
	std::array<slice_base *, sizeof...(Range)> s_access;
	size_t slice_pos;
};

}

template <typename... Range>
joined_range_iterator<Range...> make_joined_range(Range&&... r)
{
	return joined_range_iterator<Range...>(std::forward<Range>(r)...);
}

}}}

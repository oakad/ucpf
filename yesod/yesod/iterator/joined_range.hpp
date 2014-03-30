/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_ITERATOR_JOINED_RANGE_MAR_27_2014_1630)
#define UCPF_YESOD_ITERATOR_JOINED_RANGE_MAR_27_2014_1630

#include <functional>
#include <yesod/mpl/min_max.hpp>
#include <yesod/mpl/common_base.hpp>
#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename... Rn>
struct joined_range {
private:
	struct slice_base {
		virtual ~slice_base()
		{}
	};
	template <typename... Iterator>
	struct base_iterator_traits_ {
		typedef typename std::common_type<
			typename std::iterator_traits<Iterator>::value_type...
		>::type value_type;

		typedef typename mpl::common_base<
			typename std::iterator_traits<
				Iterator
			>::iterator_category...
		>::type iterator_category;

		typedef typename mpl::max<
			mpl::integral_constant<
				size_t, std::alignment_of<Iterator>::value
			>...
		>::type alignment;

		typedef typename mpl::max<
			mpl::integral_constant<size_t, sizeof(Iterator)>...
		>::type size;

		typedef typename std::aligned_storage<
			size::value, alignment::value
		>::type storage_type;
	};

	typedef base_iterator_traits_<
		decltype(std::begin(std::declval<Rn>()))...
	> base_iterator_traits;

public:
	typedef typename base_iterator_traits::value_type value_type;
	typedef typename base_iterator_traits::iterator_category
	iterator_category;

	template <typename ValueType>
	struct iterator_base : facade<
		iterator_base<ValueType>, ValueType,
		iterator_category
	> {

	private:
		friend struct core_access;
		friend struct joined_range;

		joined_range *r;
		size_t slice_pos;
		typename base_iterator_traits::storage_type pos;
	};

	typedef iterator_base<value_type> iterator;

	typedef typename std::conditional<
		std::is_const<value_type>::value,
		iterator_base<value_type>,
		iterator_base<typename std::add_const<value_type>::type>
	>::type const_iterator;

	joined_range(Rn&&... r)
	: slices(std::make_tuple(std::ref(r), this)...), slice_pos(0)
	{}

private:
	template <typename Range, size_t N>
	struct slice : slice_base {
		typedef decltype(
			std::begin(std::declval<Range>())
		) iterator;

		template <typename P>
		slice(P &&p)
		: first(std::begin(std::get<0>(p))),
		  last(std::end(std::get<0>(p)))
		{
			std::get<1>(p)->s_access[N]
			= dynamic_cast<slice_base *>(this);
		}

		iterator first;
		iterator last;
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

	typename mpl::apply_pack_c<
		typename slice_pack_type::indices,
		slice_pack_type::template apply
	>::type slices;
	std::array<slice_base *, sizeof...(Rn)> s_access;
	size_t slice_pos;
};

template <typename... Range>
joined_range<Range...> make_joined_range(Range&&... r)
{
	return joined_range<Range...>(std::forward<Range>(r)...);
}

}}}

#endif

/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2008 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_FOR_EACH_DEC_17_2013_1620)
#define UCPF_YESOD_MPL_FOR_EACH_DEC_17_2013_1620

#include <yesod/mpl/deref.hpp>
#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/lambda.hpp>

#include <functional>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Func>
constexpr Func &unwrap(Func &f, int)
{
	return f;
}

template <typename Func>
constexpr Func &&unwrap(Func &&f, int)
{
	return f;
}

template <typename Func>
constexpr Func &unwrap(std::reference_wrapper<Func> &f, int)
{
	return f;
}

template <typename Func>
constexpr Func &unwrap(std::reference_wrapper<Func> const &f, int)
{
	return f;
}

template <bool done = true>
struct for_each_impl {
	template <
		typename Iterator, typename LastIterator,
		typename TransformFunc, typename Func
	> static void execute(
		Iterator *, LastIterator *, TransformFunc *, Func &&
	) {}
};

template <>
struct for_each_impl<false> {
	template <
		typename Iterator, typename LastIterator,
		typename TransformFunc, typename Func
	> static void execute(
		Iterator *, LastIterator *, TransformFunc *, Func &&f
	)
	{
		typedef typename deref<Iterator>::type item;
		typedef typename apply<TransformFunc, item>::type arg;

		arg x{};
		unwrap(f, 0)(x);

		typedef typename next<Iterator>::type iter;
		for_each_impl<
			std::is_same<iter, LastIterator>::value
		>::execute(
			static_cast<iter *>(0),
			static_cast<LastIterator *>(0),
			static_cast<TransformFunc *>(0),
			std::forward<Func>(f)
		);
	}
};

}

template <typename Sequence, typename TransformOp, typename Func>
inline void for_each(Func &&f, Sequence * = 0, TransformOp * = 0)
{
	typedef typename begin<Sequence>::type first;
	typedef typename end<Sequence>::type last;

	detail::for_each_impl<std::is_same<first, last>::value>::execute(
		static_cast<first *>(0),
		static_cast<last *>(0),
		static_cast<TransformOp *>(0),
		std::forward<Func>(f)
	);
}

template <typename Sequence, typename Func>
inline void for_each(Func &&f, Sequence * = 0)
{
	mpl::for_each<Sequence, identity<>>(std::forward<Func>(f));
}

}}}

#endif

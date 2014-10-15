/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2002-2003 David Abrahams
    Copyright (c) 2002      Jeremy Siek
    Copyright (c) 2002      Thomas Witt
    Copyright (c) 2012      Jeffrey Lee Hellrung, Jr.

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_FACADE_20131219T1330)
#define UCPF_YESOD_ITERATOR_FACADE_20131219T1330

#include <yesod/iterator/categories.hpp>
#include <yesod/mpl/at.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/package_range_c.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

template <typename Traversal, typename ValueParam, typename Reference>
struct facade_default_category {
	template <typename T>
	struct has_traversal {
		template <typename U>
		using apply = std::is_convertible<
			T,
			typename mpl::second<U>::type
		>;
	};

	typedef typename mpl::deref<typename mpl::find_if<
		traversal_of_category, has_traversal<Traversal>
	>::type>::type x_category_type;

	typedef mpl::or_<
		mpl::and_<
			std::is_reference<Reference>,
			std::is_convertible<
				typename mpl::second<x_category_type>::type,
				forward_traversal_tag
			>
		>,
		mpl::and_<
			std::is_convertible<Reference, ValueParam>,
			std::is_convertible<
				typename mpl::second<x_category_type>::type,
				single_pass_traversal_tag
			>
		>
	> use_x_category;

	typedef typename mpl::if_<
		use_x_category,
		typename mpl::first<x_category_type>::type,
		Traversal
	>::type type;
};

template <typename Traversal, typename ValueParam, typename Reference>
struct facade_category_impl {
	static_assert(
		!is_category<Traversal>::value,
		"!is_category<Traversal>::value"
	);

	typedef typename facade_default_category<
		Traversal, ValueParam, Reference
	>::type category;

	typedef typename mpl::if_<
		std::is_same<
			Traversal,
			typename category_to_traversal<category>::type
		>,
		category,
		category_with_traversal<category, Traversal>
	>::type type;
};

template <
	typename CategoryOrTraversal, typename ValueParam, typename Reference
> using facade_category = mpl::eval_if<
	is_category<CategoryOrTraversal>,
	mpl::identity<CategoryOrTraversal>,
	facade_category_impl<
		CategoryOrTraversal, ValueParam, Reference
	>
>;

template <typename T0, typename T1>
using is_interoperable = std::integral_constant<
	bool,
	std::is_convertible<T0, T1>::value | std::is_convertible<T1, T0>::value
>;

template <typename T0, typename T1, typename Return>
using enable_if_interoperable = std::enable_if<
	is_interoperable<T0, T1>::value, Return
>;

template <
	typename ValueParam, typename CategoryOrTraversal, typename Reference,
	typename Difference
> struct facade_types {
	typedef typename facade_category<
		CategoryOrTraversal, ValueParam, Reference
	>::type iterator_category;

	typedef typename std::remove_const<ValueParam>::type value_type;

	typedef typename mpl::eval_if<
		writability_disabled<ValueParam, Reference>,
		std::add_pointer<const value_type>,
		std::add_pointer<value_type>
	>::type pointer;
};

template <typename Reference, typename Pointer>
struct operator_arrow_dispatch {
	struct proxy {
		explicit proxy(Reference const & x)
		: m_ref(x) {}

		Reference *operator->()
		{
			return std::addressof(m_ref);
		}

		operator Reference*()
		{
			return std::addressof(m_ref);
		}
		Reference m_ref;
	};

	typedef proxy result_type;

	static result_type apply(Reference const &x)
	{
		return result_type(x);
	}
};

template <typename T, typename Pointer>
struct operator_arrow_dispatch<T&, Pointer> {
	typedef Pointer result_type;
	static result_type apply(T &x)
	{
		return std::addressof(x);
	}
};

template <typename Iterator>
struct operator_brackets_proxy {
	typedef typename Iterator::reference reference;
	typedef typename Iterator::value_type value_type;

	operator_brackets_proxy(Iterator const &iter)
	: m_iter(iter)
	{}

	operator reference() const
	{
		return *m_iter;
	}

	operator_brackets_proxy& operator=(value_type const &val)
	{
		*m_iter = val;
		return *this;
	}

private:
	Iterator m_iter;
};

template <typename ValueType, typename Reference>
using use_operator_brackets_proxy = mpl::not_<
	mpl::and_<
		std::is_pod<ValueType>,
		writability_disabled<ValueType, Reference>
	>
>;

template <typename Iterator, typename Value, typename Reference>
struct operator_brackets_result {
	typedef typename mpl::if_<
		use_operator_brackets_proxy<Value, Reference>,
		operator_brackets_proxy<Iterator>,
		Value
	>::type type;
};

template <typename Iterator>
operator_brackets_proxy<Iterator> make_operator_brackets_result(
	Iterator const &iter, std::true_type
)
{
	return operator_brackets_proxy<Iterator>(iter);
}

template <typename Iterator>
typename Iterator::value_type make_operator_brackets_result(
	Iterator const &iter, std::false_type
)
{
	return *iter;
}

struct choose_difference_type {
	template <typename T0, typename T1>
	using apply = std::conditional<
		std::is_convertible<T1, T0>::value,
		typename std::iterator_traits<T0>::difference_type,
		typename std::iterator_traits<T1>::difference_type
	>;
};

template <typename Iterator>
struct postfix_increment_proxy {
private:
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	mutable value_type stored_value;

public:
	explicit postfix_increment_proxy(Iterator const &x)
	: stored_value(*x)
	{}

	value_type &operator*() const
	{
		return this->stored_value;
	}
};

template <typename Iterator>
struct writable_postfix_increment_proxy {
private:
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	mutable value_type stored_value;
	Iterator stored_iterator;

public:
	explicit writable_postfix_increment_proxy(Iterator const &x)
	: stored_value(*x), stored_iterator(x)
	{}

	writable_postfix_increment_proxy const &operator*() const
	{
		return *this;
	}

	operator value_type&() const
	{
		return stored_value;
	}

	template <typename T>
	T const &operator=(T const& x) const
	{
		*this->stored_iterator = x;
		return x;
	}

	template <typename T>
	T &operator=(T &x) const
	{
		*this->stored_iterator = x;
		return x;
	}

	operator Iterator const&() const
	{
		return stored_iterator;
	}
};

template <typename Reference, typename Value>
using is_non_proxy_reference = std::is_convertible<
	typename std::remove_reference<Reference>::type const volatile *,
	Value const volatile*
>;

template <
	typename Iterator, typename Value, typename Reference,
	typename CategoryOrTraversal
> using postfix_increment_result = mpl::eval_if<
	mpl::and_<
		std::is_convertible<Reference, Value const &>,
		mpl::not_<
			std::is_convertible<
				typename category_to_traversal<
					CategoryOrTraversal
				>::type, forward_traversal_tag
			>
		>
	>,
	mpl::if_<
		is_non_proxy_reference<Reference, Value>,
		postfix_increment_proxy<Iterator>,
		writable_postfix_increment_proxy<Iterator>
	>,
	mpl::identity<Iterator>
>;

}

template <typename... Args>
struct facade;

struct core_access {
	template <typename... Args>
	friend struct facade;

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator==(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator!=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator<(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator>(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator<=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	>::type operator>=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename detail::enable_if_interoperable<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		typename mpl::apply_wrap<
			detail::choose_difference_type,
			typename facade<Tn...>::derived_type,
			typename facade<Un...>::derived_type
		>::type
	>::type operator-(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn>
	friend typename facade<Tn...>::derived_type operator+(
		facade<Tn...>const &lhs,
		typename facade<
			Tn...
		>::derived_type::difference_type rhs
	);

	template <typename... Tn>
	friend typename facade<Tn...>::derived_type operator+(
		typename facade<
			Tn...
		>::derived_type::difference_type lhs,
		facade<Tn...> const &rhs
	);

	template <typename Facade>
	static typename Facade::reference dereference(Facade const &f)
	{
		return f.dereference();
	}

	template <typename Facade>
	static void increment(Facade &f)
	{
		f.increment();
	}

	template <typename Facade>
	static void decrement(Facade &f)
	{
		f.decrement();
	}

	template <typename Facade0, typename Facade1>
	static bool equal(Facade0 const &f0, Facade1 const &f1, std::true_type)
	{
		return f0.equal(f1);
	}

	template <typename Facade0, typename Facade1>
	static bool equal(Facade0 const &f0, Facade1 const &f1, std::false_type)
	{
		return f1.equal(f0);
	}

	template <typename Facade>
	static void advance(Facade &f, typename Facade::difference_type n)
	{
		f.advance(n);
	}

	template <typename Facade0, typename Facade1>
	static typename Facade0::difference_type distance_from(
		Facade0 const &f0, Facade1 const &f1, std::true_type
	)
	{
		return -f0.distance_to(f1);
	}

	template <typename Facade0, typename Facade1>
	static typename Facade1::difference_type distance_from(
		Facade0 const &f0, Facade1 const &f1, std::false_type
	)
	{
		return f1.distance_to(f0);
	}

	template <typename... Tn>
	static auto derived(
		facade<Tn...> &f
	) -> typename facade<Tn...>::derived_type &
	{
		return *static_cast<
			typename facade<Tn...>::derived_type *
		>(&f);
	}

	template <typename... Tn>
	static auto derived(
		facade<Tn...> &f
	) -> typename facade<Tn...>::derived_type const &
	{
		return *static_cast<
			typename facade<Tn...>::derived_type const *
		>(&f);
	}

private:
	core_access();
};

/* Optional arguments in order:
 * 1. Reference type
 * 2. Difference type
 */
template <
	typename Derived, typename ValueType, typename CategoryOrTraversal,
	typename... Args
>
struct facade<Derived, ValueType, CategoryOrTraversal, Args...> {
	typedef Derived derived_type;
	typedef ValueType x_value_type;

	derived_type &derived()
	{
		return *static_cast<derived_type *>(this);
	}

	derived_type const &derived() const
	{
		return *static_cast<derived_type const *>(this);
	}

	typedef mpl::package<x_value_type &, std::ptrdiff_t> default_args_pack;

	static_assert(
		sizeof...(Args) <= mpl::size<default_args_pack>::type::value,
		"too many optional parameters"
	);

	typedef typename mpl::at_c_indices<
		mpl::at_c_value<default_args_pack>::template at_c,
		typename mpl::package_range_c<
			long, sizeof...(Args),
			mpl::size<default_args_pack>::type::value
		>::type
        >::type x_default_args_pack;

	typedef typename mpl::join_pack<
		mpl::package<Args...>, x_default_args_pack
	>::type optional_args_pack;

	typedef typename mpl::at_c<optional_args_pack, 0>::type reference;
	typedef typename mpl::at_c<
		optional_args_pack, 1
	>::type difference_type;

private:
	typedef CategoryOrTraversal x_category_type;

	typedef detail::facade_types<
		x_value_type, x_category_type, reference, difference_type
	> associated_types;

	typedef detail::operator_arrow_dispatch<
		reference, typename associated_types::pointer
	> operator_arrow_dispatch_;

public:
	typedef typename associated_types::value_type value_type;
	typedef typename operator_arrow_dispatch_::result_type pointer;
	typedef typename associated_types::iterator_category iterator_category;

	reference operator*() const
	{
		return core_access::dereference(
			this->derived()
		);
	}

	pointer operator->() const
	{
		return operator_arrow_dispatch_::apply(*this->derived());
	}

	typename detail::operator_brackets_result<
		derived_type, x_value_type, reference
	>::type operator[](difference_type n) const
	{
		typedef detail::use_operator_brackets_proxy<
			x_value_type, reference
		> use_proxy;

		return detail::make_operator_brackets_result<derived_type>(
			this->derived() + n, use_proxy()
		);
	}

	derived_type &operator++()
	{
		core_access::increment(this->derived());
		return this->derived();
	}

	typename detail::postfix_increment_result<
		derived_type, x_value_type, reference, x_category_type
	>::type operator++(int)
	{
		typename detail::postfix_increment_result<
			derived_type, x_value_type, reference, x_category_type
		>::type tmp(*static_cast<derived_type *>(this));

		core_access::increment(this->derived());

		return tmp;
	}

	derived_type &operator--()
	{
		core_access::decrement(this->derived());
		return this->derived();
	}

	derived_type operator--(int)
	{
		derived_type tmp(this->derived());
		--*this;
		return tmp;
	}

	derived_type &operator+=(difference_type n)
	{
		core_access::advance(this->derived(), n);
		return this->derived();
	}

	derived_type &operator-=(difference_type n)
	{
		core_access::advance(this->derived(), -n);
		return this->derived();
	}

	derived_type operator-(difference_type x) const
	{
		derived_type result(this->derived());
		return result -= x;
	}
};

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator==(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator!=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return !core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator<(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 > core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator>(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 < core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator<=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 >= core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
>::type operator>=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 <= core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	typename mpl::apply_wrap<
		detail::choose_difference_type,
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type
	>::type
>::type operator-(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(
		detail::is_interoperable<lh_derived, rh_derived>::value,
		"detail::is_interoperable<lh_derived, rh_derived>::value"
	);
	
	return core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn>
auto operator+(
	facade<Tn...> const &lhs,
	typename facade<Tn...>::derived_type::difference_type rhs
) -> typename facade<Tn...>::derived_type
{
	typedef typename facade<Tn...>::derived_type lh_derived;

	lh_derived tmp(static_cast<lh_derived const &>(lhs));
	return tmp += rhs; 
}

template <typename... Tn>
auto operator+(
	typename facade<Tn...>::derived_type::difference_type lhs,
	facade<Tn...> const &rhs
) ->  typename facade<Tn...>::derived_type
{
	typedef typename facade<Tn...>::derived_type rh_derived;

	rh_derived tmp(static_cast<rh_derived const &>(rhs));
	return tmp += lhs; 
}

}}}
#endif

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

#if !defined(UCPF_YESOD_ITERATOR_FACADE_DEC_19_2013_1330)
#defined UCPF_YESOD_ITERATOR_FACADE_DEC_19_2013_1330

#include <yesod/mpl/logical.hpp>

namespace ucpf { namespace yesod {

struct no_traversal_tag {};

struct incrementable_traversal_tag : no_traversal_tag {};

struct single_pass_traversal_tag : incrementable_traversal_tag {};

struct forward_traversal_tag : single_pass_traversal_tag {};

struct bidirectional_traversal_tag : forward_traversal_tag {};

struct random_access_traversal_tag : bidirectional_traversal_tag {};

namespace detail {

typedef mpl::list<
	mpl::pair<std::random_access_iterator_tag, random_access_traversal_tag>,
	mpl::pair<std::bidirectional_iterator_tag, bidirectional_traversal_tag>,
	mpl::pair<std::forward_iterator_tag, forward_traversal_tag>,
	mpl::pair<std::input_iterator_tag, single_pass_traversal_tag>,
	mpl::pair<std::output_iterator_tag, incrementable_traversal_tag>
> traversal_of_category;

template <typename T>
struct is_reference_to_const : std::false_type
{};

template <typename T>
struct is_reference_to_const<T const &> : std::true_type
{};

template <typename ValueParam, typename Reference>
using iterator_writability_disabled = mpl::or_<
	std::is_const<Reference>,
	is_reference_to_const<Reference>,
	std::is_const<ValueParam>
>;

template <typename T>
using is_iterator_category = mpl::or_<
	std::is_convertible<T, std::input_iterator_tag>,
	std::is_convertible<T, std::output_iterator_tag>
>;

template <typename T>
using is_iterator_traversal = std::is_convertible<
	T, incrementable_traversal_tag
>;

template <typename Traversal, typename ValueParam, typename Reference>
struct iterator_facade_default_category {
	template <typename T>
	struct has_traversal {
		template <typename U>
		using apply = std::is_convertible<
			T,
			typename mpl::second<U>::type
		>;
	};

	typedef typename mpl::find_if<
		traversal_of_category, has_traversal<Traversal>
	>::type x_category_type;

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

template <typename Category>
struct iterator_category_to_traversal {
	template <typename T>
	struct has_category {
		template <typename U>
		using apply = std::is_convertible<
			T,
			typename mpl::first<U>::type
		>;
	};

	typedef typename mpl::if_<
		std::is_convertible<Category, incrementable_traversal_tag>,
		Category,
		typename mpl::second<typename mpl::find_if<
			traversal_of_category, has_category<Category>
		>::type>::type
	>::type;
};

template <typename Category, typename Traversal>
struct iterator_category_with_traversal : Category, Traversal {
	static_assert(
		std::is_convertible<
			typename iterator_category_to_traversal<Category>::type,
			Traversal
		>::value,
		"iterator category and traversal types do not match"
	);
	static_assert(
		is_iterator_category<Category>::value,
	       "is_iterator_category<Category>::value"
	);
	static_assert(
		!is_iterator_category<Traversal>::value,
		"!is_iterator_category<Traversal>::value"
	);
	static_assert(
		!is_iterator_traversal<Category>::value,
		"!is_iterator_traversal<Category>::value"
	);
	static_assert(
		is_iterator_traversal<Traversal>::value,
		"is_iterator_traversal<Traversal>::value"
	);
};

template <typename Traversal, typename ValueParam, typename Reference>
struct facade_iterator_category_impl {
	static_assert(
		!is_iterator_category<Traversal>::value,
		"!is_iterator_category<Traversal>::value"
	);

	typedef typename iterator_facade_default_category<
		Traversal, ValueParam, Reference
	>::type category;

	typedef typename mpl::if_<
		std::is_same<
			Traversal,
			typename iterator_category_to_traversal<category>::type
		>,
		category,
		iterator_category_with_traversal<category, Traversal>
	>::type type;
};

template <
	typename CategoryOrTraversal, typename ValueParam, typename Reference
> using facade_iterator_category = mpl::eval_if<
	is_iterator_category<CategoryOrTraversal>,
	mpl::identity<CategoryOrTraversal>,
	facade_iterator_category_impl<
		CategoryOrTraversal, ValueParam, Reference
	>
>;

template <typename T0, typename T1, typename Return>
using enable_if_interoperable = std::enable_if<
	mpl::or_<
		std::is_convertible<T0, T1>,
		std::is_convertible<T1, T0>
	>, Return
>;

template <
	typename ValueParam, typename CategoryOrTraversal, typename Reference,
	typename Difference
> struct iterator_facade_types {
	typedef typename facade_iterator_category<
		CategoryOrTraversal, ValueParam, Reference
	>::type iterator_category;

	typedef typename std::remove_const<ValueParam>::type value_type;

	typedef typename mpl::eval_if<
		iterator_writability_disabled<ValueParam, Reference>,
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
using use_operator_brackets_proxy : mpl::not_<
	mpl::and_<
		std::is_pod<ValueType>,
		iterator_writability_disabled<ValueType, Reference>
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
	using apply = mpl::eval_if<
		std::is_convertible<T1, T0>,
		typename std::iterator_traits<T0>::difference_type type,
		typename std::iterator_traits<T1>::difference_type type
	>;
};

}

struct iterator_core_access {
	template <typename... Tn>
	friend struct iterator_facade;

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator==(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator!=(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator<(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator>(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator<=(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		bool
	>::type operator>=(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend typename enable_if_interoperable<
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type,
		typename mpl::apply<
			choose_difference_type,
			typename iterator_facade<Tn...>::derived_type,
			typename iterator_facade<Un...>::derived_type
		>::type
	>::type operator-(
		iterator_facade<Tn...> const &lhs,
		iterator_facade<Un...> const &rhs
	);

	template <typename... Tn>
	friend typename iterator_facade<Tn...>::derived_type operator+(
		iterator_facade<Tn...>const &lhs,
		typename iterator_facade<
			Tn...
		>::derived_type::difference_type rhs
	);

	template <typename... Tn>
	friend typename iterator_facade<Tn...>::derived_type operator+(
		typename iterator_facade<
			Tn...
		>::derived_type::difference_type lhs,
		iterator_facade<Tn...> const &rhs
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
		iterator_facade<Tn...> &f
	) -> typename iterator_facade<Tn...>::derived_type &
	{
		return *static_cast<
			typename iterator_facade<Tn...>::derived_type *
		>(&f);
	}

	template <typename... Tn>
	static auto derived(
		iterator_facade<Tn...> &f
	) -> typename iterator_facade<Tn...>::derived_type const &
	{
		return *static_cast<
			typename iterator_facade<Tn...>::derived_type const *
		>(&f);
	}

private:
	iterator_core_access();
};

/* Expected arguments in order:
 * 1. Derived type for CRTP
 * 2. Value type
 * 3. Iterator category or traversal type
 * 4. Reference typede
 * 5. Difference type
 */
template <typename... Tn>
struct iterator_facade {
	static_assert(
		sizeof...(Tn) > 2,
		"At least the derived type for CRTP, value type and category "
		"must be specified."
	);

private:
	typedef typename mpl::apply_wrap<mpl::arg<0>, Tn...>::type derived_type;
	typedef typename mpl::apply_wrap<mpl::arg<1>, Tn...>::type x_value_type;
	typedef typename mpl::apply_wrap<
		mpl::arg<2>, Tn...
	>::type x_category_type;

	derived_type &derived()
	{
		return *static_cast<derived_type *>(this);
	}

	derived_type const &derived() const
	{
		return *static_cast<derived_type const *>(this);
	}

public:
	typedef typename std::conditional<
		(sizeof...(Tn) > 3),
		typename mpl::apply_wrap<mpl::arg<3>, Tn...>::type,
		x_value_type &
	>::type reference;

	typedef typename std::conditional<
		(sizeof...(Tn) > 4),
		typename mpl::apply_wrap<mpl::arg<4>, Tn...>::type,
		std::ptrdiff_t
	>::type difference_type;

private:

	typedef detail::iterator_facade_types<
		x_value_type, x_category_type, reference_type, difference_type
	> associated_types;

	typedef detail::operator_arrow_dispatch<
		reference_type, typename associated_types::pointer
	> operator_arrow_dispatch_;

public:
	typedef typename associated_types::value_type value_type;
	typedef typename operator_arrow_dispatch_::result_type pointer;
	typedef typename associated_types::iterator_category iterator_category;

	reference operator*() const
	{
		return iterator_core_access::dereference(
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

	Derived &operator++()
	{
		iterator_core_access::increment(this->derived());
		return this->derived();
	}

	Derived &operator--()
	{
		iterator_core_access::decrement(this->derived());
		return this->derived();
	}

	Derived operator--(int)
	{
		Derived tmp(this->derived());
		--*this;
		return tmp;
	}

	Derived &operator+=(difference_type n)
	{
		iterator_core_access::advance(this->derived(), n);
		return this->derived();
	}

	Derived &operator-=(difference_type n)
	{
		iterator_core_access::advance(this->derived(), -n);
		return this->derived();
	}

	Derived operator-(difference_type x) const
	{
		Derived result(this->derived());
		return result -= x;
	}
};

namespace detail {

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
				typename iterator_category_to_traversal<
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

template <typename... Tn>
auto operator++(
	iterator_facade<Tn...> &f, int
) -> typename detail::postfix_increment_result<
	typename iter_type::derived_type,
	typename iter_type::x_value_type,
	typename iter_type::reference,
	typename iter_type::x_category_type
>::type
{
	typedef decltype(f) iter_type;

	typename detail::postfix_increment_result<
		typename iter_type::derived_type,
		typename iter_type::x_value_type,
		typename iter_type::reference,
		typename iter_type::x_category_type
	>::type tmp(*static_cast<typename iter_type::derived_type *>(&f));

	++i;

	return tmp;
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator==(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return iterator_core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator!=(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return !iterator_core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator<(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 > iterator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator>(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 < iterator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator<=(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 >= iterator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	bool
>::type operator>=(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);

	return 0 <= iterator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
typename detail::enable_if_interoperable<
	typename iterator_facade<Tn...>::derived_type,
	typename iterator_facade<Un...>::derived_type,
	typename mpl::apply<
		detail::choose_difference_type,
		typename iterator_facade<Tn...>::derived_type,
		typename iterator_facade<Un...>::derived_type
	>::type
>::type operator-(
	iterator_facade<Tn...> const &lhs,
	iterator_facade<Un...> const &rhs
)
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	typedef typename decltype(rhs)::derived_type rh_derived;

	static_assert(
		std::is_interoperable<lh_derived, rh_derived>::value,
		"std::is_interoperable<lh_derived, rh_derived>::value"
	);
	
	return iterator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn>
auto operator+(
	iterator_facade<Tn...> const &lhs,
	typename iterator_facade<Tn...>::derived_type::difference_type rhs
) -> lh_derived
{
	typedef typename decltype(lhs)::derived_type lh_derived;
	lh_derived tmp(static_cast<lh_derived const &>(lhs));
	return tmp += rhs; 
}

template <typename... Tn>
auto operator+(
	typename iterator_facade<Tn...>::derived_type::difference_type lhs,
	iterator_facade<Tn...> const &rhs
) -> rh_derived
{
	typedef typename decltype(rhs)::derived_type rh_derived;
	rh_derived tmp(static_cast<rh_derived const &>(rhs));
	return tmp += lhs; 
}

}}
#endif

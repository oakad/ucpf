/*
 * Copyright (c) 2013-2017 Alex Dubov <oakad@yahoo.com>
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

#if !defined(HPP_8E77828863F45B23BE2CDAC39DB63FF8)
#define HPP_8E77828863F45B23BE2CDAC39DB63FF8

#include <iterator>
#include <yesod/iterator/is_interoperable.hpp>

namespace ucpf::yesod::iterator {
namespace detail {

template <typename T>
struct is_reference_to_const : std::false_type {};

template <typename T>
struct is_reference_to_const<T const &> : std::true_type {};

template <typename ValueType, typename Reference>
using writability_disabled = std::integral_constant<
	bool,
	std::is_const<Reference>::value
	| is_reference_to_const<Reference>::value
	| std::is_const<ValueType>::value
>;

template <typename ValueType, typename Category, typename... Args>
struct facade_types;

template <
	typename ValueType, typename Category, typename Reference,
	typename Difference
> struct facade_types<ValueType, Category, Reference, Difference> {
	typedef typename std::remove_const<ValueType>::type value_type;
	typedef Category iterator_category;
	typedef Reference reference;
	typedef std::conditional_t<
		writability_disabled<ValueType, Reference>::value,
		std::add_pointer_t<const value_type>,
		std::add_pointer_t<value_type>
	> pointer;
	typedef Difference difference_type;
};

template <
	typename ValueType, typename Category, typename Reference
> struct facade_types<
	ValueType, Category, Reference
> : facade_types<ValueType, Category, Reference, ptrdiff_t> {};

template <
	typename ValueType, typename Category
> struct facade_types<
	ValueType, Category
> : facade_types<ValueType, Category, ValueType &, ptrdiff_t> {};

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
using use_operator_brackets_proxy = std::integral_constant<bool, !(
		std::is_pod<ValueType>::value
		&& writability_disabled<ValueType, Reference>::value
)>;


template <typename Iterator, typename ValueType, typename Reference>
using operator_brackets_result = std::conditional_t<
	use_operator_brackets_proxy<ValueType, Reference>::value,
	operator_brackets_proxy<Iterator>,
	ValueType
>;

template <typename Iterator>
auto make_operator_brackets_result(Iterator const &iter, std::true_type)
{
	return operator_brackets_proxy<Iterator>(iter);
}

template <typename Iterator>
auto make_operator_brackets_result(Iterator const &iter, std::false_type)
{
	return *iter;
}

template <typename Iterator>
struct postfix_increment_proxy {
	typedef typename std::iterator_traits<Iterator>::value_type value_type;

	explicit postfix_increment_proxy(Iterator const &x)
	: stored_value(*x)
	{}

	value_type &operator*() const
	{
		return this->stored_value;
	}

private:
	mutable value_type stored_value;
};

template <typename Iterator>
struct writable_postfix_increment_proxy {
	typedef typename std::iterator_traits<Iterator>::value_type value_type;

	explicit writable_postfix_increment_proxy(Iterator const &x)
	: stored_value(*x), stored_iterator(x)
	{}

	auto const &operator*() const
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

private:
	mutable value_type stored_value;
	Iterator stored_iterator;
};

template <typename Reference, typename ValueType>
using is_non_proxy_reference = std::is_convertible<
	typename std::remove_reference<Reference>::type const volatile *,
	ValueType const volatile*
>;

template <
	typename Iterator, typename ValueType, typename Reference,
	typename Category
> using postfix_increment_result = std::conditional_t<
	std::is_convertible<Reference, ValueType const &>::value
	&& !std::is_convertible<Category, std::forward_iterator_tag>::value,
	std::conditional_t<
		is_non_proxy_reference<Reference, ValueType>::value,
		postfix_increment_proxy<Iterator>,
		writable_postfix_increment_proxy<Iterator>
	>,
	Iterator
>;

}

template <typename... Args>
struct facade;

struct core_access {
	template <typename... Args>
	friend struct facade;

	template <typename T0, typename T1>
	using choose_difference_type = std::conditional_t<
		std::is_convertible<T1, T0>::value,
		typename std::iterator_traits<T0>::difference_type,
		typename std::iterator_traits<T1>::difference_type
	>;

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator==(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator!=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator<(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator>(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator<=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		bool
	> operator>=(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type,
		choose_difference_type<
			typename facade<Tn...>::derived_type,
			typename facade<Un...>::derived_type
		>
	> operator-(
		facade<Tn...> const &lhs,
		facade<Un...> const &rhs
	);

	template <typename... Tn>
	friend auto operator+(
		facade<Tn...>const &lhs,
		typename facade<Tn...>::derived_type::difference_type rhs
	);

	template <typename... Tn>
	friend auto operator+(
		typename facade<Tn...>::derived_type::difference_type lhs,
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
	static auto distance_from(
		Facade0 const &f0, Facade1 const &f1, std::true_type
	)
	{
		return -f0.distance_to(f1);
	}

	template <typename Facade0, typename Facade1>
	static auto distance_from(
		Facade0 const &f0, Facade1 const &f1, std::false_type
	)
	{
		return f1.distance_to(f0);
	}

	template <typename... Tn>
	static auto &derived(facade<Tn...> &f)
	{
		return *static_cast<
			typename facade<Tn...>::derived_type *
		>(&f);
	}

	template <typename... Tn>
	static auto const &derived(facade<Tn...> &f)
	{
		return *static_cast<
			typename facade<Tn...>::derived_type const *
		>(&f);
	}

	core_access() = delete;
};

/* Optional arguments in order:
 * 1. Reference type
 * 2. Difference type
 */
template <
	typename Derived, typename ValueType, typename Category,
	typename... Args
>
struct facade<Derived, ValueType, Category, Args...> {
private:
	typedef detail::facade_types<
		ValueType, Category, Args...
	> associated_types;

	typedef detail::operator_arrow_dispatch<
		typename associated_types::reference,
		typename associated_types::pointer
	> operator_arrow_dispatch_;

public:
	typedef Derived derived_type;
	typedef ValueType qualified_value_type;
	typedef typename associated_types::value_type value_type;
	typedef typename associated_types::iterator_category iterator_category;
	typedef typename associated_types::reference reference;
	typedef typename operator_arrow_dispatch_::result_type pointer;
	typedef typename associated_types::difference_type difference_type;

	derived_type &derived()
	{
		return *static_cast<derived_type *>(this);
	}

	derived_type const &derived() const
	{
		return *static_cast<derived_type const *>(this);
	}

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

	auto operator[](difference_type n) const
	{
		typedef detail::use_operator_brackets_proxy<
			qualified_value_type, reference
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

	auto operator++(int)
	{
		detail::postfix_increment_result<
			derived_type, qualified_value_type, reference,
			iterator_category
		> tmp(*static_cast<derived_type *>(this));

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
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator==(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator!=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return !core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator<(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 > core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator>(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 < core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator<=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 >= core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	bool
> operator>=(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 <= core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename facade<Tn...>::derived_type,
	typename facade<Un...>::derived_type,
	core_access::choose_difference_type<
		typename facade<Tn...>::derived_type,
		typename facade<Un...>::derived_type
	>
> operator-(
	facade<Tn...> const &lhs,
	facade<Un...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;
	typedef typename facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);
	
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
)
{
	typedef typename facade<Tn...>::derived_type lh_derived;

	lh_derived tmp(static_cast<lh_derived const &>(lhs));
	return tmp += rhs; 
}

template <typename... Tn>
auto operator+(
	typename facade<Tn...>::derived_type::difference_type lhs,
	facade<Tn...> const &rhs
)
{
	typedef typename facade<Tn...>::derived_type rh_derived;

	rh_derived tmp(static_cast<rh_derived const &>(rhs));
	return tmp += lhs; 
}

}
#endif

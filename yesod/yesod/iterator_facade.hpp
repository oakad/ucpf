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

template <typename T>
struct is_reference_to_const : std::false_type
{};

template <typename T>
struct is_reference_to_const<T const &> : std::true_type
{};

template <typename ValueParam, typename Reference>
using iterator_writability_disabled = mpl::or_<
	std::is_const<Reference>, is_reference_to_const<Reference>,
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
using iterator_facade_default_category = mpl::eval_if<
	mpl::and_<
		std::is_reference<Reference>,
		std::is_convertible<Traversal, forward_traversal_tag>
	>,
	mpl::eval_if<
		std::is_convertible<Traversal, random_access_traversal_tag>,
		mpl::identity<std::random_access_iterator_tag>,
		mpl::if_<
			std::is_convertible<
				Traversal, bidirectional_traversal_tag
			>,
			std::bidirectional_iterator_tag,
			std::forward_iterator_tag
		>
	>,
	mpl::eval_if<
		mpl::and_<
			std::is_convertible<
				Traversal, single_pass_traversal_tag
			>,
			std::is_convertible<Reference, ValueParam>
		>,
		mpl::identity<std::input_iterator_tag>,
		mpl::identity<Traversal>
        >
>;

template <typename Cat>
using old_category_to_traversal = mpl::eval_if<
	is_convertible<Cat,std::random_access_iterator_tag>,
	mpl::identity<random_access_traversal_tag>,
	mpl::eval_if<
		is_convertible<Cat,std::bidirectional_iterator_tag>,
		mpl::identity<bidirectional_traversal_tag>,
		mpl::eval_if<
			is_convertible<Cat,std::forward_iterator_tag>,
			mpl::identity<forward_traversal_tag>,
			mpl::eval_if<
				is_convertible<Cat,std::input_iterator_tag>,
				mpl::identity<single_pass_traversal_tag>,
				mpl::eval_if<
					is_convertible<Cat,std::output_iterator_tag>,
					mpl::identity<incrementable_traversal_tag>,
					void
				>
			>
		>
	>
>;

template <typename Cat>
using iterator_category_to_traversal = mpl::eval_if<
	is_convertible<Cat, incrementable_traversal_tag>,
	mpl::identity<Cat>,
	boost::detail::old_category_to_traversal<Cat>
>;

template <typename Category, typename Traversal>
struct iterator_category_with_traversal : Category, Traversal {
	static_assert(std::is_convertible<
		typename iterator_category_to_traversal<Category>::type,
		Traversal
	>::value, "");
	static_assert(is_iterator_category<Category>::value, "");
	static_assert(!is_iterator_category<Traversal>::value, "");
	static_assert(!is_iterator_traversal<Category>::value, "");
	static_assert(is_iterator_traversal<Traversal>::value, "");
};

template <typename Traversal, typename ValueParam, typename Reference>
struct facade_iterator_category_impl {
	static_assert(!is_iterator_category<Traversal>::value, "");

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

struct iterator_core_access {
private:
	template<
		typename Derived, typename Value, typename CategoryOrTraversal,
		typename Reference, typename Difference
	> friend struct iterator_facade;

	struct facade_adapter {
		template<
			typename Derived, typename Value,
			typename CategoryOrTraversal, typename Reference,
			typename Difference
		> struct apply {
			typedef iterator_facade<
				Derived, Value, CategoryOrTraversal,
				Reference, Difference
			> type;
		};
	};

	template <typename Pack0, typename Pack1>
	using enable_if_derived_interoperable = enable_if_interoperable<
		typename mpl::front<Pack0>::type,
		typename mpl::front<Pack1>::type,
		typename mpl::apply<
			mpl::always<bool>,
			typename mpl::front<Pack0>::type,
			typename mpl::front<Pack1>::type
		>
	>;

	template <typename Pack0, typename Pack1>
	friend auto operator==(
		typename mpl::apply<
			mpl::unpack_args<facade_adapter>, Pack0
		>::type const &lhs,
		typename mpl::apply<
			mpl::unpack_args<facade_adapter>, Pack1
		>::type const &rhs
	) -> enable_if_derived_interoperable<Pack0, Pack1>;

/*
 

	template <
		typename Derived1, typename V1, typename TC1, typename Reference1,
		typename Difference1,
		typename Derived2, typename V2, typename TC2, typename Reference2,
		typename Difference2
	> friend typename enable_if_interoperable<
		Derived1, Derived2, typename mpl::apply<
			always_bool, Derived1, Derived2
		>::type
	>::type operator!=(
		iterator_facade<
			Derived1, V1, TC1, Reference1, Difference1
		> const &lhs,
		iterator_facade<
			Derived2, V2, TC2, Reference2, Difference2
		> const &rhs
	);

	template < typename Derived1, typename V1, typename TC1, typename Reference1, typename Difference1 , typename Derived2, typename V2, typename TC2, typename Reference2, typename Difference2 > friend typename boost::detail::enable_if_interoperable< Derived1, Derived2 , typename mpl::apply2<boost::detail::always_bool2,Derived1,Derived2>::type >::type operator <( iterator_facade<Derived1, V1, TC1, Reference1, Difference1> const& lhs , iterator_facade<Derived2, V2, TC2, Reference2, Difference2> const& rhs);
	template < typename Derived1, typename V1, typename TC1, typename Reference1, typename Difference1 , typename Derived2, typename V2, typename TC2, typename Reference2, typename Difference2 > friend typename boost::detail::enable_if_interoperable< Derived1, Derived2 , typename mpl::apply2<boost::detail::always_bool2,Derived1,Derived2>::type >::type operator >( iterator_facade<Derived1, V1, TC1, Reference1, Difference1> const& lhs , iterator_facade<Derived2, V2, TC2, Reference2, Difference2> const& rhs);
	template < typename Derived1, typename V1, typename TC1, typename Reference1, typename Difference1 , typename Derived2, typename V2, typename TC2, typename Reference2, typename Difference2 > friend typename boost::detail::enable_if_interoperable< Derived1, Derived2 , typename mpl::apply2<boost::detail::always_bool2,Derived1,Derived2>::type >::type operator <=( iterator_facade<Derived1, V1, TC1, Reference1, Difference1> const& lhs , iterator_facade<Derived2, V2, TC2, Reference2, Difference2> const& rhs);
	template < typename Derived1, typename V1, typename TC1, typename Reference1, typename Difference1 , typename Derived2, typename V2, typename TC2, typename Reference2, typename Difference2 > friend typename boost::detail::enable_if_interoperable< Derived1, Derived2 , typename mpl::apply2<boost::detail::always_bool2,Derived1,Derived2>::type >::type operator >=( iterator_facade<Derived1, V1, TC1, Reference1, Difference1> const& lhs , iterator_facade<Derived2, V2, TC2, Reference2, Difference2> const& rhs);


	template < typename Derived1, typename V1, typename TC1, typename Reference1, typename Difference1 , typename Derived2, typename V2, typename TC2, typename Reference2, typename Difference2 > friend typename boost::detail::enable_if_interoperable< Derived1, Derived2 , typename mpl::apply2<boost::detail::choose_difference_type,Derived1,Derived2>::type >::type operator -( iterator_facade<Derived1, V1, TC1, Reference1, Difference1> const& lhs , iterator_facade<Derived2, V2, TC2, Reference2, Difference2> const& rhs)

	;

	template <typename Derived, typename V, typename TC, typename R, typename D> friend inline Derived operator+ (iterator_facade<Derived, V, TC, R, D> const& , typename Derived::difference_type)




      ;

      template <typename Derived, typename V, typename TC, typename R, typename D> friend inline Derived operator+ (typename Derived::difference_type , iterator_facade<Derived, V, TC, R, D> const&)




      ;
*/
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

	template <typename Pack>
	static auto derived(typename mpl::apply<
		mpl::unpack_args<facade_adapter>, Pack
	>::type &facade) -> mpl::front<Pack>::type &
	{
		return *static_cast<I *>(&facade);
	}

	template <typename Pack>
	static auto derived(typename mpl::apply<
		mpl::unpack_args<facade_adapter>, Pack
	>::type const &facade) -> mpl::front<Pack>::type const &
	{
		return *static_cast<I const *>(&facade);
	}

	iterator_core_access();
};

}

template <
	typename Derived, typename Value, typename CategoryOrTraversal,
	typename Reference = Value&, typename Difference = std::ptrdiff_t
> struct iterator_facade {
private:
	Derived &derived()
	{
		return *static_cast<Derived *>(this);
	}

	Derived const &derived() const
	{
		return *static_cast<Derived const *>(this);
	}

	typedef detail::iterator_facade_types<
		Value, CategoryOrTraversal, Reference, Difference
	> associated_types;

	typedef detail::operator_arrow_dispatch<
		Reference, typename associated_types::pointer
	> operator_arrow_dispatch_;

protected:
	typedef iterator_facade<
		Derived, Value, CategoryOrTraversal, Reference, Difference
	> iterator_facade_;

	typedef mpl::package<
		Derived, Value, CategoryOrTraversal, Reference, Difference
	> iterator_facade_defs;

public:
	typedef typename associated_types::value_type value_type;
	typedef Reference reference;
	typedef Difference difference_type;
	typedef typename operator_arrow_dispatch_::result_type pointer;
	typedef typename associated_types::iterator_category iterator_category;

	reference operator*() const
	{
		return detail::iterator_core_access::dereference(
			this->derived()
		);
	}

	pointer operator->() const
	{
		return operator_arrow_dispatch_::apply(*this->derived());
	}

	typename detail::operator_brackets_result<
		Derived, Value, reference
	>::type operator[](difference_type n) const
	{
		typedef detail::use_operator_brackets_proxy<
			Value, Reference
		> use_proxy;

		return detail::make_operator_brackets_result<Derived>(
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

}}
#endif

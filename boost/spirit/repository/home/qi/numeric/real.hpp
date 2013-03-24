/*=============================================================================
    Copyright (c) 2001-2011 Joel de Guzman
    Copyright (c) 2011      Bryce Lelbach
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(SPIRIT_REPOSITORY_QI_REAL_MAR_13_2013_2300)
#define SPIRIT_REPOSITORY_QI_REAL_MAR_13_2013_2300

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/repository/home/qi/numeric/numeral.hpp>
#include <boost/spirit/repository/home/qi/numeric/real_policies.hpp>
#include <boost/mpl/set.hpp>

namespace boost { namespace spirit { namespace repository { namespace qi
{

namespace mpl = boost::mpl;	

struct real_policies;

///////////////////////////////////////////////////////////////////////
// This is the class that the user can instantiate directly in
// order to create a customized real parser
template <
	typename T = double,
	typename Policy = mpl::at<real_policies, T>::type
> struct real_parser : spirit::terminal<
	tag::stateful_tag<Policy, tag::double_, T>
> {
	typedef tag::stateful_tag<Policy, tag::double_, T> tag_type;

	real_parser() {}

	real_parser(Policy const &p) : spirit::terminal<tag_type>(p) {}
};

///////////////////////////////////////////////////////////////////////////
// Enablers
///////////////////////////////////////////////////////////////////////////
template <> // enables float_
struct use_terminal<qi::domain, tag::float_>
: mpl::true_ {};

template <> // enables double_
struct use_terminal<qi::domain, tag::double_>
: mpl::true_ {};

template <> // enables long_double
struct use_terminal<qi::domain, tag::long_double>
: mpl::true_ {};

///////////////////////////////////////////////////////////////////////////
template <typename A0> // enables lit(n)
struct use_terminal<
	qi::domain, terminal_ex<tag::lit, fusion::vector1<A0> >,
	typename enable_if<is_same<A0, float> >::type
> : mpl::true_ {};

template <typename A0> // enables lit(n)
struct use_terminal<
	qi::domain, terminal_ex<tag::lit, fusion::vector1<A0> >,
	typename enable_if<is_same<A0, double> >::type
> : mpl::true_ {};

template <typename A0> // enables lit(n)
struct use_terminal<
	qi::domain, terminal_ex<tag::lit, fusion::vector1<A0> >,
	typename enable_if<is_same<A0, long double> >::type
> : mpl::true_ {};

///////////////////////////////////////////////////////////////////////////
template <typename A0> // enables float_(...)
struct use_terminal<
	qi::domain, terminal_ex<tag::float_, fusion::vector1<A0> >
> : mpl::true_ {};

template <typename A0> // enables double_(...)
struct use_terminal<
	qi::domain, terminal_ex<tag::double_, fusion::vector1<A0> >
> : mpl::true_ {};

template <typename A0> // enables long_double(...)
struct use_terminal<
	qi::domain, terminal_ex<tag::long_double, fusion::vector1<A0> >
> : mpl::true_ {};

template <> // enables *lazy* float_(...)
struct use_lazy_terminal<qi::domain, tag::float_, 1> : mpl::true_ {};

template <> // enables *lazy* double_(...)
struct use_lazy_terminal<qi::domain, tag::double_, 1> : mpl::true_ {};

template <> // enables *lazy* long_double_(...)
struct use_lazy_terminal<qi::domain, tag::long_double, 1> : mpl::true_ {};

///////////////////////////////////////////////////////////////////////////
// enables custom real_parser
template <typename T, typename Policy>
struct use_terminal<
	qi::domain, tag::stateful_tag<Policy, tag::double_, T>
> : mpl::true_ {};

// enables custom real_parser(...)
template <typename T, typename Policy, typename A0>
struct use_terminal<
	qi::domain, terminal_ex<tag::stateful_tag<Policy, tag::double_, T>,
	fusion::vector1<A0> >
> : mpl::true_ {};

// enables *lazy* custom real_parser(...)
template <typename T, typename Policy>
struct use_lazy_terminal<
	qi::domain, tag::stateful_tag<Policy, tag::double_, T>, 1 // arity
> : mpl::true_ {};

///////////////////////////////////////////////////////////////////////////
// Parser generators: make_xxx function (objects)
///////////////////////////////////////////////////////////////////////////
template <typename T, typename Policy = mpl::at<real_policies, T>::type >
struct make_real {
	typedef any_numeral_parser<T, Policy> result_type;

	result_type operator()(unused_type, unused_type) const
	{
		return result_type();
	}
};

template <typename T, typename Policy = mpl::at<real_policies, T>::type >
struct make_direct_real {
	typedef literal_numeral_parser<T, Policy, false> result_type;

	template <typename Terminal>
	result_type operator()(Terminal const& term, unused_type) const
	{
		return result_type(T(fusion::at_c<0>(term.args)));
	}
};

template <typename T, typename Policy = mpl::at<real_policies, T>::type >
struct make_literal_real {
	typedef literal_numeral_parser<T, Policy> result_type;

	template <typename Terminal>
	result_type operator()(Terminal const& term, unused_type) const
	{
		return result_type(fusion::at_c<0>(term.args));
	}
};

///////////////////////////////////////////////////////////////////////////
template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::lit, fusion::vector1<A0> >, Modifiers,
	typename enable_if<is_same<A0, float> >::type
> : make_literal_real<float> {};

template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::lit, fusion::vector1<A0> >, Modifiers,
	typename enable_if<is_same<A0, double> >::type
> : make_literal_real<double> {};

template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::lit, fusion::vector1<A0> >, Modifiers,
	typename enable_if<is_same<A0, long double> >::type
> : make_literal_real<long double> {};

///////////////////////////////////////////////////////////////////////////
template <typename T, typename Policy, typename Modifiers>
struct make_primitive<
	tag::stateful_tag<Policy, tag::double_, T>, Modifiers
> : make_real<T, Policy> {};

template <typename T, typename Policy, typename A0, typename Modifiers>
struct make_primitive<
	terminal_ex<tag::stateful_tag<Policy, tag::double_, T>,
	fusion::vector1<A0> >, Modifiers
> : make_direct_real<T, Policy> {};

///////////////////////////////////////////////////////////////////////////
template <typename Modifiers>
struct make_primitive<tag::float_, Modifiers> : make_real<float> {};

template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::float_, fusion::vector1<A0> >, Modifiers
> : make_direct_real<float> {};

///////////////////////////////////////////////////////////////////////////
template <typename Modifiers>
struct make_primitive<tag::double_, Modifiers> : make_real<double> {};

template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::double_, fusion::vector1<A0> >, Modifiers
> : make_direct_real<double> {};

///////////////////////////////////////////////////////////////////////////
template <typename Modifiers>
struct make_primitive<tag::long_double, Modifiers> : make_real<long double> {};

template <typename Modifiers, typename A0>
struct make_primitive<
	terminal_ex<tag::long_double, fusion::vector1<A0> >, Modifiers
> : make_direct_real<long double> {};

}}}}

#endif

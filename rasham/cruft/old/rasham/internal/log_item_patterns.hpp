/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_INTERNAL_LOG_ITEM_PATTERNS_HPP)
#define _RASHAM_INTERNAL_LOG_ITEM_PATTERNS_HPP

#include <rasham/internal/log_item.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_real.hpp>

namespace rasham
{
namespace log_item
{
namespace pattern
{

template <typename input_iter_t, typename value_type>
struct pointer : qi::grammar<input_iter_t, value_type()> {
	typedef typename std::iterator_traits<input_iter_t>::value_type
	char_type;

	pointer() : pointer::base_type(start)
	{
		using namespace qi;

		uint_parser<value_type, 16, 1, 2 * sizeof(value_type)>
		ptr;

		start %= lit(char_type('0')) > lit(char_type('x'))
			 > ptr;
	}

	qi::rule<input_iter_t, value_type()> start;
};

template <typename input_iter_t, typename value_type>
struct time : qi::grammar<input_iter_t, value_type()> {
	typedef typename std::iterator_traits<input_iter_t>::value_type
	char_type;

	time() : time::base_type(start)
	{
		using namespace qi;

		real_parser<
			value_type, strict_ureal_policies<value_type>
		> t_spec;

		start %= t_spec;
	}

	qi::rule<input_iter_t, value_type()> start;
};

#if 0
#include <boost/spirit/include/qi_real.hpp>

template< typename T >
struct rational_parser
  : boost::spirit::qi::real_parser<
        boost::rational< T >
      , boost::spirit::qi::real_policies< boost::rational< T > >
    >
{};

namespace boost { namespace spirit { namespace traits {

    template< typename T >
    inline void scale( int exp, boost::rational< T >& n )
    {
        // Multiply n by 10^exp
        T num = n.numerator();
        T den = n.denominator();
        if( exp > 0 )
        {
            num *= std::pow( 10., exp );
        } else {
            den *= std::pow( 10., -exp );
        }

        n.assign( num, den );
    }

} /*namespace boost*/ } /*namespace spirit*/ } /*namespace traits*/
#endif

}
}
}

#endif

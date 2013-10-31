/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_INTERNAL_LOG_ITEM_HPP)
#define _RASHAM_INTERNAL_LOG_ITEM_HPP

#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/repository/include/qi_seek.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi.hpp>

BOOST_FUSION_DEFINE_TPL_STRUCT(
	(char_type),
	(rasham)(log_item),
	stmt,
	(std::basic_string<char_type>, name)
	(std::basic_string<char_type>, c_expr)
	(std::basic_string<char_type>, p_expr)
);

namespace rasham
{
namespace log_item
{

namespace qi = boost::spirit::qi;
namespace qi_repo = boost::spirit::repository::qi;

template <
	typename input_iter_t,
	typename char_type
	= typename std::iterator_traits<input_iter_t>::value_type
> struct range;

enum struct type : unsigned int {
	INVALID = 0,
	ESCAPE,
	CAPTURE
};

template <typename input_iter_t, typename char_type>
struct value_set_base {
	virtual ~value_set_base() {}
	virtual bool consume(
		stmt<char_type> const &s, input_iter_t &begin,
		input_iter_t const &end
	) = 0;
};

template <typename container_type, typename input_iter_t, typename char_type>
struct value_set : public value_set_base<input_iter_t, char_type> {
	typedef typename container_type::value_type     value_type;
	typedef qi::grammar<input_iter_t, value_type()> parser_type;

	value_set(parser_type const &parser_)
	: parser(parser_) {}

	bool consume(
		stmt<char_type> const &s, input_iter_t &begin,
		input_iter_t const &end
	);

	container_type *get(std::basic_string<char_type> name)
	{
		return var_names.find(name);
	}

private:
	parser_type const &parser;
	qi::symbols<char_type, container_type> var_names;
};

}
}

BOOST_FUSION_DEFINE_TPL_STRUCT(
	(input_iter_t)(char_type),
	(rasham)(log_item),
	range,
	(boost::iterator_range<input_iter_t>, item_match)
	(boost::iterator_range<input_iter_t>, extent)
	(rasham::log_item::type, type)
	(rasham::log_item::stmt<char_type>, stmt)
);

namespace rasham
{
namespace log_item
{

template <typename char_type>
struct basic_parsers;

template <>
struct basic_parsers<char> {
	typedef boost::spirit::standard::char_type   char_type;
	typedef boost::spirit::standard::alnum_type  alnum_type;
	typedef boost::spirit::standard::alpha_type  alpha_type;
	typedef boost::spirit::standard::xdigit_type xdigit_type;
};

template <>
struct basic_parsers<wchar_t> {
	typedef boost::spirit::standard_wide::char_type   char_type;
	typedef boost::spirit::standard_wide::alnum_type  alnum_type;
	typedef boost::spirit::standard_wide::alpha_type  alpha_type;
	typedef boost::spirit::standard_wide::xdigit_type xdigit_type;
};

template <typename input_iter_t>
struct scanner : qi::grammar<input_iter_t, range<input_iter_t>()> {
	typedef typename std::iterator_traits<input_iter_t>::value_type
	char_type;

	struct on_pattern {
		template <typename context_type>
		void operator()(
			stmt<char_type> const &attr, context_type &ctx,
			bool &pass
		) const
		{
			using boost::phoenix::at_c;

			at_c<0>(ctx.attributes).type = type::CAPTURE;
			at_c<0>(ctx.attributes).stmt = attr;
		}
	};

	scanner(char_type esc_) : scanner::base_type(pattern)
	{
		using boost::phoenix::at_c;
		using namespace qi;
		using namespace qi_repo;
		using namespace qi::labels;

		typename basic_parsers<char_type>::char_type  char_;
		typename basic_parsers<char_type>::alpha_type alpha;
		typename basic_parsers<char_type>::alnum_type alnum;

		curly_block_char %= (lit(char_type('\\')) || char_)
				    - lit(char_type('}'));

		curly_block = char_(char_type('{'))[_val += _1] >> *(
			curly_block[_val += _1] | curly_block_char[_val += _1]
		) >> char_(char_type('}'))[_val += _1];

		item_p_expr %= confix(
			lit(char_type('{')), lit(char_type('}'))
		)[*(curly_block | curly_block_char)];

		item_c_expr %= confix(lit(char_type('[')), lit(char_type(']')))
			       [*(char_ - lit(char_type(']')))];

		item_name %= alpha >> *(alnum | lit(char_type('_')));

		item %= item_name >> (-item_c_expr >> -item_p_expr);

		pattern = raw[seek[
			raw[lit(esc_) >> (
				lit(esc_)[at_c<2>(_val) = type::ESCAPE]
				| item[on_pattern()]
			)][at_c<0>(_val) = _1]
		]][at_c<1>(_val) = _1];
	}

	qi::rule<input_iter_t, char_type()> curly_block_char;
	qi::rule<input_iter_t, std::basic_string<char_type>()> curly_block;
	qi::rule<input_iter_t, std::basic_string<char_type>()> item_p_expr;
	qi::rule<input_iter_t, std::basic_string<char_type>()> item_c_expr;
	qi::rule<input_iter_t, std::basic_string<char_type>()> item_name;
	qi::rule<input_iter_t, stmt<char_type>()> item;
	qi::rule<input_iter_t, range<input_iter_t>()> pattern;
};

}
}

#endif

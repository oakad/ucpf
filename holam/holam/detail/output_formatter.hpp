/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_CA7734C48F3FB819FA74B86C178619BD)
#define HPP_CA7734C48F3FB819FA74B86C178619BD

#include <cstdio>
#include <type_traits>
#include <holam/traits/string.hpp>
#include <holam/traits/integral.hpp>
#include <holam/detail/format_integral.hpp>

namespace ucpf { namespace holam { namespace detail {

template <typename T, typename U = std::__void_t<>>
struct is_trait_disabled : std::false_type {};

template <typename T>
struct is_trait_disabled<
	T, std::__void_t<decltype(T::disabled)>
>: std::true_type {};

template <typename OutputIterator>
struct output_formatter {
	enum arg_output_mode {
		SIGNED,
		UNSIGNED,
		FLOAT,
		STRING
	};

	struct arg_def;

	struct nested_output {
		nested_output(arg_def &arg_d_)
		: arg_d(arg_d_)
		{}

		arg_def &arg_d;
	};

	struct arg_def {
		arg_def(output_formatter &formatter_)
		: formatter(formatter_), arg_pos(0),
		  fmt_pos(formatter.fmt_pos + 1), valid(false)
		{}

		~arg_def()
		{
			if (valid)
				formatter.fmt_pos = fmt_pos;
		}

		template <typename Arg>
		bool emit_value(Arg &&arg)
		{
			typedef traits::integral_value<Arg> int_trait_type;
			//typedef traits::fp_value<Arg> fp_trait_type;
			typedef traits::string_value<Arg> string_trait_type;

			switch (output_mode) {
			case SIGNED: {
				if (is_trait_disabled<int_trait_type>::value)
					return false;

				format_integral(
					int_trait_type::apply(
						std::forward<Arg>(arg)
					), *this
				);
				return true;
			}
			case UNSIGNED:
				break;
			case FLOAT:
				break;
			case STRING: {
				if (!is_trait_disabled<
					string_trait_type
				>::value) {
					auto n_out(nested_output(*this));
					string_trait_type::apply(
						n_out, std::forward<Arg>(arg)
					);
					return true;
				} else
					return false;
			}};
			return true;
		}

		int get_base()
		{
			return 10;
		}

		output_formatter &formatter;
		arg_output_mode output_mode;
		std::size_t arg_pos;
		std::size_t fmt_pos;
		bool valid;
	};

	output_formatter(OutputIterator &iter_, char const *format_)
	: iter(iter_), format(format_), fmt_pos(0), count(0), arg_pos(0)
	{}

	bool advance(bool parse)
	{
		if (parse) {
			while (true) {
				if (!format[fmt_pos])
					return false;

				if ((format[fmt_pos] == '%')  && !at_escape())
					return true;

				*iter++ = format[fmt_pos];
				++fmt_pos;
				++count;
			}
		} else {
			while (format[fmt_pos]) {
				*iter++ = format[fmt_pos];
				++fmt_pos;
				++count;
			}
			return false;
		}
	}

	bool at_escape()
	{
		if (!format[fmt_pos + 1])
			return true;

		if (format[fmt_pos + 1] == '%') {
			++fmt_pos;
			return true;
		} else
			return false;
	}

	void emit_char(char ch)
	{
		*iter++ = ch;
		++count;
	}

	arg_def get_arg_def(std::size_t arg_count)
	{
		arg_def rv(*this);

		if ('d' != format[rv.fmt_pos])
			return rv;
		else
			rv.output_mode = SIGNED;

		++rv.fmt_pos;
		if (arg_count <= arg_pos)
			return rv;

		rv.arg_pos = arg_count - 1 - arg_pos;
		++arg_pos;
		rv.valid = true;
		
		return rv;
	}

	OutputIterator &iter;
	char const *format;
	std::size_t fmt_pos;
	std::size_t count;
	std::size_t arg_pos;
};

}}}
#endif

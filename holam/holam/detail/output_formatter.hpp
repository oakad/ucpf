/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_CA7734C48F3FB819FA74B86C178619BD)
#define HPP_CA7734C48F3FB819FA74B86C178619BD

namespace ucpf { namespace holam { namespace detail {

template <typename OutputIterator>
struct output_formatter {
	struct arg {
		arg(output_formatter &formatter_)
		: formatter(formatter_), arg_pos(0),
		  fmt_pos(formatter.fmt_pos + 1), valid(false)
		{}

		~arg()
		{
			if (valid)
				formatter.fmt_pos = fmt_pos;
		}

		output_formatter &formatter;
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
			while (format(fmt_pos) {
				*iter++ = format[fmt_pos];
				++fmt_pos;
				++count;
			}
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

	arg get_arg(std::size_t arg_count)
	{
		arg rv(this);

		if (!format[rv.fmt_pos])
			return rv;

		if ('d' != format[rv.fmt_pos])
			return rv;

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

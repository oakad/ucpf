/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_8AB2B6049EB6ABD37D6443C0A7527014)
#define HPP_8AB2B6049EB6ABD37D6443C0A7527014

#include <holam/detail/formatter.hpp>
#include <holam/output/stdio.hpp>

/*
 * An argument placeholder within string is designated by a balanced pair of
 * delimiters.
 * - Default delimiters: curly braces ('{}').
 * - Delimiter multiplicity: multiple sequential opening delimiters are
 *   forwarded to output as is, same as multiple closing delimiters.
 * - Suppressed delimiter: hash character following the opening delimiter '{#'
 *   will suppress the placeholder parsing and result in delimiter being printed
 *   as is.
 * - Delimiter replacement: special placeholder sequence '!^^' (exclamation
 *   mark, any character, any character) within a currently valid placeholder
 *   definitions will replace the placeholder delimiters with the characters
 *   specified (for example, '{![]}' will set the active placeholder delimiters
 *   to square brackets).
 *
 * An argument placeholder within delimiters may contain 0 to 3 sections
 * delimited by vertical bars '|': '{}', '{s0}', '{s0|s1}', '{s0|s1|s2}',
 * '{|s1}', '{|s1|s2}', '{||s2}'. 
 *
 * Format of section s0: 0 or more decimal digits followed by 0 or 1 characters.
 * Decimal digits designate the desired argument for replacement (if not
 * specified, internal argument counter is used and incremented). Character,
 * if specified, serves as preferred output format flag:
 * - 'i': integral signed
 * - 'u': integral unsigned
 * - 'f': floating point
 * - 's': character string
 *
 * Format of section s1: various flags controlling output
 *
 * Format of section s2: field width control in a form of '[+][[*]d][.[*]d]',
 * where 'd' is 1 or more decimal digits.
 * - '+': force output of 'plus' character for values identified as positive
 *        numerals
 * - '[[*]d]: 1 or more decimal digits are interpreted as minimal output field
 *            width, unless preceded by asterisk '*'. In the later case, digits
 *            are interpreted as argument position to obtain the minimal field
 *            width from.
 * - '[.[*]d]': 1 or more decimal digits following the dot are interpreted as
 *              desired floating point precision, unless preceded by asterisk
 *              '*'. In the later case, digits are interpreted as argument
 *              position to obtain the precision value from.
 */
namespace ucpf { namespace holam {
namespace detail {

template <typename FormatSequence>
struct format_range {
	typedef decltype(
		std::cbegin(std::declval<FormatSequence>())
	) iterator_type;

	format_range(FormatSequence const &format)
	: first(std::cbegin(format)), last(std::cend(format))
	{}

	iterator_type first;
	iterator_type last;
};

template <>
struct format_range<char const *> {
	typedef char const *iterator_type;

	format_range(char const *format)
	: first(format), last(format + std::strlen(format))
	{}

	iterator_type first;
	iterator_type last;
};

template <typename OutputIterator, typename FormatSequence>
OutputIterator &out_impl(
	OutputIterator &iter, FormatSequence const &format,
	arg_holder_base **arg_refs, uint16_t pos
) {
	format_range<FormatSequence> r(format);
	static_assert(
		std::is_same<
			typename std::iterator_traits<
				typename format_range<FormatSequence>::iterator_type
			>::value_type,
			typename std::iterator_traits<
				OutputIterator
			>::value_type
		>::value,
	       "Format sequence character type must match output iterator "
		"character type"
	);

	u8_formatter formatter(arg_refs, pos);
	return formatter.process(iter, r.first, r.last);
}

template <
	typename OutputIterator, typename FormatSequence,
	typename Arg0
>
OutputIterator &out_impl(
	OutputIterator &iter, FormatSequence const &format,
	arg_holder_base **arg_refs, uint16_t pos, Arg0 arg0
) {
	arg_holder<Arg0, OutputIterator> arg_ref(arg0, iter);
	arg_refs[pos] = &arg_ref;

	return out_impl(
		iter, format, arg_refs, pos + 1
	);
}

template <
	typename OutputIterator, typename FormatSequence,
	typename Arg0, typename... Args
>
OutputIterator &out_impl(
	OutputIterator &iter, FormatSequence const &format,
	arg_holder_base **arg_refs, uint16_t pos,
	Arg0 arg0, Args &&...args
) {
	arg_holder<Arg0, OutputIterator> arg_ref(arg0, iter);
	arg_refs[pos] = &arg_ref;
	return out_impl(
		iter, format, arg_refs, pos + 1, std::forward<Args>(args)...
	);
}

}

template <typename OutputIterator, typename FormatSequence, typename... Args>
OutputIterator &out(
	OutputIterator &iter, FormatSequence const &format, Args &&...args
) {
	detail::arg_holder_base *arg_refs[sizeof...(args)];
	return detail::out_impl(
		iter, format, arg_refs, uint16_t(0),
		std::forward<Args>(args)...
	);
}

template <typename... Args>
std::size_t out(char const *format, Args &&...args)
{
	output::stdio dest(stdout);
	return out(dest, format, std::forward<Args>(args)...).count();
}

}}
#endif

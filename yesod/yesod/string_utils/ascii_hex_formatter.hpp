/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_BDE50A1177CDBD345AF75F5A9143DB53)
#define BDE50A1177CDBD345AF75F5A9143DB53

namespace ucpf::yesod::string_utils {

template <typename OutputIterator, typename CharSeq>
struct ascii_hex_formatter {
	static OutputIterator &apply(
		OutputIterator &out, CharSeq &&seq, size_t bytes_per_line = 16
	)
	{
		char hex_line[bytes_per_line * 3];
		char print_line[bytes_per_line];
		size_t offset(0), line_pos(0);
		auto last(seq.cend());

		for (auto first(seq.cbegin()); first != last; ++first) {
			char ch(*first);
			hex_line[3 * line_pos] = hex_digits[(ch >> 4) & 0xf];
			hex_line[3 * line_pos + 1] = hex_digits[ch & 0xf];
			hex_line[3 * line_pos + 2] = ' ';
			print_line[line_pos] = std::isprint(ch) ? ch : '.';
			++line_pos;
			if (line_pos == bytes_per_line) {
				emit_line(
					out, hex_line, print_line, offset,
					bytes_per_line
				);
				offset += bytes_per_line;
				line_pos = 0;
			}
		}

		if (line_pos) {
			for (; line_pos < bytes_per_line; ++line_pos) {
				hex_line[3 * line_pos] = ' ';
				hex_line[3 * line_pos + 1] = ' ';
				hex_line[3 * line_pos + 2] = ' ';
				print_line[line_pos] = ' ';
			}
			emit_line(
				out, hex_line, print_line, offset,
				bytes_per_line
			);
		}
		return out;
	}

private:
	constexpr static char const hex_digits[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	static void emit_line(
		OutputIterator &out, char const *hex_line,
		char const *print_line, size_t offset, size_t bytes_per_line
	)
	{
		for (size_t c(4); c <= 32; c += 4)
			*out++ = hex_digits[(offset >> (32 - c)) & 0xf];

		*out++ = ' ';
		*out++ = ' ';
		for (size_t c(0); c < 3 * bytes_per_line; ++c)
			*out++ = hex_line[c];

		*out++ = ' ';
		for (size_t c(0); c < bytes_per_line; ++c)
			*out++ = print_line[c];

		*out++ = '\n';
	}
};

template <typename OutputIterator, typename CharSeq>
constexpr char const ascii_hex_formatter<
	OutputIterator, CharSeq
>::hex_digits[];

}
#endif

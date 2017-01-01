/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_B9CF8BC160BF3047A7C115BA88C856D2)
#define HPP_B9CF8BC160BF3047A7C115BA88C856D2

namespace ucpf::yesod::string_utils {

template <typename U8CharSeq>
struct u8string_tokenizer_sce {
	u8string_tokenizer_sce(U8CharSeq const &seq_, char sep_, char esc_)
	: t_state(NORMAL), seq(seq_), pos(seq_.cbegin()), sep(sep_), esc(esc_)
	{}

	void reset()
	{
		t_state = NORMAL;
		pos = seq.cbegin();
	}

	auto char_count()
	{
		size_t rv(0);

		while (parse([&rv](char c) {
			++rv;
		}));
		reset();
		return rv;
	}

	auto next_element(uint8_t *data, size_t &count)
	{
		return parse([data, &count](char c) {
			data[count] = c;
			++count;
		});
	}

private:
	template <typename Consumer>
	bool parse(Consumer &&cons)
	{
		for (; pos != seq.cend(); ++pos) {
			auto c(*pos);
			switch (t_state) {
			case NORMAL:
				if (c == sep) {
					t_state = SEPARATOR;
					++pos;
					return true;
				} else if (c == esc)
					t_state = ESCAPE;
				else
					cons(c);
				break;
			case SEPARATOR:
				if (c == sep) {
					++pos;
					return true;
				} else if (c == esc)
					t_state = ESCAPE;
				else
					cons(c);
				break;
			case ESCAPE:
				cons(c);
				t_state = NORMAL;
				break;
			}
		}
		return false;
	}

	enum T_STATE {
		NORMAL,
		SEPARATOR,
		ESCAPE
	} t_state;

	U8CharSeq const &seq;
	typename U8CharSeq::const_iterator pos;
	char sep;
	char esc;
};
}
#endif

/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_3A7A8AE4B19710EC3DCCC60AAF9FDADA)
#define HPP_3A7A8AE4B19710EC3DCCC60AAF9FDADA

#include <holam/traits/string_value.hpp>

namespace ucpf { namespace holam { namespace detail {

template <typename T, typename OutputIterator>
struct arg_holder : arg_holder_base {
	typedef T value_type;

	arg_holder(value_type &v_, OutputIterator &out_)
	: v(v_), out(out_)
	{}

	virtual bool apply(place_holder const &ph)
	{
		switch (ph.output_trait) {
		case place_holder::STRING: {
			using traits_type = traits::string_value<value_type>;

			if (!is_trait_disabled<traits_type>::value)
				return traits_type::apply(out, v, ph);
			else
				return false;
		}
		case place_holder::SIGNED:
		case place_holder::UNSIGNED:
		case place_holder::FLOAT:
			break;
		};
		return false;
	}

	value_type &v;
	OutputIterator &out;
};

struct formatter_base {
	enum formatter_state {
		ERROR = -1,
		NORMAL = 0,
		PH_POSSIBLE,
		PH_BODY,
		PH_REPLACE_LEFT,
		PH_REPLACE_RIGHT,
		PH_REPLACE_FINISH
	};

	formatter_base(arg_holder_base **arg_refs_, uint16_t count_)
	: arg_refs(arg_refs_), arg_count(count_), arg_pos(0)
	{}

	bool process_ph(place_holder &ph)
	{
		return ph.apply(arg_refs, arg_count, arg_pos);
	}

	arg_holder_base **arg_refs;
	uint16_t arg_count;
	uint16_t arg_pos;
};

template <typename FormatterImpl>
struct single_symbol_formatter : formatter_base {
	single_symbol_formatter(
		arg_holder_base **arg_refs_, uint16_t count_
	) : formatter_base(arg_refs_, count_)
	{}

	template <typename OutputIterator, typename FormatIterator>
	OutputIterator &process(
		OutputIterator &out, FormatIterator first, FormatIterator last
	) {
		formatter_state state(NORMAL);
		decltype(first) ph_start;
		typename FormatterImpl::char_type delim_left(
			FormatterImpl::DELIM_LEFT
		);
		typename FormatterImpl::char_type delim_right(
			FormatterImpl::DELIM_RIGHT
		);
		typename FormatterImpl::char_type delim_last(
			FormatterImpl::DELIM_RIGHT
		);

		for (; first != last; ++first) {
			auto ch(*first);

			switch (state) {
			case ERROR:
				*out++ = ch;
				break;
			case NORMAL:
				if (ch != delim_left)
					*out++ = ch;
				else {
					state = PH_POSSIBLE;
					ph_start = first;
				}
				break;
			case PH_POSSIBLE:
				if (ch == delim_left) {
					*out++ = delim_left;
					++ph_start;
					break;
				} else if (
					ch == FormatterImpl::DELIM_SUPPRESS
				) {
					*out++ = delim_left;
					state = NORMAL;
					break;
				} else if (
					ch == FormatterImpl::DELIM_REPLACE
				) {
					state = PH_REPLACE_LEFT;
					break;
				} else
					state = PH_BODY;
			case PH_BODY:
				if (ch == delim_right) {
					if (!process_ph(
						out, ph_start + 1, first
					)) {
						*out++ = *ph_start;
						first = ph_start;
						state = ERROR;
					} else
						state = NORMAL;
				}
				break;
			case PH_REPLACE_LEFT:
				delim_left = ch;
				state = PH_REPLACE_RIGHT;
				break;
			case PH_REPLACE_RIGHT:
				delim_right = ch;
				state = PH_REPLACE_FINISH;
				break;
			case PH_REPLACE_FINISH:
				if (ch == delim_last) {
					delim_last = delim_right;
					state = NORMAL;
				} else {
					*out++ = *ph_start;
					first = ph_start;
					state = ERROR;
				}
				break;
			}
		}

		if ((state != NORMAL) && (state != ERROR)) {
			for (; ph_start != last; ++ph_start)
				*out++ = *ph_start;
		}

		return out;
	}

	template <typename OutputIterator, typename FormatIterator>
	bool process_ph(
		OutputIterator &out, FormatIterator first, FormatIterator last
	) {
		place_holder ph;

		for (; first != last; ++first)
		{
		}

		return formatter_base::process_ph(ph);
	}
};

struct u8_formatter : single_symbol_formatter<u8_formatter> {
	typedef char char_type;

	enum symbol_def: char_type {
		DELIM_LEFT = '{',
		DELIM_RIGHT = '}',
		DELIM_SUPPRESS = '#',
		DELIM_REPLACE = '!'
	};

	u8_formatter(arg_holder_base **arg_refs_, uint16_t count_)
	: single_symbol_formatter(arg_refs_, count_)
	{}
};

}}}
#endif

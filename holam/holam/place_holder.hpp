/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_933DBC3560F63F9C6A880AE87D09EFB6)
#define HPP_933DBC3560F63F9C6A880AE87D09EFB6

namespace ucpf { namespace holam {

struct place_holder;

namespace detail {

struct formatter_base;

template <typename FormatterImpl>
struct single_symbol_formatter;

struct arg_holder_base {
	virtual bool apply(place_holder const &ph) = 0;
};

template <typename T, typename OutputIterator>
struct arg_holder;

}

struct place_holder {

private:
	friend struct detail::formatter_base;

	template <typename T, typename OutputIterator>
	friend struct detail::arg_holder;

	template <typename FormatterImpl>
	friend struct detail::single_symbol_formatter;

	place_holder()
	: output_trait(STRING), arg_pos(arg_pos_next)
	{}

	bool apply(
		detail::arg_holder_base **arg_refs, uint16_t arg_count,
		uint16_t &next_pos
	)
	{
		if (arg_pos == arg_pos_next)
			arg_pos = next_pos++;

		if (arg_pos >= arg_count)
			return false;

		return arg_refs[arg_pos]->apply(*this);
	}

	enum output_trait_type {
		STRING,
		SIGNED,
		UNSIGNED,
		FLOAT
	};

	constexpr static uint16_t arg_pos_next = 0xffff;

	enum output_trait_type output_trait;
	uint16_t arg_pos;
};

}}
#endif

/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_54E9198F94D0BD96956D13EA0ABC931A)
#define HPP_54E9198F94D0BD96956D13EA0ABC931A

namespace ucpf { namespace yesod { namespace string_map { namespace encoding {

struct u8_in_order {
	typedef uint8_t char_type;
	typedef uint16_t offset_type;

	constexpr u8_in_order()
	: l_offset(0)
	{
		for (int c = 0; c < 256; ++c) {
			c2o_map[c] = c;
			o2c_map[c] = c;
		}
	}

	template <typename Iterator>
	bool char_to_offset(
		offset_type &offset, Iterator &first, Iterator const &last
	) const
	{
		if (first == last)
			return false;

		uint8_t ch(*first);
		uint8_t offset(c2o_map[ch]);

		if (offset < l_offset) {
			++offset;
			++first;
			return true;
		} else
			return false;
	}

	template <typename Iterator>
	bool char_to_offset(
		offset_type &offset, Iterator &first, Iterator const &last
	)
	{
		if (first == last)
			return false;

		uint8_t ch(*first);
		uint8_t offset(c2o_map[ch]);

		if (offset < l_offset)
			++offset;
		else {
			swap_offsets(l_offset, ch);
			++l_offset;
			offset = l_offset;
		}
		++first;
		return true;
	}

private:
	void swap_offsets(uint8_t offset, uint8_t ch)
	{
		auto p_o(c2o_map[ch]);
		auto p_ch(o2c_map[offset]);
		c2o_map[ch] = offset;
		o2c_map[offset] = ch;
		c2o_map[p_ch] = p_o;
		o2c_map[p_o] = p_ch;
	}

	uint8_t c2o_map[256];
	uint8_t o2c_map[256];
	offset_type l_offset;
};

}}}}
#endif

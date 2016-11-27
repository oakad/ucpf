/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_D486B96ECCF2B1A4D20D64CE9ECB4A0D)
#define HPP_D486B96ECCF2B1A4D20D64CE9ECB4A0D

namespace ucpf::yesod::detail {

struct ptr_or_data {
	constexpr static size_t ptr_size = sizeof(uintptr_t);
	constexpr static size_t data_size = 2 * ptr_size;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	constexpr static size_t extra_lsb_pos = 0;
	constexpr static size_t extra_msb_pos = ptr_size - 1;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	constexpr static size_t extra_lsb_pos = ptr_size - 1;
	constexpr static size_t extra_msb_pos = 0;
#endif

	union {
		struct {
			uintptr_t extra;
			uintptr_t ptr;
		};
		uint8_t bytes[data_size];
	};
};

}
#endif

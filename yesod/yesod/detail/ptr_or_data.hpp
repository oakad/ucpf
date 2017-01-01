/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_D486B96ECCF2B1A4D20D64CE9ECB4A0D)
#define HPP_D486B96ECCF2B1A4D20D64CE9ECB4A0D

#include <yesod/detail/bitops.hpp>
#include <limits>

namespace ucpf::yesod::detail {

struct ptr_or_data {
	typedef uintptr_t extra_type;

	constexpr static size_t extra_type_bits = std::numeric_limits<
		extra_type
	>::digits;

	constexpr static size_t extra_type_bit_shift = extra_type_bits - clz(
		extra_type_bits
	);

	constexpr static extra_type extra_type_zeros = extra_type(0);
	constexpr static extra_type extra_type_ones = ~extra_type(0);
	constexpr static size_t data_size = sizeof(extra_type) + sizeof(void *);

	ptr_or_data &reset()
	{
		__builtin_memset(bytes, 0, data_size);
		return *this;
	}

	template <typename PtrType>
	auto get_ptr_at(size_t offset)
	{
		return reinterpret_cast<PtrType>(
			reinterpret_cast<uint8_t *>(ptr) + offset
		);
	}

	template <typename PtrType>
	auto get_ptr_at(size_t offset) const
	{
		return reinterpret_cast<PtrType>(
			reinterpret_cast<uint8_t const *>(ptr) + offset
		);
	}

	extra_type get_extra_bits_lsb(size_t offset, size_t count) const
	{
		return (extra >> offset) & ((extra_type(1) << count) - 1);
	}

	ptr_or_data &set_extra_bits_lsb(
		extra_type value, size_t offset, size_t count
	)
	{
		auto mask((extra_type(1) << count) - 1);
		extra &= ~(mask << offset);
		extra |= (value & mask) << offset;
		return *this;
	}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	extra_type get_extra_bits(size_t offset, size_t count) const
	{
		return get_extra_bits_lsb(offset, count);
	}

	ptr_or_data &set_extra_bits(
		extra_type value, size_t offset, size_t count
	)
	{
		return set_extra_bits_lsb(value, offset, count);
	}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	extra_type get_extra_bits(size_t offset, size_t count) const
	{
		return get_extra_bits_lsb(
			extra_type_bits - offset - count, count
		);
	}

	ptr_or_data &set_extra_bits(
		extra_type value, size_t offset, size_t count
	)
	{
		return set_extra_bits_lsb(
			value, extra_type_bits - offset - count, count
		);
	}
#else
#error Unknown endianness!
#endif

	union {
		struct {
			extra_type extra;
			void *ptr;
		};
		uint8_t bytes[data_size];
	};
};

}
#endif

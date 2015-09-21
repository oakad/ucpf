/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_6DEB721F67A9697F61B81CF9B8551CBA)
#define HPP_6DEB721F67A9697F61B81CF9B8551CBA

#include <memory>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {

template <std::size_t ByteSize>
struct basic_block {
	template <std::size_t OtherByteSize>
	struct rebind {
		typedef basic_block<OtherByteSize> type;
	};

	template <typename T>
	T *cast()
	{
		return get(0, ByteSize);
	}

	template <typename T>
	T *cast(std::size_t first, std::size_t last)
	{
		typedef std::aligned_storage_t<
			sizeof(T), alignof(T)
		> storage_type;

		auto const base = reinterpret_cast<uintptr_t>(data);
		auto const a_base = (base - 1u + alignof(T)) & -alignof(T);
		auto const a_off = a_base - base;

		return reinterpret_cast<T *>(get(
			a_off + first * sizeof(storage_type),
			a_off + last * sizeof(storage_type)
		));
	}

	template <typename T>
	std::size_t cast_size() const
	{
		typedef std::aligned_storage_t<
			sizeof(T), alignof(T)
		> storage_type;

		auto const base = reinterpret_cast<uintptr_t>(data);
		auto const a_base = (base - 1u + alignof(T)) & -alignof(T);
		return (base + size() - a_base) / sizeof(storage_type);
	}

	uint8_t *get()
	{
		return get(0, ByteSize);
	}

	uint8_t *get(std::size_t first, std::size_t last)
	{
		return data + first;
	}

	constexpr std::size_t size() const
	{
		return ByteSize;
	}

	uint8_t data[ByteSize];
};

}
}}}}
#endif

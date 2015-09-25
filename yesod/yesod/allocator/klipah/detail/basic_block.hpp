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
struct alignas(uintptr_t) basic_block {
	template <std::size_t OtherByteSize>
	using rebind = basic_block<OtherByteSize>;

	constexpr static std::size_t storage_size = ByteSize;

	template <typename T, typename... Args>
	auto construct(std::size_t pos_, std::size_t count, Args&&... args)
	{
		std::size_t pos = 0;
		auto deleter = [&pos](auto p) {
			if (!pos)
				return;

			do {
				--pos;
				(p + pos)->~T();
			} while (pos > 0);
		};

		std::unique_ptr<T[], decltype(deleter)> pp(
			get_range_as<T>(pos_, count), deleter
		);

		for (; pos < count; ++pos)
			::new(&pp[pos]) T(std::forward<Args>(args)...);

		return std::unique_ptr<T[], yield_block<T>>(
			pp.release(), yield_block<T>()
		);		
	}

	template <typename T>
	auto assign(std::size_t pos_, std::size_t count, T const &other)
	{
		auto p = get_range_as<T>(pos_, count);
		for (std::size_t pos(0); pos < count; ++pos)
			p[pos] = other;

		return std::unique_ptr<T[], yield_block<T>>(
			p, yield_block<T>()
		);
	}

	template <typename T>
	void destroy(std::size_t pos_, std::size_t count)
	{
		auto v = get_range_as<T>(pos_, count);
		for (std::size_t pos(0); pos < count; ++pos)
			(v + pos)->~T();
	}

	template <typename T>
	auto get(std::size_t pos, std::size_t count)
	{
		return std::unique_ptr<T[], yield_block<T>>(
			get_range_as<T>(pos, count), yield_block<T>()
		);
	}

	template <typename T>
	auto count() const
	{
		typedef std::aligned_storage_t<
			sizeof(T), alignof(T)
		> storage_type;

		auto const a_off = aligned_offset<alignof(storage_type)>();
		return (size() - a_off) / sizeof(storage_type);
	}

	template <std::size_t OtherByteSize>
	auto subblock_ptr(std::size_t pos)
	{
		return construct<
			basic_block<OtherByteSize>
		>(pos, 1).release();
	}

	void yield()
	{
	}

	constexpr std::size_t size() const
	{
		return storage_size;
	}

private:
	template <typename Pointer>
	struct yield_block {
		void operator()(Pointer *p)
		{}
	};

	template <std::size_t Alignment>
	auto aligned_offset() const
	{
		auto const base = reinterpret_cast<uintptr_t>(data);
		auto const a_base = (base - 1u + Alignment) & -Alignment;
		return a_base - base;
	}

	void *get_range(std::size_t pos, std::size_t count)
	{
		if ((pos >= storage_size) || ((pos + count) > storage_size))
			throw std::out_of_range("basic_block");

		return data + pos;
	}

	template <typename T>
	auto get_range_as(std::size_t pos, std::size_t count)
	{
		typedef std::aligned_storage_t<
			sizeof(T), alignof(T)
		> storage_type;

		auto const off = aligned_offset<
			alignof(storage_type)
		>() + pos * sizeof(storage_type);

		return reinterpret_cast<T *>(get_range(
			off, count * sizeof(storage_type)
		));
	}

	uint8_t data[storage_size];
};

template <>
struct basic_block<0> {
	template <std::size_t OtherByteSize>
	using rebind = basic_block<OtherByteSize>;

	constexpr static std::size_t storage_size = 0;
};

}
}}}}
#endif

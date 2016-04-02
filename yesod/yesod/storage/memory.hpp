/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_65446A15760B3A5D0F0E8AD39444619F)
#define HPP_65446A15760B3A5D0F0E8AD39444619F

#include <yesod/storage/detail/memory_pointer_facade.hpp>
#include <yesod/storage/basic_pointer.hpp>
#include <yesod/bitops.hpp>
#include <cstdlib>

namespace ucpf { namespace yesod { namespace storage {

struct memory {
	typedef detail::memory_pointer_facade::size_type size_type;

	typedef basic_pointer<
		std::uint8_t, detail::memory_pointer_facade, false
	> storage_pointer;

	template <typename ElementType>
	using pointer = basic_pointer<
		ElementType, detail::memory_pointer_facade, false
	>;

	template <typename ElementType>
	using const_pointer = basic_pointer<
		ElementType, detail::memory_pointer_facade, true
	>;

	template <typename ElementType>
	storage_pointer storage_cast(pointer<ElementType> p) const
	{
		storage_pointer rv;
		pointer_access::assign(rv, pointer_access::address(p), nullptr);
		return rv;
	}

	template <typename ElementType>
	pointer<ElementType> storage_cast(storage_pointer p) const
	{
		pointer<ElementType> rv;
		pointer_access::assign(rv, pointer_access::address(p), nullptr);
		return rv;
	}

	storage_pointer allocate(
		size_type count,
		size_type alignment = alignof(std::max_align_t)
	) const
	{
		storage_pointer p;
		pointer_access::assign(p, nullptr, nullptr);

		if (alignment == 1)
			pointer_access::assign(
				p, reinterpret_cast<uint8_t *>(malloc(count)),
				nullptr
			);
		else {
			if (alignment < alignof(std::max_align_t))
				alignment = alignof(std::max_align_t);

			auto a_shift = ctz(alignment);
			auto a_count = (count >> a_shift) << a_shift;
			if (a_count < count)
				a_count += alignment;

			pointer_access::assign(
				p, reinterpret_cast<uint8_t *>(
					::aligned_alloc(alignment, a_count)
				), nullptr
			);
		}

		if (!p)
			throw std::bad_alloc();

		return p;
	}

	void deallocate(
		storage_pointer p, size_type count,
		size_type alignment = alignof(std::max_align_t)
	) const
	{
		::free(pointer_access::address(p));
	}

	template <typename ElementType, typename... Args>
	pointer<ElementType> construct(storage_pointer p, Args&&... args) const
	{
		pointer<ElementType> q;

		auto qq(::new(pointer_access::address(p)) ElementType(
			std::forward<Args>(args)...
		));

		pointer_access::assign(
			q, reinterpret_cast<
				detail::memory_pointer_facade::address_type
			>(qq), nullptr
		);
		return q;
	}

	template <typename ElementType>
	void destroy(pointer<ElementType> p) const
	{
		auto pp(reinterpret_cast<ElementType *>(
			pointer_access::address(p)
		));
		pp->~ElementType();
	}

private:
	typedef detail::basic_pointer_access<
		detail::memory_pointer_facade,
		detail::memory_pointer_facade::is_stateful
	> pointer_access;
};

}}}
#endif

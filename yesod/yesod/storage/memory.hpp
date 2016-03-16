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

struct memory : detail::memory_pointer_facade {
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
	storage_pointer storage_cast(pointer<ElementType> p)
	{
		storage_pointer rv;
		pointer_access::assign(rv, pointer_access::address(p), this);
		return rv;
	}

	template <typename ElementType>
	pointer<ElementType> storage_cast(storage_pointer p)
	{
		pointer<ElementType> rv;
		pointer_access::assign(rv, pointer_access::address(p), this);
		return rv;
	}

	storage_pointer allocate(
		size_type count,
		size_type alignment = alignof(std::max_align_t)
	)
	{
		storage_pointer p;
		pointer_access::assign(p, nullptr, this);

		if (alignment == 1)
			pointer_access::assign(
				p, reinterpret_cast<uint8_t *>(malloc(count)),
				this
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
					aligned_alloc(alignment, a_count)
				), this
			);
		}

		if (!p)
			throw std::bad_alloc();

		return p;
	}

	void deallocate(
		storage_pointer p, size_type count,
		size_type alignment = alignof(std::max_align_t)
	)
	{
		free(pointer_access::address(p));
	}

	template <typename ElementType, typename... Args>
	pointer<ElementType> construct(storage_pointer p, Args&&... args)
	{
		pointer<ElementType> q;

		auto qq(::new(pointer_access::address(p)) ElementType(
			std::forward<Args>(args)...
		));

		pointer_access::assign(
			q, reinterpret_cast<
				detail::memory_pointer_facade::address_type
			>(qq), this
		);
		return q;
	}

	template <typename ElementType>
	void destroy(pointer<ElementType> p)
	{
		auto pp(reinterpret_cast<ElementType *>(
			pointer_access::address(p)
		));
		pp->~ElementType();
	}

private:
	typedef detail::basic_pointer_access<
		memory_pointer_facade, memory_pointer_facade::is_stateful
	> pointer_access;
};

}}}
#endif

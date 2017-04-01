/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_BC0D6FE1972D50F19A302F3C53C4B597)
#define HPP_BC0D6FE1972D50F19A302F3C53C4B597

#include <memory>

namespace ucpf::yesod::detail {

template <typename ValueType>
struct allocator_helper {
	typedef ValueType value_type;
	typedef size_t size_type;
	typedef std::aligned_storage_t<
		sizeof(value_type), alignof(value_type)
	> storage_type;

	template <typename Allocator>
	struct traits {
		typedef Allocator allocator_type;
		typedef typename std::allocator_traits<
			allocator_type
		>::template rebind_traits<storage_type> storage;
		typedef typename std::allocator_traits<
			allocator_type
		>::template rebind_traits<value_type> value;

		static typename storage::allocator_type to_storage(
			Allocator const &alloc
		)
		{
			return typename storage::allocator_type(alloc);
		}

		static typename storage::allocator_type to_value(
			Allocator const &alloc
		)
		{
			return typename value::allocator_type(alloc);
		}
	};

	template <typename Allocator>
	static storage_type *allocate_n(
		Allocator const &alloc, size_type count
	)
	{
		return traits<Allocator>::to_storage(alloc).allocate(count);
	}

	template <typename Allocator, typename... Args>
	static value_type *construct_n(
		Allocator const &alloc, storage_type *storage,
		size_type count, Args &&...args
	)
	{
		auto value_alloc(traits<Allocator>::to_value(alloc));
		size_t data_pos(0);
		auto deleter([
			&data_pos, count, &value_alloc
		](value_type *p) {
			if (!data_pos)
				return;

			do {
				--data_pos;
				value_alloc.destroy(p + data_pos);
			} while (data_pos);
		});

		std::unique_ptr<value_type, decltype(deleter)> p(
			reinterpret_cast<value_type *>(storage),
			deleter
		);

		for (; data_pos < count; ++data_pos)
			value_alloc.construct(
				p.get() + data_pos,
				std::forward<Args>(args)...
			);

		return p.release();
	}

	template <typename Allocator, typename... Args>
	static value_type *claim_n(
		Allocator const &alloc, size_type count, Args &&...args
	)
	{
		auto deleter([count, &alloc](storage_type *p) {
			deallocate_n(alloc, p, count);
			
		});
		std::unique_ptr<storage_type, decltype(deleter)> p(
			allocate_n(alloc, count), deleter
		);

		auto rv(construct_n(
			alloc, p.get(), count, std::forward<Args>(args)...
		));

		p.release();
		return rv;
	}

	template <typename Allocator>
	static void relinquish_n(
		Allocator const &alloc, value_type *p, size_type count
	)
	{
		deallocate_n(alloc, destroy_n(alloc, p, count), count);
	}

	template <typename Allocator>
	static storage_type *destroy_n(
		Allocator const &alloc, value_type *p, size_type count
	)
	{
		if (!count)
			return reinterpret_cast<storage_type *>(p);

		auto value_alloc(traits<Allocator>::to_value(alloc));

		do {
			--count;
			value_alloc.destroy(p + count);
		} while (count);

		return reinterpret_cast<storage_type *>(p);
	}

	template <typename Allocator>
	static void deallocate_n(
		Allocator const &alloc, storage_type *p, size_type count
	)
	{
		return traits<Allocator>::to_storage(alloc).deallocate(
			p, count
		);
	}
};

}
#endif


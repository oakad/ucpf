/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_JAN_15_2014_1150)
#define UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_JAN_15_2014_1150

#include <array>
#include <tuple>
#include <bitset>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename ValueType, size_t N, bool IsPodArray,
	typename Alloc = std::allocator<void>,
	typename StoredType = typename std::aligned_storage<
		sizeof(ValueType), std::alignment_of<ValueType>::value
	>::type
> struct placement_array {
	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		placement_array
	> self_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		placement_array
	> self_allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		ValueType
	> allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		ValueType
	> allocator_traits;

	typedef ValueType value_type;
	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::pointer pointer;
	typedef typename allocator_traits::const_pointer const_pointer;
	typedef typename allocator_traits::void_pointer void_pointer;
	typedef typename allocator_traits::size_type size_type;

	typedef pointer iterator;
	typedef const_pointer const_iterator;

	template <typename Alloc1>
	static placement_array *construct(Alloc1 const &a_)
	{
		self_allocator_type a(a_);
		auto p(self_allocator_traits::allocate(a, 1));
		self_allocator_traits::construct(a, p, a_);
		return p;
	}

	template <typename Alloc1>
	static void destroy(Alloc1 const &a_, placement_array *p)
	{
		p->destroy(a_);
		self_allocator_type a(a_);
		self_allocator_traits::destroy(a, p);
		self_allocator_traits::deallocate(a, p, 1);
	}

	template <typename Alloc1>
	placement_array(Alloc1 const &a_)
	{
		if (IsPodArray) {
			allocator_type a(a_);
			for (auto &p: std::get<0>(items))
				allocator_traits::construct(
					a, reinterpret_cast<pointer>(&p)
				);
		}
	}

	reference operator[](size_type pos)
	{
		return reinterpret_cast<reference>(std::get<0>(items)[pos]);
	}

	const_reference operator[](size_type pos) const
	{
		return reinterpret_cast<const_reference>(
			std::get<0>(items)[pos]
		);
	}

	iterator begin()
	{
		return iterator(&(*this)[0]);
	}

	const_iterator cbegin() const
	{
		return const_iterator(&(*this)[0]);
	}

	iterator end()
	{
		return iterator(&(*this)[N]);
	}

	const_iterator cend() const
	{
		return const_iterator(&(*this)[N]);
	}

	pointer ptr_at(size_type pos)
	{
		auto p(&(*this)[pos]);

		if (!IsPodArray) {
			if (!std::get<1>(items).test(pos))
				return nullptr;
		}

		return p;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto p(&(*this)[pos]);

		if (!IsPodArray) {
			if (!std::get<1>(items).test(pos))
				return nullptr;
		}

		return p;
	}

	template <typename Alloc1>
	reference at(size_type pos, Alloc1 const &a_)
	{
		auto p(&(*this)[pos]);

		if (!IsPodArray) {
			if (!std::get<1>(items).test(pos)) {
				allocator_type a(a_);
				allocator_traits::construct(a, p);
				std::get<1>(items).set(pos);
			}
		}

		return *p;
	}

	template <typename Alloc1, typename... Args>
	reference emplace_at(size_type pos, Alloc1 const &a_, Args&&... args)
	{
		auto p(&(*this)[pos]);
		allocator_type a(a_);

		if (IsPodArray) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			return *p;
		}

		if (!std::get<1>(items).test(pos)) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			std::get<1>(items).set(pos);
			return *p;
		}

		int restore_mode(0);
		std::function<void (value_type *)> backup_deleter(
			[&a, &restore_mode, p](value_type *bp) {
				switch (restore_mode) {
				case 2:
					allocator_traits::construct(
						a, p, std::move(*bp)
					);
				case 1:
					allocator_traits::destroy(a, bp);
				case 0:
					allocator_traits::deallocate(a, bp, 1);
				}
			}
		);

		std::unique_ptr<value_type, decltype(backup_deleter)> bp(
			allocator_traits::allocate(a, 1), backup_deleter
		);

		allocator_traits::construct(a, bp.get(), std::move(*p));
		restore_mode = 1;

		allocator_traits::destroy(a, p);
		restore_mode = 2;

		allocator_traits::construct(a, p, std::forward<Args>(args)...);
		restore_mode = 1;

		return *p;
	}

	bool for_each(
		size_type offset,
		std::function<bool (size_type, const_reference)> &&f
	) const
	{
		for (
			size_type pos(0);
			pos < std::get<0>(items).size();
			++pos
		) {
			if (std::get<1>(items).test(pos)) {
				if (!f(pos + offset, (*this)[pos]))
					return false;
			}
		}
		return true;
	}

private:
	template <typename Alloc1>
	void destroy(Alloc1 const &a_)
	{
		if (!IsPodArray) {
			allocator_type a(a_);
			for (
				size_type pos(0);
				pos < std::get<0>(items).size();
				++pos
			) {
				if (std::get<1>(items).test(pos))
					allocator_traits::destroy(
						a, &(*this)[pos]
					);
			}
		}
	}

	struct dummy_bitset {
		bool test( size_t pos ) const
		{
			return true;
		}

		dummy_bitset &set()
		{
			return *this;
		}

		dummy_bitset &set(size_t pos, bool value = true)
		{
			return *this;
		}
	};

	std::tuple<
		std::array<StoredType, N>,
		typename std::conditional<
			IsPodArray, dummy_bitset, std::bitset<N>
		>::type
	> items;
};

}}}
#endif

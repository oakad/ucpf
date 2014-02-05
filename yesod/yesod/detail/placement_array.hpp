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
	typename ValueType, size_t N, typename Policy, typename Alloc,
	typename StoredType
> struct placement_array;

struct placement_array_pod_policy {
	struct metadata_type {};
	constexpr static bool needs_safe_initialization = false;

	template <typename ArrayType>
	static bool test_valid(ArrayType const &a, size_t pos)
	{
		return a[pos] != 0;
	}

	template <typename ArrayType>
	static void set_valid(ArrayType &a, size_t pos, bool valid)
	{}
};

template <size_t N>
struct placement_array_obj_policy {
	typedef std::bitset<N> metadata_type;
	constexpr static bool needs_safe_initialization = true;

	template <typename ArrayType>
	static bool test_valid(ArrayType const &a, size_t pos)
	{
		return a.get_metadata().test(pos);
	}

	template <typename ArrayType>
	static void set_valid(ArrayType &a, size_t pos, bool valid)
	{
		a.get_metadata().set(pos, valid);
	}
};

template <
	typename ValueType, size_t N,
	typename Policy = typename std::conditional<
		std::is_pod<ValueType>::value,
		placement_array_pod_policy,
		placement_array_obj_policy<N>
	>::type,
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
		if (!Policy::needs_safe_initialization) {
			allocator_type a(a_);
			for (auto &p: std::get<0>(items))
				allocator_traits::construct(
					a, reinterpret_cast<pointer>(&p)
				);
		}
	}

	typename Policy::metadata_type const &get_metadata() const
	{
		return std::get<1>(items);
	}

	typename Policy::metadata_type &get_metadata()
	{
		return std::get<1>(items);
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

	const_iterator begin() const
	{
		return const_iterator(&(*this)[0]);
	}

	const_iterator cbegin() const
	{
		return const_iterator(&(*this)[0]);
	}

	iterator end()
	{
		return iterator(&(*this)[N]);
	}

	const_iterator end() const
	{
		return const_iterator(&(*this)[N]);
	}

	const_iterator cend() const
	{
		return const_iterator(&(*this)[N]);
	}

	pointer ptr_at(size_type pos)
	{
		auto p(&(*this)[pos]);

		if (
			Policy::needs_safe_initialization
			&& !Policy::test_valid(*this, pos)
		)
			return nullptr;

		return p;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto p(&(*this)[pos]);

		if (
			Policy::needs_safe_initialization
			&& !Policy::test_valid(*this, pos)
		)
			return nullptr;

		return p;
	}

	template <typename Alloc1>
	reference at(size_type pos, Alloc1 const &a_)
	{
		auto p(&(*this)[pos]);

		if (Policy::needs_safe_initialization) {
			if (!Policy::test_valid(*this, pos)) {
				allocator_type a(a_);
				allocator_traits::construct(a, p);
				Policy::set_valid(*this, pos, true);
			}
		}

		return *p;
	}

	template <typename Alloc1, typename... Args>
	reference emplace_at(size_type pos, Alloc1 const &a_, Args&&... args)
	{
		auto p(&(*this)[pos]);
		allocator_type a(a_);

		if (!Policy::needs_safe_initialization) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			return *p;
		}

		if (!Policy::test_valid(*this, pos)) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			Policy::set_valid(*this, pos, true);
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

		allocator_traits::construct(
			a, bp.get(), std::move_if_noexcept(*p)
		);
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
			if (Policy::test_valid(*this, pos)) {
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
		if (Policy::needs_safe_initialization) {
			allocator_type a(a_);
			for (
				size_type pos(0);
				pos < std::get<0>(items).size();
				++pos
			) {
				if (Policy::test_valid(*this, pos))
					allocator_traits::destroy(
						a, &(*this)[pos]
					);
			}
		}
	}

	std::tuple<
		std::array<StoredType, N>, typename Policy::metadata_type
	> items;
};

}}}
#endif

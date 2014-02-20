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

template <size_t N, typename ValueValidPred = void, bool IsPodType = false>
struct placement_array_base {
	bool needs_safe_initialization() const
	{
		return true;
	}

	template <typename T>
	bool test_valid(T const *v, size_t pos) const
	{
		return value_valid.test(pos);
	}

	template <typename T>
	void set_valid(T const *v, size_t pos, bool valid)
	{
		value_valid.set(pos, valid);
	}

	std::bitset<N> value_valid;
};

template <size_t N, typename ValueValidPred>
struct placement_array_base<N, ValueValidPred, true> {
	bool needs_safe_initialization() const
	{
		return false;
	}

	template <typename T>
	bool test_valid(T const *v, size_t pos) const
	{
		return ValueValidPred::test(*v);
	}

	template <typename T>
	void set_valid(T const *v, size_t pos, bool valid)
	{}
};

template <size_t N>
struct placement_array_base<N, void, true> {
	bool needs_safe_initialization() const
	{
		return false;
	}

	template <typename T>
	bool test_valid(T const *v, size_t pos) const
	{
		return *v != 0;
	}

	template <typename T>
	void set_valid(T const *v, size_t pos, bool valid)
	{}
};

template <
	typename ValueType, size_t N,
	typename Alloc = std::allocator<void>,
	typename ValueValidPred = void,
	typename StoredType = typename std::aligned_storage<
		sizeof(ValueType), std::alignment_of<ValueType>::value
	>::type
> struct placement_array : placement_array_base<
	N, ValueValidPred, std::is_pod<ValueType>::value
> {
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
		if (!this->needs_safe_initialization()) {
			allocator_type a(a_);
			for (auto &p: items)
				allocator_traits::construct(
					a, reinterpret_cast<pointer>(&p)
				);
		}
	}

	reference operator[](size_type pos)
	{
		return reinterpret_cast<reference>(items[pos]);
	}

	const_reference operator[](size_type pos) const
	{
		return reinterpret_cast<const_reference>(items[pos]);
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

	size_type size() const
	{
		return items.size();
	}

	pointer ptr_at(size_type pos)
	{
		auto p(&(*this)[pos]);

		if (!this->test_valid(p, pos))
			return nullptr;

		return p;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto p(&(*this)[pos]);

		if (!this->test_valid(p, pos))
			return nullptr;

		return p;
	}

	template <typename Alloc1>
	reference at(size_type pos, Alloc1 const &a_)
	{
		auto p(&(*this)[pos]);

		if (this->needs_safe_initialization()) {
			if (!this->test_valid(p, pos)) {
				allocator_type a(a_);
				allocator_traits::construct(a, p);
				this->set_valid(p, pos, true);
			}
		}

		return *p;
	}

	template <typename Alloc1, typename... Args>
	reference emplace_at(size_type pos, Alloc1 const &a_, Args&&... args)
	{
		auto p(&(*this)[pos]);
		allocator_type a(a_);

		if (!this->needs_safe_initialization()) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			return *p;
		}

		if (!this->test_valid(p, pos)) {
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			this->set_valid(p, pos, true);
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

	size_type find_empty_above(size_type pos) const
	{
		for (; pos < items.size(); ++pos) {
			if (!this->test_valid(&(*this)[pos], pos))
				break;
		}
		return pos;
	}

	bool for_each_above(
		size_type pos,
		std::function<bool (size_type, reference)> &&f,
		size_type base_offset = 0
	)
	{
		for (; pos < items.size(); ++pos) {
			if (this->test_valid(&(*this)[pos], pos)) {
				if (!f(pos + base_offset, (*this)[pos]))
					return false;
			}
		}
		return true;
	}

	bool for_each_above(
		size_type pos,
		std::function<bool (size_type, const_reference)> &&f,
		size_type base_offset = 0
	) const
	{
		for (; pos < items.size(); ++pos) {
			if (this->test_valid(&(*this)[pos], pos)) {
				if (!f(pos + base_offset, (*this)[pos]))
					return false;
			}
		}
		return true;
	}

private:
	template <typename Alloc1>
	void destroy(Alloc1 const &a_)
	{
		if (this->needs_safe_initialization()) {
			allocator_type a(a_);
			for (size_type pos(0); pos < items.size(); ++pos) {
				if (this->test_valid(&(*this)[pos], pos))
					allocator_traits::destroy(
						a, &(*this)[pos]
					);
			}
		}
	}

	std::array<StoredType, N> items;
};

}}}
#endif

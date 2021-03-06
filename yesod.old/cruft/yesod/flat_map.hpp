/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_FLAT_MAP_20140411T1915)
#define UCPF_YESOD_FLAT_MAP_20140411T1915

#include <yesod/detail/flat_map_base.hpp>
#include <yesod/detail/flat_map_ops.hpp>
#include <yesod/allocator/policy/fibonacci.hpp>

namespace ucpf { namespace yesod {

template <
	typename KeyType, typename T,
	typename CompareF = std::less<KeyType>,
	typename Alloc = std::allocator<void>
> struct flat_map {
	typedef KeyType key_type;
	typedef T mapped_type;
	typedef std::tuple<key_type, mapped_type> value_type;

private:
	struct extract_key {
		typedef flat_map::value_type value_type;
		typedef flat_map::key_type result_type;

		static key_type const &apply(value_type const &p)
		{
			return std::get<0>(p);
		}
	};

	typedef detail::flat_map_impl<
		extract_key, CompareF, Alloc, allocator::policy::fibonacci
	> map_impl_type;

public:
	typedef typename map_impl_type::const_iterator const_iterator;
	typedef typename map_impl_type::iterator iterator;
	typedef typename map_impl_type::allocator_type allocator_type;
	typedef typename map_impl_type::size_type size_type;

	flat_map()
	: flat_map(CompareF())
	{}

	explicit flat_map(Alloc const &a)
	: flat_map(CompareF(), a)
	{}

	explicit flat_map(CompareF const &comp, Alloc const &a = Alloc())
	: impl(comp, a)
	{}

	template <typename Iterator>
	flat_map(
		Iterator first, Iterator last,
		Alloc const &a = Alloc()
	) : flat_map(first, last, CompareF(), a)
	{}

	flat_map(
		std::initializer_list<value_type> init,
		CompareF const &comp = CompareF(),
		Alloc const &a = Alloc()
	) : flat_map(init.begin(), init.end(), comp, a)
	{}

	template <typename Iterator>
	flat_map(
		Iterator first, Iterator last,
		CompareF const &comp = CompareF(),
		Alloc const &a = Alloc()
	) : impl(comp, a)
	{
		insert(first, last);
	}

	flat_map(flat_map const &other)
	: flat_map(other, Alloc())
	{}

	flat_map(flat_map const &other, Alloc const &a)
	: impl(other.impl, a)
	{}

	flat_map(flat_map &&other)
	: flat_map(std::forward<flat_map>(other), Alloc())
	{}

	flat_map(flat_map &&other, Alloc const &a)
	: impl(std::move(other.impl), a)
	{}

	flat_map &operator=(flat_map const &other )
	{
		impl = other.impl;
		return *this;
	}

	flat_map &operator=(flat_map &&other)
	{
		impl = std::move(other.impl);
		return *this;
	}

	flat_map& operator=(std::initializer_list<value_type> init)
	{
		insert(init);
		return *this;
	}

	allocator_type get_allocator() const
	{
		return impl.get_allocator();
	}

	mapped_type &at(key_type const &key);
	mapped_type const &at(key_type const &key) const;

	mapped_type &operator[](key_type const &key)
	{
		auto iter_p(find_impl(key));
		if (!iter_p.second)
			iter_p = impl.emplace_unique(
				iter_p.first, value_type(key, mapped_type{})
			);

		return std::get<1>(*iter_p.first);
	}

	mapped_type &operator[](key_type &&key)
	{
		auto iter_p(find_impl(key));
		if (!iter_p.second)
			iter_p = impl.emplace_unique(
				iter_p.first, value_type(key, mapped_type{})
			);

		return std::get<1>(*iter_p.first);
	}

	iterator begin()
	{
		return impl.begin();
	}

	const_iterator begin() const
	{
		return cbegin();
	}

	const_iterator cbegin() const
	{
		return impl.cbegin();
	}

	iterator end()
	{
		return impl.end();
	}

	const_iterator end() const
	{
		return cend();
	}

	const_iterator cend() const
	{
		return impl.cend();
	}

	bool empty() const
	{
		return impl.empty();
	}

	size_type size() const
	{
		return impl.size();
	}

	void clear()
	{
		impl.clear();
	}

	std::pair<iterator, bool> insert(value_type const &value)
	{
		auto iter_p(find_impl(extract_key::apply(value)));

		if (iter_p.second) {
			iter_p.second = false;
			return iter_p;
		}

		return impl.emplace_unique(iter_p.first, value);
	}

	iterator insert(const_iterator hint, value_type const &value)
	{
		return impl.emplace_unique(hint, value);
	}

	template <typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		auto iter_p(std::make_pair(cend(), false));;

		for (; first != last; ++first)
			iter_p = impl.emplace_unique(iter_p.first, *first);

		printf("ddd\n");
	}

	void insert(std::initializer_list<value_type> init)
	{
		insert(init.begin(), init.end());
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		return impl.emplace_unique(
			impl.cend(), std::forward<Args>(args)...
		);
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace_hint(
		const_iterator hint, Args&&... args
	)
	{
		return impl.emplace_unique(hint, std::forward<Args>(args)...);
	}

	iterator erase(const_iterator pos)
	{
		return impl.erase(pos, pos + 1);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		return impl.erase(first, last);
	}

	size_type erase(key_type const &key)
	{
		auto p(find(key));
		if (p != end()) {
			erase(p, p + 1);
			return 1;
		} else
			return 0;
	}

	void swap(flat_map &other)
	{
		impl.swap(other.impl);
	}

	iterator find(key_type const &key)
	{
		auto iter_p(find_impl(key));
		return iter_p.second ? iter_p.first : end();
	}

	const_iterator find(key_type const &key) const
	{
		auto iter_p(find_impl(key));
		return iter_p.second ? iter_p.first : cend();
	}

	iterator lower_bound(key_type const &key)
	{
		return impl.lower_bound(key);
	}

	const_iterator lower_bound(key_type const &key) const
	{
		return impl.lower_bound(key);
	}

	iterator upper_bound(key_type const &key)
	{
		return impl.upper_bound(key);
	}

	const_iterator upper_bound(key_type const &key) const
	{
		return impl.upper_bound(key);
	}

private:
	std::pair<iterator, bool> find_impl(key_type const &key)
	{
		auto iter(impl.lower_bound(key));
		if (iter != end()) {
			if (!impl.key_comp()(key, std::get<0>(*iter)))
				return std::make_pair(iter, true);
		}
		return std::make_pair(iter, false);
	}

	std::pair<const_iterator, bool> find_impl(key_type const &key) const
	{
		auto iter(impl.lower_bound(key));
		if (iter != cend()) {
			if (!impl.key_comp()(key, std::get<0>(*iter)))
				return std::make_pair(iter, true);
		}
		return std::make_pair(iter, false);
	}

	map_impl_type impl;
};

}}
#endif

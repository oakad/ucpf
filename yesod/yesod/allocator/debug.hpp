/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ALLOCATOR_DEBUG_20141023T2300)
#define UCPF_YESOD_ALLOCATOR_DEBUG_20141023T2300

#include <string>
#include <typeinfo>
#include <cxxabi.h>
#include <type_traits>

namespace ucpf { namespace yesod { namespace allocator {

template <typename T>
struct debug;

template <>
struct debug<void> {
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void *pointer;
	typedef void const *const_pointer;
	typedef void value_type;

	template<typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_move_assignment;

	debug() noexcept
	: instance_id(random())
	{
		set_type_name();
	}

	debug(debug const &a) noexcept
	: instance_id(a.instance_id)
	{
		set_type_name();
	}

	template <typename U>
	debug(debug<U> const &a) noexcept
	: instance_id(a.instance_id)
	{
		set_type_name();
	}

	~debug() noexcept
	{}

private:
	template <typename U>
	friend struct debug;

	void set_type_name()
	{
		if (!value_type_name.empty())
			return;

		value_type_name.assign("void");
	}

	size_type instance_id;
	static std::string value_type_name;
};

std::string debug<void>::value_type_name;

template <typename T>
struct debug {
	typedef std::size_t size_type;
	typedef ptrdiff_t  difference_type;
	typedef T *pointer;
	typedef T const *const_pointer;
	typedef T &reference;
	typedef T const &const_reference;
	typedef T value_type;

	template <typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_move_assignment;

	debug() noexcept
	: instance_id(random())
	{
		set_type_name();
	}

	debug(debug const &a) noexcept
	: instance_id(a.instance_id)
	{
		set_type_name();
	}

	template <typename U>
	debug(debug<U> const &a) noexcept
	: instance_id(a.instance_id)
	{
		set_type_name();
	}

	~debug() noexcept
	{}

	pointer address(reference x) const noexcept
	{
		return std::addressof(x);
	}

	const_pointer address(const_reference x) const noexcept
	{
		return std::addressof(x);
	}

	pointer allocate(size_type n, void const * = nullptr)
	{
		auto p(static_cast<pointer>(
			::operator new(n * sizeof(value_type))
		));

		printf(
			"allocator::debug<%s> (%zd): allocate %p (%zd)\n",
			value_type_name.c_str(), instance_id, p, n
		);

		return p;
	}

	void deallocate(pointer p, size_type n)
	{
		printf(
			"allocator::debug<%s> (%zd): deallocate %p (%zd)\n",
			value_type_name.c_str(), instance_id, p, n
		);

		::operator delete(p);
	}

	size_type max_size() const noexcept
	{
		return size_type(~0) / sizeof(value_type);
	}

	template <typename... Args>
	void construct(pointer p, Args&&... args)
	{
		printf(
			"allocator::debug<%s> (%zd): construct %p\n",
			value_type_name.c_str(), instance_id, p
		);

		::new(reinterpret_cast<void *>(p)) value_type(
			std::forward<Args>(args)...
		);
	}

	void destroy(pointer p)
	{
		printf(
			"allocator::debug<%s> (%zd): destroy %p\n",
			value_type_name.c_str(), instance_id, p
		);
		fflush(stdout);
		p->~value_type();
	}

private:
	template <typename U>
	friend struct debug;

	template <typename U>
	friend bool operator==(debug<U> const &a0, debug<U> const &a1);

	template <typename U>
	friend bool operator!=(debug<U> const &a0, debug<U> const &a1);

	void set_type_name()
	{
		if (!value_type_name.empty())
			return;

		auto *s(abi::__cxa_demangle(
			typeid(value_type).name(), 0, 0, 0
		));

		value_type_name.assign(s);

		free(s);
	}

	size_type instance_id;
	static std::string value_type_name;
};

template <typename T>
std::string debug<T>::value_type_name;

template <typename T>
bool operator==(debug<T> const &a0, debug<T> const &a1)
{
	return a0.instance_id == a1.instance_id;
}

template <typename T>
bool operator!=(debug<T> const &a0, debug<T> const &a1)
{
	return a0.instance_id != a1.instance_id;
}

}}}
#endif

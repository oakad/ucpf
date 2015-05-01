/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_FCB600647E3379FFE866C1CC907B2908)
#define HPP_FCB600647E3379FFE866C1CC907B2908

#include <map>
#include <string>
#include <typeinfo>
#include <cxxabi.h>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

namespace ucpf { namespace yesod { namespace allocator {
namespace detail {

template <typename Dummy = void>
struct debug_base {
	template <typename T>
	void on_alloc(
		std::size_t instance_id, std::size_t count
	)
	{
		stats.alloc_map[instance_id][
			std::type_index(typeid(T))
		].allocated += count;
	}

	template <typename T>
	void on_free(
		std::size_t instance_id, std::size_t count
	)
	{
		stats.alloc_map[instance_id][
			std::type_index(typeid(T))
		].freed += count;
	}

	template <typename T>
	void register_type(std::size_t instance_id)
	{
		stats.alloc_map[instance_id][
			std::type_index(typeid(T))
		].unit_size = sizeof(T);
	}

	std::size_t extant_bytes() const
	{
		std::size_t rv(0);

		for (auto const &m: stats.alloc_map) {
			for (auto const &n: m.second) {
				auto c(
					n.second.balance() * n.second.unit_size
				);
				rv += c >= 0 ? c : -c;
			}
		}
		return rv;
	}

	static struct stats_holder {
		~stats_holder()
		{
			for (auto const &m: alloc_map) {
				fprintf(stderr, "inst: %zd\n", m.first);
				for (auto const &n: m.second) {
					auto *s(abi::__cxa_demangle(
						n.first.name(),
						0, 0, 0
					));

					fprintf(
						stderr,
						"\t%s: out: %zd, in: %zd "
						"total: %zd\n",
						s,
						n.second.allocated,
						n.second.freed,
						n.second.balance()
					);

					free(s);
				}
			}
		}

		struct debug_stats {
			long balance() const
			{
				return allocated >= freed
				       ? long(allocated - freed)
				       : -long(freed - allocated);
			}

			std::size_t unit_size = 0;
			std::size_t allocated = 0;
			std::size_t freed = 0;
		};

		std::map<
			std::size_t,
			std::unordered_map<std::type_index, debug_stats>
		> alloc_map;
	} stats;
};

template <typename Dummy>
typename debug_base<Dummy>::stats_holder debug_base<Dummy>::stats;

}

template <typename T>
struct debug;

template <>
struct debug<void> : detail::debug_base<> {
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void *pointer;
	typedef void const *const_pointer;
	typedef void value_type;

	template<typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

	debug() noexcept
	: instance_id(random())
	{}

	debug(debug const &a) noexcept
	: instance_id(a.instance_id)
	{}

	template <typename U>
	debug(debug<U> const &a) noexcept
	: instance_id(a.instance_id)
	{}

	~debug() noexcept
	{}

private:
	template <typename U>
	friend struct debug;

	size_type instance_id;
};

template <typename T>
struct debug : detail::debug_base<> {
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

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

	debug() noexcept
	: instance_id(random())
	{
		register_type<value_type>(instance_id);
	}

	debug(debug const &a) noexcept
	: instance_id(a.instance_id)
	{
		register_type<value_type>(instance_id);	
	}

	template <typename U>
	debug(debug<U> const &a) noexcept
	: instance_id(a.instance_id)
	{
		register_type<value_type>(instance_id);
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
		on_alloc<value_type>(instance_id, n);
		auto p(static_cast<pointer>(
			::operator new(n * sizeof(value_type))
		));

	
		return p;
	}

	void deallocate(pointer p, size_type n)
	{
		on_free<value_type>(instance_id, n);
		::operator delete(p);
	}

	size_type max_size() const noexcept
	{
		return size_type(~0) / sizeof(value_type);
	}

	template <typename... Args>
	void construct(pointer p, Args&&... args)
	{	
		::new(reinterpret_cast<void *>(p)) value_type(
			std::forward<Args>(args)...
		);
	}

	void destroy(pointer p)
	{
		p->~value_type();
	}

private:
	template <typename U>
	friend struct debug;

	template <typename U>
	friend bool operator==(debug<U> const &a0, debug<U> const &a1);

	template <typename U>
	friend bool operator!=(debug<U> const &a0, debug<U> const &a1);

	size_type instance_id;
};

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

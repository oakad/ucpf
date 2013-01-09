/*
 * Copyright (C) 2010, 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_INTERNAL_COUNTED_PTR_BASE_HPP)
#define _RASHAM_INTERNAL_COUNTED_PTR_BASE_HPP

#include <cstddef>
#include <atomic>
#include <tuple>
#include <typeinfo>

namespace rasham
{

struct counted_base {
	counted_base(counted_base const &) = delete;
	counted_base &operator=(counted_base const &) = delete;

	counted_base()
	: use_count(1UL)
	{}

	virtual ~counted_base()
	{}

	virtual void add_ref_copy()
	{
		use_count.fetch_add(1UL);
	}

	virtual void release()
	{
		auto rv(use_count.fetch_sub(1UL));
		if (rv == 1)
			destroy();
	}

	unsigned long get_use_count() const
	{
		return use_count.load();
	}

	virtual void destroy() = 0;

private:
	std::atomic_ulong use_count;
};

template <typename value_type>
struct ref_count : public counted_base {
	virtual ~ref_count()
	{}

	virtual void *get_allocator(std::type_info const &ti)
	{
		return 0;
	}

	virtual value_type *get_ptr() = 0;

	virtual value_type const *get_ptr() const = 0;

	unsigned long get_use_count() const
	{
		return counted_base::get_use_count();
	}

	bool unique() const
	{
		return this->get_use_count() == 1;
	}

	virtual void *get_extra()
	{
		return 0;
	}

	virtual void const *get_extra() const
	{
		return 0;
	}

	virtual void *get_extra(size_t &sz)
	{
		sz = 0;
		return 0;
	}

	virtual void const *get_extra(size_t &sz) const
	{
		sz = 0;
		return 0;
	}
};

template <typename value_type>
struct ref_count_val {
	static ref_count<value_type> *get_this(value_type *p)
	{
		return *reinterpret_cast<ref_count<value_type> **>(
			reinterpret_cast<char *>(p)
			- sizeof(ref_count<value_type> *)
		);
	}

	template <typename... arg_type>
	ref_count_val(ref_count<value_type> *count_, arg_type&&... args)
	: count(count_), val(std::forward<arg_type>(args)...)
	{}

	value_type *get_ptr()
	{
		return &val;
	}

	value_type const *get_ptr() const
	{
		return &val;
	}

private:
	ref_count<value_type> *count;
	value_type            val;
};

template <typename value_type>
struct ref_count_p : public ref_count<value_type> {
	template <typename... arg_type>
	static ref_count<value_type> *create(arg_type&&... args)
	{
		return ::new ref_count_p(std::forward<arg_type>(args)...);
	}

	virtual ~ref_count_p()
	{}

	virtual void destroy()
	{
		::delete this;
	}

	virtual value_type *get_ptr()
	{
		return val.get_ptr();
	}

	virtual value_type const *get_ptr() const
	{
		return val.get_ptr();
	}

private:
	template <typename... arg_type>
	ref_count_p(arg_type&&... args)
	: val(this, std::forward<arg_type>(args)...)
	{}

	ref_count_val<value_type> val;
};

template <typename value_type>
struct ref_count_e : public ref_count<value_type> {
	template <typename... arg_type>
	static ref_count<value_type> *create(size_t extra_size_,
					     arg_type&&... args)
	{
		void *p(::operator new(sizeof(ref_count_e<value_type>)
				       + extra_size_));

		try {
			return ::new (p) ref_count_e(
				extra_size_, std::forward<arg_type>(args)...
			);
		} catch (...) {
			::operator delete(p);
			throw;
		}
	}

	virtual ~ref_count_e()
	{}

	virtual void destroy()
	{
		void *p(this);
		this->~ref_count_e<value_type>();

		::operator delete(p);
	}

	virtual value_type *get_ptr()
	{
		return val.get_ptr();
	}

	virtual value_type const *get_ptr() const
	{
		return val.get_ptr();
	}

	virtual void *get_extra()
	{
		return extra_storage;
	}

	virtual void const *get_extra() const
	{
		return extra_storage;
	}

	virtual void *get_extra(size_t &sz)
	{
		sz = extra_size;
		return extra_storage;
	}

	virtual void const *get_extra(size_t &sz) const
	{
		sz = extra_size;
		return extra_storage;
	}

private:
	template <typename... arg_type>
	ref_count_e(size_t extra_size_, arg_type&&... args)
	: extra_size(extra_size_), val(this, std::forward<arg_type>(args)...)
	{}

	size_t                    extra_size;
	ref_count_val<value_type> val;
	char                      extra_storage[];
};

template <typename value_type, typename alloc_type>
struct ref_count_a : public ref_count<value_type> {
	template <typename... arg_type>
	static ref_count<value_type> *create(alloc_type a, arg_type&&... args)
	{
		typename alloc_type::template rebind<this_type>::other a2(a);

		ref_count_a *p(a2.allocate(1));

		try {
			return ::new (p) ref_count_a(
				std::forward<alloc_type>(a),
				std::forward<arg_type>(args)...
			);
		} catch(...) {
			a2.deallocate(p, 1);
			throw;
		}
	}

	virtual ~ref_count_a()
	{}

	virtual void destroy()
	{
		typename alloc_type::template rebind<this_type>::other
		a(std::get<1>(val_plus));

		a.destroy(this);
		a.deallocate(this, 1);
	}

	virtual void *get_allocator(std::type_info const &ti)
	{
		if (ti == typeid(alloc_type))
			return &std::get<1>(val_plus);
		else
			return 0;
	}

	virtual value_type *get_ptr()
	{
		return std::get<0>(val_plus).get_ptr();
	}

	virtual value_type const *get_ptr() const
	{
		return std::get<0>(val_plus).get_ptr();
	}

private:
	typedef ref_count_a<value_type, alloc_type> this_type;

	template <typename... arg_type>
	ref_count_a(alloc_type a, arg_type&&... args)
	: val_plus(
		ref_count_val<value_type>(
			this, std::forward<arg_type>(args)...
		),
		std::forward<alloc_type>(a)
	)
	{}

	std::tuple<ref_count_val<value_type>, alloc_type> val_plus;
};

template <typename value_type, typename alloc_type>
struct ref_count_a_e : public ref_count<value_type> {
	template <typename... arg_type>
	static ref_count<value_type> *create(alloc_type a, size_t extra_size,
					     arg_type&&... args)
	{
		size_t sz(sizeof(this_type) + extra_size);
		typename alloc_type::template rebind<char>::other
		raw(a);

		void *p(raw.allocate(sz));

		try {
			return ::new (p) ref_count_a_e(
				std::forward<alloc_type>(a), extra_size,
				std::forward<arg_type>(args)...
			);
		} catch (...) {
			raw.deallocate(reinterpret_cast<char *>(p), sz);
			throw;
		}
	}

	virtual ~ref_count_a_e()
	{}

	virtual void destroy()
	{
		size_t sz(sizeof(this_type) + std::get<0>(alloc_plus));
		typename alloc_type::template rebind<char>::other
		raw(std::get<1>(alloc_plus));

		this->~ref_count_a_e();
		raw.deallocate(reinterpret_cast<char *>(this), sz);
	}

	virtual void *get_allocator(std::type_info const &ti)
	{
		if (ti == typeid(alloc_type))
			 return &std::get<1>(alloc_plus);
		else
			return 0;
	}

	virtual value_type *get_ptr()
	{
		return val.get_ptr();
	}

	virtual value_type const *get_ptr() const
	{
		return val.get_ptr();
	}

	virtual void *get_extra()
	{
		return extra_storage;
	}

	virtual void const *get_extra() const
	{
		return extra_storage;
	}

	virtual void *get_extra(size_t &sz)
	{
		sz = std::get<0>(alloc_plus);
		return extra_storage;
	}

	virtual void const *get_extra(size_t &sz) const
	{
		sz = std::get<0>(alloc_plus);
		return extra_storage;
	}

private:
	typedef ref_count_a_e<value_type, alloc_type> this_type;

	template <typename... arg_type>
	ref_count_a_e(alloc_type a, size_t extra_size, arg_type&&... args)
	: alloc_plus(extra_size, std::forward<alloc_type>(a)),
	  val(this, std::forward<arg_type>(args)...)
	{}

	std::tuple<size_t, alloc_type> alloc_plus;
	ref_count_val<value_type>      val;
	char                           extra_storage[];
};

}
#endif

/*
 * Copyright (c) 2010 - 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800)
#define UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800

#include <cstddef>
#include <atomic>
#include <tuple>
#include <typeinfo>

namespace ucpf { namespace yesod { namespace detail {

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

template <typename ValueType>
struct ref_count : public counted_base {
	virtual ~ref_count()
	{}

	virtual void *get_allocator(std::type_info const &ti)
	{
		return 0;
	}

	virtual ValueType *get_ptr() = 0;

	virtual ValueType const *get_ptr() const = 0;

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

template <typename ValueType>
struct ref_count_val {
	static ref_count<ValueType> *get_this(ValueType *p)
	{
		return *reinterpret_cast<ref_count<ValueType> **>(
			reinterpret_cast<char *>(p)
			- sizeof(ref_count<ValueType> *)
		);
	}

	template <typename... arg_type>
	ref_count_val(ref_count<ValueType> *count_, arg_type&&... args)
	: count(count_), val(std::forward<arg_type>(args)...)
	{}

	ValueType *get_ptr()
	{
		return &val;
	}

	ValueType const *get_ptr() const
	{
		return &val;
	}

private:
	ref_count<ValueType> *count;
	ValueType            val;
};

template <typename ValueType>
struct ref_count_p : public ref_count<ValueType> {
	template <typename... arg_type>
	static ref_count<ValueType> *create(arg_type&&... args)
	{
		return ::new ref_count_p(std::forward<arg_type>(args)...);
	}

	virtual ~ref_count_p()
	{}

	virtual void destroy()
	{
		::delete this;
	}

	virtual ValueType *get_ptr()
	{
		return val.get_ptr();
	}

	virtual ValueType const *get_ptr() const
	{
		return val.get_ptr();
	}

private:
	template <typename... arg_type>
	ref_count_p(arg_type&&... args)
	: val(this, std::forward<arg_type>(args)...)
	{}

	ref_count_val<ValueType> val;
};

template <typename ValueType>
struct ref_count_e : public ref_count<ValueType> {
	template <typename... arg_type>
	static ref_count<ValueType> *create(
		size_t extra_size_, arg_type&&... args
	)
	{
		void *p(::operator new(
			sizeof(ref_count_e<ValueType>) + extra_size_
		));

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
		this->~ref_count_e<ValueType>();

		::operator delete(p);
	}

	virtual ValueType *get_ptr()
	{
		return val.get_ptr();
	}

	virtual ValueType const *get_ptr() const
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
	ref_count_val<ValueType> val;
	char                      extra_storage[];
};

template <typename ValueType, typename AllocType>
struct ref_count_a : public ref_count<ValueType> {
	template <typename... arg_type>
	static ref_count<ValueType> *create(AllocType a, arg_type&&... args)
	{
		typename AllocType::template rebind<this_type>::other a2(a);

		ref_count_a *p(a2.allocate(1));

		try {
			return ::new (p) ref_count_a(
				std::forward<AllocType>(a),
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
		typename AllocType::template rebind<
			this_type
		>::other a(std::get<1>(val_plus));

		a.destroy(this);
		a.deallocate(this, 1);
	}

	virtual void *get_allocator(std::type_info const &ti)
	{
		if (ti == typeid(AllocType))
			return &std::get<1>(val_plus);
		else
			return 0;
	}

	virtual ValueType *get_ptr()
	{
		return std::get<0>(val_plus).get_ptr();
	}

	virtual ValueType const *get_ptr() const
	{
		return std::get<0>(val_plus).get_ptr();
	}

private:
	typedef ref_count_a<ValueType, AllocType> this_type;

	template <typename... arg_type>
	ref_count_a(AllocType a, arg_type&&... args)
	: val_plus(
		ref_count_val<ValueType>(
			this, std::forward<arg_type>(args)...
		),
		std::forward<AllocType>(a)
	)
	{}

	std::tuple<ref_count_val<ValueType>, AllocType> val_plus;
};

template <typename ValueType, typename AllocType>
struct ref_count_a_e : public ref_count<ValueType> {
	template <typename... arg_type>
	static ref_count<ValueType> *create(
		AllocType a, size_t extra_size, arg_type&&... args
	)
	{
		size_t sz(sizeof(this_type) + extra_size);
		typename AllocType::template rebind<
			char
		>::other raw(a);

		void *p(raw.allocate(sz));

		try {
			return ::new (p) ref_count_a_e(
				std::forward<AllocType>(a), extra_size,
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
		typename AllocType::template rebind<
			char
		>::other raw(std::get<1>(alloc_plus));

		this->~ref_count_a_e();
		raw.deallocate(reinterpret_cast<char *>(this), sz);
	}

	virtual void *get_allocator(std::type_info const &ti)
	{
		if (ti == typeid(AllocType))
			 return &std::get<1>(alloc_plus);
		else
			return 0;
	}

	virtual ValueType *get_ptr()
	{
		return val.get_ptr();
	}

	virtual ValueType const *get_ptr() const
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
	typedef ref_count_a_e<ValueType, AllocType> this_type;

	template <typename... arg_type>
	ref_count_a_e(AllocType a, size_t extra_size, arg_type&&... args)
	: alloc_plus(extra_size, std::forward<AllocType>(a)),
	  val(this, std::forward<arg_type>(args)...)
	{}

	std::tuple<size_t, AllocType> alloc_plus;
	ref_count_val<ValueType>      val;
	char                          extra_storage[];
};

}}}
#endif

/*
 * Copyright (C) 2010 - 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_COUNTED_PTR_20131031T1800)
#define UCPF_YESOD_COUNTED_PTR_20131031T1800

#include <yesod/detail/counted_ptr.hpp>

namespace ucpf { namespace yesod {

template <typename ValueType>
struct counted_ptr {
	typedef ValueType value_type;

	struct extra_size_t {
		explicit extra_size_t(size_t size_)
		: size(size_)
		{}

		size_t size;
	};

	~counted_ptr()
	{
		reset();
	}

	counted_ptr()
	: ptr(0)
	{}

	counted_ptr(std::nullptr_t)
	: ptr(0)
	{}

	counted_ptr(counted_ptr const &other)
	: ptr(other.ptr)
	{
		if (ptr)
			ptr->add_ref_copy();
	}

	counted_ptr(counted_ptr &&other)
	: ptr(other.ptr)
	{
		other.ptr = nullptr;
	}

	counted_ptr &operator=(counted_ptr const &other)
	{
		if (ptr)
			ptr->release();

		ptr = other.ptr;

		if (ptr)
			ptr->add_ref_copy();

		return *this;
	}

	counted_ptr &operator=(counted_ptr &&other)
	{
		if (ptr)
			ptr->release();

		ptr = other.ptr;
		other.ptr = nullptr;

		return *this;
	}

	void reset()
	{
		if (ptr) {
			ptr->release();
			ptr = nullptr;
		}
	}

	value_type *get() const
	{
		return ptr->get_val_ptr();
	}

	typename std::add_lvalue_reference<value_type>::type operator*() const
	{
		return *get();
	}

	value_type *operator->() const
	{
		return get();
	}

	void *get_extra() const
	{
		return ptr->get_extra();
	}

	void *get_extra(size_t &sz) const
	{
		return ptr->get_extra(sz);
	}

	void access_allocator(void *data) const
	{
		ptr->access_allocator(data);
	}

	explicit operator bool() const
	{
		return ptr ? true : false;
	}

	bool unique() const
	{
		return use_count() == 1ul;
	}

	unsigned long use_count() const
	{
		return ptr ? ptr->get_use_count() : 0;
	}

	void swap(counted_ptr<ValueType> &other)
	{
		std::swap(ptr, other.ptr);
	}

	detail::counted_ptr_val<ValueType> *get_value_container() const
	{
		return ptr;
	}

	template <typename Alloc>
	Alloc const &get_allocator() const
	{
		Alloc *a_ptr(nullptr);
		if (ptr) {
			auto aw(ptr->get_alloc_wrapper());
			a_ptr = dynamic_cast<Alloc *>(aw);
		}

		if (!a_ptr)
			throw std::bad_cast();

		return *a_ptr;
	}

	template <typename ValueType1, typename Alloc, typename... Args>
	friend counted_ptr<ValueType1> allocate_counted(
		Alloc const &a,
		typename counted_ptr<ValueType1>::extra_size_t extra_size,
		Args&&... args
	);

	template <typename ValueType1, typename Alloc, typename... Args>
	friend counted_ptr<ValueType1> allocate_counted(
		Alloc const &a, Args&&... args
	);

	template <typename ValueType1, typename... Args>
	friend counted_ptr<ValueType1> make_counted(
		typename counted_ptr<ValueType1>::extra_size_t extra_size,
		Args&&... args
	);

	template <typename ValueType1, typename... Args>
	friend counted_ptr<ValueType1> make_counted(Args&&... args);

private:
	template <typename ValueType1>
	friend struct counted_ptr;

	typedef detail::counted_ptr_val<ValueType> container_type;

	explicit counted_ptr(container_type *ptr_, bool inc)
	: ptr(ptr_)
	{
		if (inc && ptr)
			ptr->add_ref_copy();
	}

	container_type *ptr;
};

template <typename ValueType, typename Alloc, typename... Args>
counted_ptr<ValueType> allocate_counted(
	Alloc const &a,
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	Args&&... args
)
{
	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			a, extra_size.size, std::forward<Args>(args)...
		), false
	);
}

template <typename ValueType, typename Alloc, typename... Args>
counted_ptr<ValueType> allocate_counted(Alloc const &a, Args&&... args)
{
	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			a, 0, std::forward<Args>(args)...
		), false
	);
}

template <typename ValueType, typename... Args>
counted_ptr<ValueType> make_counted(
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	Args&&... args
)
{
	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			std::allocator<void>(), extra_size.size,
			std::forward<Args>(args)...
		), false
	);
}

template <typename ValueType, typename... Args>
counted_ptr<ValueType> make_counted(Args&&... args)
{
	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			std::allocator<void>(), 0, std::forward<Args>(args)...
		), false
	);
}

template <typename ValueType0, typename ValueType1>
bool operator==(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	return a.get() == b.get();
}

template <typename ValueType>
bool operator==(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !a;
}

template <typename ValueType>
bool operator==(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !a;
}

template <typename ValueType0, typename ValueType1>
bool operator!=(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	return a.get() != b.get();
}

template <typename ValueType>
bool operator!=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return bool(a);
}

template <typename ValueType>
bool operator!=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return bool(a);
}

template <typename ValueType0, typename ValueType1>
bool operator<(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	typedef typename std::common_type<
		ValueType0 *, ValueType1 *
	>::type c_type;
	return std::less<c_type>()(a.get(), b.get());
}

template <typename ValueType>
bool operator<(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return std::less<ValueType *>()(a.get(), nullptr);
}

template <typename ValueType>
bool operator<(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return std::less<ValueType *>()(nullptr, a.get());
}

template <typename ValueType0, typename ValueType1>
bool operator<=(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	return !(b < a);
}

template <typename ValueType>
bool operator<=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !(nullptr < a);
}

template <typename ValueType>
bool operator<=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !(a < nullptr);
}

template <typename ValueType0, typename ValueType1>
bool operator>(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	return b < a;
}

template <typename ValueType>
bool operator>(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return std::less<ValueType *>()(nullptr, a.get());
}

template <typename ValueType>
bool operator>(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return std::less<ValueType *>()(a.get(), nullptr);
}

template <typename ValueType0, typename ValueType1>
bool operator>=(
	counted_ptr<ValueType0> const &a, counted_ptr<ValueType1> const &b
)
{
	return !(a < b);
}

template <typename ValueType>
bool operator>=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !(a < nullptr);
}

template <typename ValueType>
bool operator>=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !(nullptr < a);
}

}}
#endif

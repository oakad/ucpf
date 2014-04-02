/*
 * Copyright (C) 2010 - 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_COUNTED_PTR_OCT_31_2013_1800)
#define UCPF_YESOD_COUNTED_PTR_OCT_31_2013_1800

#include <yesod/detail/counted_ptr.hpp>

namespace ucpf { namespace yesod {

template <typename ValueType>
struct counted_ptr;

template <typename ValueType, typename Alloc, typename... Args>
counted_ptr<ValueType> allocate_counted(
	Alloc const &a,
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	Args&&... args
);

template <typename ValueType, typename Alloc, typename... Args>
counted_ptr<ValueType> allocate_counted(Alloc const &a, Args&&... args);

template <typename ValueType, typename... Args>
counted_ptr<ValueType> make_counted(
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	Args&&... args
);

template <typename ValueType, typename... Args>
counted_ptr<ValueType> make_counted(Args&&... args);

template <typename ValueType0, typename ValueType1>
counted_ptr<ValueType0> const_pointer_cast(counted_ptr<ValueType1> const &p);

template <typename ValueType>
struct counted_ptr {
	typedef ValueType type;

	struct extra_size_t {
		explicit extra_size_t(size_t size_)
		: size(size_)
		{}

		size_t size;
	};

	~counted_ptr()
	{
		auto s_ptr(ptr.exchange(nullptr));
		if (s_ptr)
			s_ptr->release();
	}

	counted_ptr()
	: ptr(nullptr)
	{}

	counted_ptr(std::nullptr_t)
	: ptr(nullptr)
	{}

	counted_ptr(counted_ptr const &p)
	: ptr(p.ptr.load())
	{
		if (ptr)
			ptr.load()->add_ref_copy();
	}

	counted_ptr(counted_ptr &&p)
	: ptr(p.ptr.exchange(nullptr))
	{}

	counted_ptr &operator=(counted_ptr const &p)
	{
		auto r_ptr(p.ptr.load());
		auto s_ptr(ptr.exchange(r_ptr));

		if (s_ptr != r_ptr) {
			if (r_ptr)
				r_ptr->add_ref_copy();

			if (s_ptr)
				s_ptr->release();
		}
		return *this;
	}

	counted_ptr &operator=(counted_ptr &&p)
	{
		auto r_ptr(p.ptr.exchange(nullptr));
		auto s_ptr(ptr.exchange(r_ptr));
		if (s_ptr && (s_ptr != r_ptr))
			s_ptr->release();

		return *this;
	}

	void reset()
	{
		auto s_ptr(ptr.exchange(nullptr));
		if (s_ptr)
			s_ptr->release();
	}

	ValueType *get() const
	{
		return ptr.load()->get_val_ptr();
	}

	typename std::add_lvalue_reference<ValueType>::type operator*() const
	{
		return *get();
	}

	ValueType *operator->() const
	{
		return get();
	}

	void *get_extra() const
	{
		return ptr.load()->get_extra();
	}

	void *get_extra(size_t &sz) const
	{
		return ptr.load()->get_extra(sz);
	}

	void access_allocator(void *data) const
	{
		ptr.load()->access_allocator(data);
	}

	explicit operator bool() const
	{
		return ptr.load() ? true : false;
	}

	bool unique() const
	{
		return use_count() == 1ul;
	}

	unsigned long use_count() const
	{
		if (ptr)
			ptr.load()->get_use_count();
		else
			return 0;
	}

	void swap(counted_ptr<ValueType> &p)
	{
		auto s_ptr(ptr.exchange(p.ptr.load()));
		p.ptr.exchange(s_ptr);
	}

	detail::counted_ptr_val<ValueType> *get_value_container() const
	{
		return ptr.load();
	}

	template <typename Alloc>
	Alloc const &get_allocator() const
	{
		Alloc *a_ptr(nullptr);
		auto s_ptr(ptr.load());
		if (s_ptr) {
			auto aw(s_ptr->get_alloc_wrapper());
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

	template <typename ValueType0, typename ValueType1>
	friend counted_ptr<ValueType0> static_pointer_cast(
		counted_ptr<ValueType1> const &p
	);

	template <typename ValueType0, typename ValueType1>
	friend counted_ptr<ValueType0> const_pointer_cast(
		counted_ptr<ValueType1> const &p
	);

	template <typename ValueType0, typename ValueType1>
	friend counted_ptr<ValueType0> dynamic_pointer_cast(
		counted_ptr<ValueType1> const &p
	);

private:
	template <typename ValueType1>
	friend struct counted_ptr;

	explicit counted_ptr(
		detail::counted_ptr_val<ValueType> *ptr_, bool inc
	) : ptr(ptr_)
	{
		if (inc && ptr) {
			ptr.load()->add_ref_copy();
		}
	}

	std::atomic<detail::counted_ptr_val<ValueType> *> ptr;
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
	std::allocator<void> a;

	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			a, extra_size.size, std::forward<Args>(args)...
		), false
	);
}

template <typename ValueType, typename... Args>
counted_ptr<ValueType> make_counted(Args&&... args)
{
	std::allocator<void> a;

	return counted_ptr<ValueType>(
		detail::counted_ptr_val<ValueType>::construct(
			a, 0, std::forward<Args>(args)...
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

template <typename ValueType0, typename ValueType1>
counted_ptr<ValueType0> const_pointer_cast(counted_ptr<ValueType1> const &p)
{
	return counted_ptr<ValueType0>(
		const_cast<ValueType0>(p.ptr.load()), true
	);
}

}}
#endif

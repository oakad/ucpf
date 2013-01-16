/*
 * Copyright (C) 2010, 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(_RASHAM_COUNTED_PTR_HPP)
#define _RASHAM_COUNTED_PTR_HPP

#include <rasham/internal/counted_ptr_base.hpp>

namespace rasham
{

template<typename value_type>
struct counted_ptr;

template<typename value_type, typename alloc_type, typename... arg_type>
counted_ptr<value_type> allocate_counted(
	alloc_type a,
	typename counted_ptr<value_type>::extra_size_t extra_size,
	arg_type&&... args
);

template<typename value_type, typename alloc_type, typename... arg_type>
counted_ptr<value_type> allocate_counted(alloc_type a, arg_type&&... args);

template<typename value_type, typename... arg_type>
counted_ptr<value_type> make_counted(
	typename counted_ptr<value_type>::extra_size_t extra_size,
	arg_type&&... args
);

template<typename value_type, typename... arg_type>
counted_ptr<value_type> make_counted(arg_type&&... args);

template<typename value_type1, typename value_type2>
counted_ptr<value_type1> static_pointer_cast(counted_ptr<value_type2> const &p);

template<typename value_type1, typename value_type2>
counted_ptr<value_type1> const_pointer_cast(counted_ptr<value_type2> const &p);

template<typename value_type1, typename value_type2>
counted_ptr<value_type1>
dynamic_pointer_cast(counted_ptr<value_type2> const &p);

template<typename value_type>
struct counted_ptr {
	typedef value_type type;

	struct extra_size_t {
		explicit extra_size_t(size_t size_)
		: size(size_)
		{}

		size_t size;
	};

	~counted_ptr()
	{
		if (val)
			ref_count_val<value_type>::get_this(val)->release();
	}

	counted_ptr()
	: val(0)
	{}

	counted_ptr(counted_ptr const &p)
	: val(p.val)
	{
		if (val)
			ref_count_val<value_type>::get_this(val)
			->add_ref_copy();
	}

	template<typename value_type1>
	counted_ptr(counted_ptr<value_type1> const &p)
	: val(p.val)
	{
		if (val)
			ref_count_val<value_type>::get_this(val)
			->add_ref_copy();
	}

	counted_ptr(counted_ptr &&p)
	: val(p.val)
	{
		p.val = 0;
	}

	template<typename value_type1>
	counted_ptr(counted_ptr<value_type1> &&p)
	: val(p.val)
	{
		p.val = 0;
	}

	counted_ptr &operator=(counted_ptr const &p)
	{
		if (p.val != val) {
			if (p.val)
				ref_count_val<value_type>::get_this(p.val)
				->add_ref_copy();

			if (val)
				ref_count_val<value_type>::get_this(val)
				->release();

			val = p.val;
		}
		return *this;
	}

	template<typename value_type1>
	counted_ptr &operator=(counted_ptr<value_type1> const &p)
	{
		if (p.val != val) {
			if (p.val)
				ref_count_val<value_type1>::get_this(p.val)
				->add_ref_copy();

			if (val)
				ref_count_val<value_type>::get_this(val)
				->release();

			val = p.val;
		}
		return *this;
	}

	counted_ptr &operator=(counted_ptr &&p)
	{
		counted_ptr(std::move(p)).swap(*this);
		return *this;
	}

	template<class value_type1>
	counted_ptr &operator=(counted_ptr<value_type1> &&p)
	{
		counted_ptr(std::move(p)).swap(*this);
		return *this;
	}

	void reset()
	{
		counted_ptr().swap(*this);
	}

	typename std::add_lvalue_reference<value_type>::type operator*() const
	{
		return *val;
	}

	value_type *operator->() const
	{
		return val;
	}

	value_type *get() const
	{
		return val;
	}

	ref_count<value_type> const *get_this() const
	{
		if (val)
			return ref_count_val<value_type>::get_this(val);
		else
			return 0;
	}

	template<typename value_type1>
	value_type1 *get_extra()
	{
		if (val)
			return static_cast<value_type1 *>(
				ref_count_val<value_type>::get_this(val)
				->get_extra()
			);
		else
			return nullptr;
	}

	template<typename value_type1>
	value_type1 *get_extra() const
	{
		if (val)
			return static_cast<value_type1 *>(
				ref_count_val<value_type>::get_this(val)
				->get_extra()
			);
		else
			return nullptr;
	}

	template<typename value_type1>
	value_type1 *get_extra(size_t &sz)
	{
		if (val)
			return static_cast<value_type1 *>(
				ref_count_val<value_type>::get_this(val)
				->get_extra(sz)
			);
		else
			return nullptr;
	}

	template<typename value_type1>
	value_type1 *get_extra(size_t &sz) const
	{
		if (val)
			return static_cast<value_type1 *>(
				ref_count_val<value_type>::get_this(val)
				->get_extra(sz)
			);
		else
			return nullptr;
	}

private:
	typedef value_type* counted_ptr::*__unspecified_bool_type;

public:
	operator __unspecified_bool_type() const
	{
		if (val)
			return &counted_ptr::val;
		else
			return 0;
	}

	bool unique() const
	{
		if (val)
			return ref_count_val<value_type>::get_this(val)
			       ->unique();
		else
			return 0;
	}

	long use_count() const
	{
		if (val)
			return ref_count_val<value_type>::get_this(val)
			       ->get_use_count();
		else
			return 0;
	}

	void swap(counted_ptr<value_type> &other)
	{
		std::swap(val, other.val);
	}

	template<typename alloc_type>
	alloc_type *get_allocator() const
	{
		return static_cast<alloc_type *>(
			this->get_allocator(typeid(alloc_type))
		);
	}

	template<typename value_type1, typename alloc_type,
		 typename... arg_type>
	friend counted_ptr<value_type1> allocate_counted(
		alloc_type a,
		typename counted_ptr<value_type1>::extra_size_t extra_size,
		arg_type&&... args
	);

	template<typename value_type1, typename alloc_type,
		 typename... arg_type>
	friend counted_ptr<value_type1> allocate_counted(
		alloc_type a, arg_type&&... args
	);

	template<typename value_type1, typename... arg_type>
	friend counted_ptr<value_type1> make_counted(
		typename counted_ptr<value_type1>::extra_size_t extra_size,
		arg_type&&... args
	);

	template<typename value_type1, typename... arg_type>
	friend counted_ptr<value_type1> make_counted(arg_type&&... args);

	template<typename value_type1, typename value_type2>
	friend counted_ptr<value_type1> static_pointer_cast(
		counted_ptr<value_type2> const &p
	);

	template<typename value_type1, typename value_type2>
	friend counted_ptr<value_type1> const_pointer_cast(
		counted_ptr<value_type2> const &p
	);

	template<typename value_type1, typename value_type2>
	friend counted_ptr<value_type1> dynamic_pointer_cast(
		counted_ptr<value_type2> const &p
	);

private:
	template<typename value_type1> friend class counted_ptr;

	explicit counted_ptr(ref_count<value_type> *r, bool increment = false)
	: val(r->get_ptr())
	{
		if (increment && val) {
			ref_count_val<value_type>::get_this(val)
			->add_ref_copy();
		}
	}

	void *get_allocator(std::type_info const &ti) const
	{
		if (val)
			return ref_count_val<value_type>::get_this(val)
			       ->get_allocator(ti);
		else
			return 0;
	}

	value_type *val;
};

template<typename value_type, typename alloc_type, typename... arg_type>
counted_ptr<value_type> allocate_counted(
	alloc_type a,
	typename counted_ptr<value_type>::extra_size_t extra_size,
	arg_type&&... args
)
{
	return counted_ptr<value_type>(
		ref_count_a_e<value_type, alloc_type>::create(
			std::forward<alloc_type>(a),
			extra_size.size,
			std::forward<arg_type>(args)...
		)
	);
}

template<typename value_type, typename alloc_type, typename... arg_type>
counted_ptr<value_type> allocate_counted(alloc_type a, arg_type&&... args)
{
	return counted_ptr<value_type>(
		ref_count_a<value_type, alloc_type>::create(
			std::forward<alloc_type>(a),
			std::forward<arg_type>(args)...
		)
	);
}

template<typename value_type, typename... arg_type>
counted_ptr<value_type> make_counted(
	typename counted_ptr<value_type>::extra_size_t extra_size,
	arg_type&&... args
)
{
	return counted_ptr<value_type>(
		ref_count_e<value_type>::create(
			extra_size.size,
			std::forward<arg_type>(args)...
		)
	);
}

template<typename value_type, typename... arg_type>
counted_ptr<value_type> make_counted(arg_type&&... args)
{
	return counted_ptr<value_type>(
		ref_count_p<value_type>::create(
			std::forward<arg_type>(args)...
		)
	);
}

template<typename value_type1, typename value_type2>
bool operator==(counted_ptr<value_type1> const &a,
		counted_ptr<value_type2> const &b)
{
	return a.get() == b.get();
}

template<typename value_type>
bool operator==(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return !a;
}

template<typename value_type>
bool operator==(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return !a;
}

template<typename value_type1, typename value_type2>
bool operator!=(counted_ptr<value_type1> const &a,
		counted_ptr<value_type2> const &b)
{
	return a.get() != b.get();
}

template<typename value_type>
bool operator!=(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return bool(a);
}

template<typename value_type>
bool operator!=(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return bool(a);
}

template<typename value_type1, typename value_type2>
bool operator<(counted_ptr<value_type1> const &a,
	       counted_ptr<value_type2> const &b)
{
	typedef typename std::common_type<value_type1 *, value_type2 *>::type
	c_type;
	return std::less<c_type>()(a.get(), b.get());
}

template<typename value_type>
bool operator<(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return std::less<value_type *>()(a.get(), nullptr);
}

template<typename value_type>
bool operator<(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return std::less<value_type *>()(nullptr, a.get());
}

template<typename value_type1, typename value_type2>
bool operator<=(counted_ptr<value_type1> const &a,
		counted_ptr<value_type2> const &b)
{
	return !(b < a);
}

template<typename value_type>
bool operator<=(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return !(nullptr < a);
}

template<typename value_type>
bool operator<=(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return !(a < nullptr);
}

template<typename value_type1, typename value_type2>
bool operator>(counted_ptr<value_type1> const &a,
	       counted_ptr<value_type2> const &b)
{
	return b < a;
}

template<typename value_type>
bool operator>(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return std::less<value_type *>()(nullptr, a.get());
}

template<typename value_type>
bool operator>(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return std::less<value_type *>()(a.get(), nullptr);
}

template<typename value_type1, typename value_type2>
bool operator>=(counted_ptr<value_type1> const &a,
		counted_ptr<value_type2> const &b)
{
	return !(a < b);
}

template<typename value_type>
bool operator>=(counted_ptr<value_type> const &a, std::nullptr_t)
{
	return !(a < nullptr);
}

template<typename value_type>
bool operator>=(std::nullptr_t, counted_ptr<value_type> const &a)
{
	return !(nullptr < a);
}

template<typename value_type1, typename value_type2>
counted_ptr<value_type1> static_pointer_cast(counted_ptr<value_type2> const &p)
{
	if (p.val)
		return counted_ptr<value_type1>(
			ref_count_val<value_type1>::get_this(
				static_cast<value_type1 *>(p.val)
			), true
		);
	else
		return counted_ptr<value_type1>();
}

template<typename value_type1, typename value_type2>
counted_ptr<value_type1> const_pointer_cast(counted_ptr<value_type2> const &p)
{
	if (p.val)
		return counted_ptr<value_type1>(
			ref_count_val<value_type1>::get_this(
				const_cast<value_type1 *>(p.val)
			), true
		);
	else
		return counted_ptr<value_type1>();
}

template<typename value_type1, typename value_type2>
counted_ptr<value_type1> dynamic_pointer_cast(counted_ptr<value_type2> const &p)
{
	if (dynamic_cast<value_type1 *>(p.val))
		return static_pointer_cast(p);
	else
		return counted_ptr<value_type1>();
}



}
#endif

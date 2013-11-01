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

template<typename ValueType>
struct counted_ptr;

template<typename ValueType, typename AllocType, typename... arg_type>
counted_ptr<ValueType> allocate_counted(
	AllocType a,
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	arg_type&&... args
);

template<typename ValueType, typename AllocType, typename... arg_type>
counted_ptr<ValueType> allocate_counted(AllocType a, arg_type&&... args);

template<typename ValueType, typename... arg_type>
counted_ptr<ValueType> make_counted(
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	arg_type&&... args
);

template<typename ValueType, typename... arg_type>
counted_ptr<ValueType> make_counted(arg_type&&... args);

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1> static_pointer_cast(counted_ptr<ValueType2> const &p);

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1> const_pointer_cast(counted_ptr<ValueType2> const &p);

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1>
dynamic_pointer_cast(counted_ptr<ValueType2> const &p);

template<typename ValueType>
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
		if (val)
			detail::ref_count_val<
				ValueType
			>::get_this(val)->release();
	}

	counted_ptr()
	: val(0)
	{}

	counted_ptr(counted_ptr const &p)
	: val(p.val)
	{
		if (val)
			detail::ref_count_val<ValueType>::get_this(val)
			->add_ref_copy();
	}

	template<typename ValueType1>
	counted_ptr(counted_ptr<ValueType1> const &p)
	: val(p.val)
	{
		if (val)
			detail::ref_count_val<ValueType>::get_this(val)
			->add_ref_copy();
	}

	counted_ptr(counted_ptr &&p)
	: val(p.val)
	{
		p.val = 0;
	}

	template<typename ValueType1>
	counted_ptr(counted_ptr<ValueType1> &&p)
	: val(p.val)
	{
		p.val = 0;
	}

	counted_ptr &operator=(counted_ptr const &p)
	{
		if (p.val != val) {
			if (p.val)
				detail::ref_count_val<
					ValueType
				>::get_this(p.val)->add_ref_copy();

			if (val)
				detail::ref_count_val<ValueType>::get_this(val)
				->release();

			val = p.val;
		}
		return *this;
	}

	template<typename ValueType1>
	counted_ptr &operator=(counted_ptr<ValueType1> const &p)
	{
		if (p.val != val) {
			if (p.val)
				detail::ref_count_val<
					ValueType1
				>::get_this(p.val)->add_ref_copy();

			if (val)
				detail::ref_count_val<
					ValueType
				>::get_this(val)->release();

			val = p.val;
		}
		return *this;
	}

	counted_ptr &operator=(counted_ptr &&p)
	{
		counted_ptr(std::move(p)).swap(*this);
		return *this;
	}

	template<class ValueType1>
	counted_ptr &operator=(counted_ptr<ValueType1> &&p)
	{
		counted_ptr(std::move(p)).swap(*this);
		return *this;
	}

	void reset()
	{
		counted_ptr().swap(*this);
	}

	typename std::add_lvalue_reference<ValueType>::type operator*() const
	{
		return *val;
	}

	ValueType *operator->() const
	{
		return val;
	}

	ValueType *get() const
	{
		return val;
	}

	detail::ref_count<ValueType> const *get_this() const
	{
		if (val)
			return detail::ref_count_val<ValueType>::get_this(val);
		else
			return 0;
	}

	template<typename ValueType1>
	ValueType1 *get_extra()
	{
		if (val)
			return static_cast<ValueType1 *>(
				detail::ref_count_val<ValueType>::get_this(val)
				->get_extra()
			);
		else
			return nullptr;
	}

	template<typename ValueType1>
	ValueType1 *get_extra() const
	{
		if (val)
			return static_cast<ValueType1 *>(
				detail::ref_count_val<ValueType>::get_this(val)
				->get_extra()
			);
		else
			return nullptr;
	}

	template<typename ValueType1>
	ValueType1 *get_extra(size_t &sz)
	{
		if (val)
			return static_cast<ValueType1 *>(
				detail::ref_count_val<ValueType>::get_this(val)
				->get_extra(sz)
			);
		else
			return nullptr;
	}

	template<typename ValueType1>
	ValueType1 *get_extra(size_t &sz) const
	{
		if (val)
			return static_cast<ValueType1 *>(
				detail::ref_count_val<ValueType>::get_this(val)
				->get_extra(sz)
			);
		else
			return nullptr;
	}

private:
	typedef ValueType* counted_ptr::*__unspecified_bool_type;

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
			return detail::ref_count_val<ValueType>::get_this(val)
			       ->unique();
		else
			return 0;
	}

	long use_count() const
	{
		if (val)
			return detail::ref_count_val<ValueType>::get_this(val)
			       ->get_use_count();
		else
			return 0;
	}

	void swap(counted_ptr<ValueType> &other)
	{
		std::swap(val, other.val);
	}

	template<typename AllocType>
	AllocType *get_allocator() const
	{
		return static_cast<AllocType *>(
			this->get_allocator(typeid(AllocType))
		);
	}

	template<typename ValueType1, typename AllocType,
		 typename... arg_type>
	friend counted_ptr<ValueType1> allocate_counted(
		AllocType a,
		typename counted_ptr<ValueType1>::extra_size_t extra_size,
		arg_type&&... args
	);

	template<typename ValueType1, typename AllocType,
		 typename... arg_type>
	friend counted_ptr<ValueType1> allocate_counted(
		AllocType a, arg_type&&... args
	);

	template<typename ValueType1, typename... arg_type>
	friend counted_ptr<ValueType1> make_counted(
		typename counted_ptr<ValueType1>::extra_size_t extra_size,
		arg_type&&... args
	);

	template<typename ValueType1, typename... arg_type>
	friend counted_ptr<ValueType1> make_counted(arg_type&&... args);

	template<typename ValueType1, typename ValueType2>
	friend counted_ptr<ValueType1> static_pointer_cast(
		counted_ptr<ValueType2> const &p
	);

	template<typename ValueType1, typename ValueType2>
	friend counted_ptr<ValueType1> const_pointer_cast(
		counted_ptr<ValueType2> const &p
	);

	template<typename ValueType1, typename ValueType2>
	friend counted_ptr<ValueType1> dynamic_pointer_cast(
		counted_ptr<ValueType2> const &p
	);

private:
	template<typename ValueType1> friend struct counted_ptr;

	explicit counted_ptr(
		detail::ref_count<ValueType> *r, bool increment = false
	) : val(r->get_ptr())
	{
		if (increment && val) {
			detail::ref_count_val<ValueType>::get_this(val)
			->add_ref_copy();
		}
	}

	void *get_allocator(std::type_info const &ti) const
	{
		if (val)
			return detail::ref_count_val<ValueType>::get_this(val)
			       ->get_allocator(ti);
		else
			return 0;
	}

	ValueType *val;
};

template<typename ValueType, typename AllocType, typename... arg_type>
counted_ptr<ValueType> allocate_counted(
	AllocType a,
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	arg_type&&... args
)
{
	return counted_ptr<ValueType>(
		detail::ref_count_a_e<ValueType, AllocType>::create(
			std::forward<AllocType>(a),
			extra_size.size,
			std::forward<arg_type>(args)...
		)
	);
}

template<typename ValueType, typename AllocType, typename... arg_type>
counted_ptr<ValueType> allocate_counted(AllocType a, arg_type&&... args)
{
	return counted_ptr<ValueType>(
		detail::ref_count_a<ValueType, AllocType>::create(
			std::forward<AllocType>(a),
			std::forward<arg_type>(args)...
		)
	);
}

template<typename ValueType, typename... arg_type>
counted_ptr<ValueType> make_counted(
	typename counted_ptr<ValueType>::extra_size_t extra_size,
	arg_type&&... args
)
{
	return counted_ptr<ValueType>(
		detail::ref_count_e<ValueType>::create(
			extra_size.size,
			std::forward<arg_type>(args)...
		)
	);
}

template<typename ValueType, typename... arg_type>
counted_ptr<ValueType> make_counted(arg_type&&... args)
{
	return counted_ptr<ValueType>(
		detail::ref_count_p<ValueType>::create(
			std::forward<arg_type>(args)...
		)
	);
}

template<typename ValueType1, typename ValueType2>
bool operator==(counted_ptr<ValueType1> const &a,
		counted_ptr<ValueType2> const &b)
{
	return a.get() == b.get();
}

template<typename ValueType>
bool operator==(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !a;
}

template<typename ValueType>
bool operator==(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !a;
}

template<typename ValueType1, typename ValueType2>
bool operator!=(counted_ptr<ValueType1> const &a,
		counted_ptr<ValueType2> const &b)
{
	return a.get() != b.get();
}

template<typename ValueType>
bool operator!=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return bool(a);
}

template<typename ValueType>
bool operator!=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return bool(a);
}

template<typename ValueType1, typename ValueType2>
bool operator<(counted_ptr<ValueType1> const &a,
	       counted_ptr<ValueType2> const &b)
{
	typedef typename std::common_type<ValueType1 *, ValueType2 *>::type
	c_type;
	return std::less<c_type>()(a.get(), b.get());
}

template<typename ValueType>
bool operator<(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return std::less<ValueType *>()(a.get(), nullptr);
}

template<typename ValueType>
bool operator<(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return std::less<ValueType *>()(nullptr, a.get());
}

template<typename ValueType1, typename ValueType2>
bool operator<=(counted_ptr<ValueType1> const &a,
		counted_ptr<ValueType2> const &b)
{
	return !(b < a);
}

template<typename ValueType>
bool operator<=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !(nullptr < a);
}

template<typename ValueType>
bool operator<=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !(a < nullptr);
}

template<typename ValueType1, typename ValueType2>
bool operator>(counted_ptr<ValueType1> const &a,
	       counted_ptr<ValueType2> const &b)
{
	return b < a;
}

template<typename ValueType>
bool operator>(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return std::less<ValueType *>()(nullptr, a.get());
}

template<typename ValueType>
bool operator>(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return std::less<ValueType *>()(a.get(), nullptr);
}

template<typename ValueType1, typename ValueType2>
bool operator>=(counted_ptr<ValueType1> const &a,
		counted_ptr<ValueType2> const &b)
{
	return !(a < b);
}

template<typename ValueType>
bool operator>=(counted_ptr<ValueType> const &a, std::nullptr_t)
{
	return !(a < nullptr);
}

template<typename ValueType>
bool operator>=(std::nullptr_t, counted_ptr<ValueType> const &a)
{
	return !(nullptr < a);
}

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1> static_pointer_cast(counted_ptr<ValueType2> const &p)
{
	if (p.val)
		return counted_ptr<ValueType1>(
			detail::ref_count_val<ValueType1>::get_this(
				static_cast<ValueType1 *>(p.val)
			), true
		);
	else
		return counted_ptr<ValueType1>();
}

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1> const_pointer_cast(counted_ptr<ValueType2> const &p)
{
	if (p.val)
		return counted_ptr<ValueType1>(
			detail::ref_count_val<ValueType1>::get_this(
				const_cast<ValueType1 *>(p.val)
			), true
		);
	else
		return counted_ptr<ValueType1>();
}

template<typename ValueType1, typename ValueType2>
counted_ptr<ValueType1> dynamic_pointer_cast(counted_ptr<ValueType2> const &p)
{
	if (dynamic_cast<ValueType1 *>(p.val))
		return static_pointer_cast(p);
	else
		return counted_ptr<ValueType1>();
}

}}
#endif

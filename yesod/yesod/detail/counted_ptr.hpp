/*
 * Copyright (c) 2010 - 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800)
#define UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800

#include <atomic>
#include <memory>
#include <cstddef>

namespace ucpf { namespace yesod { namespace detail {

template <typename T, typename Alloc>
struct has_destroy {
	template <typename U>
	static std::true_type test(
		U *, decltype(&U::template destroy<Alloc>) * = nullptr
	);

	static std::false_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;

	static constexpr bool value = type::value;
};

template <typename T, typename Alloc, bool HasDestroy = false>
struct counted_ptr_destroy_value {
	typedef std::allocator_traits<Alloc> allocator_traits;

	static void destroy(Alloc &a, T *v)
	{
		allocator_traits::destroy(a, v);
	}
};

template <typename T, typename Alloc>
struct counted_ptr_destroy_value<T, Alloc, true> {
	typedef std::allocator_traits<Alloc> allocator_traits;

	static void destroy(Alloc &a, T *v)
	{
		T::destroy(a, v);
	}
};

template <typename T, typename Alloc>
struct has_access_allocator {
	template <typename U>
	static std::true_type test(
		U *, decltype(&U::template access_allocator<Alloc>) * = nullptr
	);

	static std::false_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;

	static constexpr bool value = type::value;
};

template <typename T, typename Alloc, bool HasAccess = false>
struct counted_ptr_access_allocator {
	static void invoke(Alloc &a, T const *v, void *data)
	{}
};

template <typename T, typename Alloc>
struct counted_ptr_access_allocator<T, Alloc, true> {
	static void invoke(Alloc &a, T const *v, void *data)
	{
		return T::access_allocator(a, v, data);
	}
};

template <typename ValueType> struct counted_ptr_val;

template <typename ValueType>
struct counted_ptr_disp {
	void (*access_allocator)(
		counted_ptr_val<ValueType> const *p, void *data
	);
	std::tuple<void const *, size_t> (*get_extra)(
		counted_ptr_val<ValueType> const *p
	);
	size_t (*get_alloc_size)(counted_ptr_val<ValueType> const *p);
	void (*destroy)(counted_ptr_val<ValueType> *p);
};

struct counted_ptr_alloc_base {
	virtual ~counted_ptr_alloc_base()
	{}

	virtual void destroy_a(void *p) = 0;
	virtual void destroy_b(void *p) = 0;
	virtual void access_allocator(void const *p, void *data) const = 0;
};

template <typename ContainerType, typename Alloc>
struct counted_ptr_alloc_wrapper : counted_ptr_alloc_base, Alloc {
	typedef counted_ptr_alloc_wrapper          alloc_wrapper_type;
	typedef ContainerType                      container_type;
	typedef typename ContainerType::value_type value_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		alloc_wrapper_type
	> allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		alloc_wrapper_type
	> allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		value_type
	> value_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		value_type
	> value_allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		uint8_t
	> byte_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		uint8_t
	> byte_allocator_traits;

	counted_ptr_alloc_wrapper(Alloc const &a_)
	: Alloc(a_)
	{}

	counted_ptr_alloc_wrapper(Alloc &&a_)
	: Alloc(std::forward<Alloc>(a_))
	{}

	virtual ~counted_ptr_alloc_wrapper()
	{}

	virtual void destroy_a(void *p)
	{
		auto x_p(reinterpret_cast<container_type *>(p));
		auto x_value(x_p->get_val_ptr());
		auto sz(x_p->get_alloc_size());

		value_allocator_type value_alloc(*this);
		allocator_type self_alloc(*this);
		byte_allocator_type byte_alloc(*this);

		counted_ptr_destroy_value<
			value_type, value_allocator_type,
			has_destroy<value_type, value_allocator_type>::value
		>::destroy(value_alloc, x_value);
		allocator_traits::destroy(self_alloc, this);

		byte_allocator_traits::deallocate(
			byte_alloc, reinterpret_cast<uint8_t *>(p), sz
		);
	}

	virtual void destroy_b(void *p)
	{
		auto x_p(reinterpret_cast<container_type *>(p));
		auto x_value(x_p->get_val_ptr());
		auto sz(x_p->get_alloc_size());

		value_allocator_type value_alloc(*this);
		byte_allocator_type byte_alloc(*this);

		counted_ptr_destroy_value<
			value_type, value_allocator_type,
			has_destroy<value_type, value_allocator_type>::value
		>::destroy(value_alloc, x_value);

		byte_allocator_traits::deallocate(
			byte_alloc, reinterpret_cast<uint8_t *>(p), sz
		);
	}

	virtual void access_allocator(void const *p, void *data) const
	{
		auto x_p(reinterpret_cast<container_type const *>(p));
		auto x_value(x_p->get_val_ptr());

		value_allocator_type value_alloc(*this);
		counted_ptr_access_allocator<
			value_type, value_allocator_type,
			has_access_allocator<
				value_type, value_allocator_type
			>::value
		>::invoke(value_alloc, x_value, data);
	}

};

struct counted_ptr_extra_a {
	static size_t const h_size;
	static constexpr bool has_alloc = true;

	size_t t_size;
	counted_ptr_alloc_base *a_ref;
};

size_t const counted_ptr_extra_a::h_size = sizeof(
	typename std::aligned_storage<
		sizeof(counted_ptr_extra_a),
		std::alignment_of<counted_ptr_extra_a>::value
	>::type
);

struct counted_ptr_extra_ae {
	static size_t const h_size;
	static constexpr bool has_alloc = true;

	size_t t_size;
	counted_ptr_alloc_base *a_ref;
	size_t extra_offset;
};

size_t const counted_ptr_extra_ae::h_size = sizeof(
	typename std::aligned_storage<
		sizeof(counted_ptr_extra_ae),
		std::alignment_of<counted_ptr_extra_ae>::value
	>::type
);

struct counted_ptr_extra_b {
	static size_t const h_size;
	static constexpr bool has_alloc = false;

	size_t t_size;
};

size_t const counted_ptr_extra_b::h_size = sizeof(
	typename std::aligned_storage<
		sizeof(counted_ptr_extra_b),
		std::alignment_of<counted_ptr_extra_b>::value
	>::type
);

template <typename ValueType>
struct alignas(void *) counted_ptr_val {
	typedef ValueType value_type;

	counted_ptr_val(counted_ptr_val const &) = delete;
	counted_ptr_val &operator=(counted_ptr_val const &) = delete;

	template <typename Alloc, typename... Args>
	static counted_ptr_val *construct(
		Alloc const &a, size_t extra, Args&&... args
	);

	static void destroy(counted_ptr_val *p)
	{
		if (p->extra_disp && p->extra_disp->destroy)
			p->extra_disp->destroy(p);
		else {
			counted_ptr_alloc_wrapper<
				counted_ptr_val, std::allocator<void>
			> aw(std::allocator<void>{});

			aw.destroy_b(p);
		}
	}

	ValueType const *get_val_ptr() const
	{
		return reinterpret_cast<ValueType const *>(&val);
	}

	ValueType *get_val_ptr()
	{
		return reinterpret_cast<ValueType *>(&val);
	}

	void const *get_extra() const
	{
		if (extra_disp && extra_disp->get_extra)
			return std::get<0>(extra_disp->get_extra(this));
		else
			return nullptr;
	}

	void const *get_extra(size_t &sz) const
	{
		void const *p(nullptr);
		sz = 0;

		if (extra_disp && extra_disp->get_extra)
			std::tie(p, sz) = extra_disp->get_extra(this);

		return p;
	}

	void *get_extra()
	{
		return const_cast<void *>(
			const_cast<counted_ptr_val const *>(this)->get_extra()
		);
	}

	void *get_extra(size_t &sz)
	{
		return const_cast<void *>(
			const_cast<counted_ptr_val const *>(this)->get_extra(sz)
		);
	}

	void access_allocator(void *data) const
	{
		if (extra_disp && extra_disp->access_allocator)
			extra_disp->access_allocator(this, data);
		else {
			counted_ptr_alloc_wrapper<
				counted_ptr_val, std::allocator<void>
			> aw(std::allocator<void>{});

			aw.access_allocator(this, data);
		}
	}

	void release()
	{
		auto c(use_count.fetch_sub(1));
		if (c == 1)
			destroy(this);
	}

	void add_ref_copy()
	{
		use_count.fetch_add(1);
	}

	unsigned long get_use_count() const
	{
		return use_count.load();
	}

	size_t get_alloc_size() const
	{
		if (extra_disp)
			return extra_disp->get_alloc_size(this);
		else
			return sizeof(*this);
	}

private:
	typedef counted_ptr_val container_type;

	template <typename Extra>
	static void access_allocator_a(container_type const *p, void *data)
	{
		auto x_p(reinterpret_cast<Extra const *>(
			p->extra_storage
		));

		x_p->a_ref->access_allocator(p, data);
	}

	template <typename Extra>
	static size_t get_alloc_size(container_type const *p)
	{
		auto x_p(reinterpret_cast<Extra const *>(
			p->extra_storage
		));
		return x_p->t_size;
	}

	static std::tuple<
		void const *, size_t
	> get_extra_ae(container_type const *p)
	{
		auto x_p(reinterpret_cast<counted_ptr_extra_ae const *>(
			p->extra_storage
		));

		return std::make_pair<void const *, size_t>(
			p->extra_storage + x_p->extra_offset,
			x_p->t_size
			- offsetof(container_type, extra_storage)
			- x_p->extra_offset
		);
	}

	template <typename Extra>
	static void destroy_a(container_type *p)
	{
		auto x_p(reinterpret_cast<Extra *>(
			p->extra_storage
		));

		x_p->a_ref->destroy_a(p);
	}

	static std::tuple<
		void const *, size_t
	> get_extra_b(container_type const *p)
	{
		auto x_p(reinterpret_cast<counted_ptr_extra_b const *>(
			p->extra_storage
		));

		return std::make_tuple<void const *, size_t>(
			reinterpret_cast<void const *>(
				p->extra_storage + counted_ptr_extra_b::h_size
			),
			x_p->t_size
			- offsetof(container_type, extra_storage)
			- counted_ptr_extra_b::h_size
		);
	}

	static constexpr counted_ptr_disp<ValueType> disp_a = {
		.access_allocator = &access_allocator_a<counted_ptr_extra_a>,
		.get_extra = nullptr,
		.get_alloc_size = &get_alloc_size<counted_ptr_extra_a>,
		.destroy = &destroy_a<counted_ptr_extra_a>
	};

	static constexpr counted_ptr_disp<ValueType> disp_ae = {
		.access_allocator = &access_allocator_a<counted_ptr_extra_ae>,
		.get_extra = &get_extra_ae,
		.get_alloc_size = &get_alloc_size<counted_ptr_extra_ae>,
		.destroy = &destroy_a<counted_ptr_extra_ae>
	};

	static constexpr counted_ptr_disp<ValueType> disp_b = {
		.access_allocator = nullptr,
		.get_extra = &get_extra_b,
		.get_alloc_size = &get_alloc_size<counted_ptr_extra_b>,
		.destroy = nullptr
	};

	template <typename Alloc>
	static void construct_alloc(
		Alloc const &a, container_type *p, size_t a_size
	);

	typename std::aligned_storage<
		sizeof(ValueType),
		std::alignment_of<ValueType>::value
	>::type val;
	std::atomic_ulong use_count;
	counted_ptr_disp<ValueType> const *extra_disp;
	uint8_t extra_storage[];
};

template <typename ValueType>
template <typename Alloc, typename... Args>
auto counted_ptr_val<ValueType>::construct(
	Alloc const &a, size_t extra, Args&&... args
) -> container_type *
{
	typedef counted_ptr_alloc_wrapper<counted_ptr_val, Alloc> aw_t;
	typedef typename std::aligned_storage<
		sizeof(aw_t), std::alignment_of<aw_t>::value
	>::type aw_st;

	auto a_size(sizeof(counted_ptr_val));
	counted_ptr_disp<ValueType> const *disp(nullptr);
	bool custom_alloc(!std::is_same<
		typename std::allocator_traits<
			Alloc
		>::template rebind_alloc<void>,
		std::allocator<void>
	>::value);

	if (custom_alloc) {
		a_size += sizeof(aw_st);

		if (extra) {
			a_size += counted_ptr_extra_ae::h_size;
			a_size += extra;
			disp = &disp_ae;
		} else {
			a_size += counted_ptr_extra_a::h_size;
			disp = &disp_a;
		}
	} else {
		if (extra) {
			a_size += counted_ptr_extra_b::h_size;
			a_size += extra;
			disp = &disp_b;
		}
	}

	typename aw_t::byte_allocator_type byte_alloc(a);
	auto p(reinterpret_cast<counted_ptr_val *>(
		aw_t::byte_allocator_traits::allocate(byte_alloc, a_size)
	));

	p->use_count = 1;
	p->extra_disp = disp;

	construct_alloc(a, p, a_size);

	std::unique_ptr<
		container_type, std::function<void (container_type *p)>
	> up(p, [&a, a_size](container_type *p) -> void {

		typename aw_t::allocator_type aw_alloc(a);

		if (p->extra_disp == &disp_a) {
			auto a_ptr(reinterpret_cast<aw_t *>(
				p->extra_storage
				+ counted_ptr_extra_a::h_size
			));
			aw_t::allocator_traits::destroy(aw_alloc, a_ptr);
		} else if (p->extra_disp == &disp_ae) {
			auto a_ptr(reinterpret_cast<aw_t *>(
				p->extra_storage
				+ counted_ptr_extra_ae::h_size
			));
			aw_t::allocator_traits::destroy(aw_alloc, a_ptr);
		}

		typename aw_t::byte_allocator_type byte_alloc(a);

		aw_t::byte_allocator_traits::deallocate(
			byte_alloc,
			reinterpret_cast<uint8_t *>(p),
			a_size
		);
	});

	typename aw_t::value_allocator_type value_alloc(a);
	aw_t::value_allocator_traits::construct(
		value_alloc, reinterpret_cast<ValueType *>(&p->val),
		std::forward<Args>(args)...
	);

	return up.release();
}

template <typename ValueType>
template <typename Alloc>
void counted_ptr_val<ValueType>::construct_alloc(
	Alloc const &a, container_type *p, size_t a_size
)
{
	typedef counted_ptr_alloc_wrapper<counted_ptr_val, Alloc> aw_t;

	typedef typename std::aligned_storage<
		sizeof(aw_t), std::alignment_of<aw_t>::value
	>::type aw_st;

	std::unique_ptr<
		container_type, std::function<void (container_type *p)>
	> up(p, [&a, a_size](container_type *p) -> void {
		typename aw_t::byte_allocator_type byte_alloc(a);	
		aw_t::byte_allocator_traits::deallocate(
			byte_alloc,
			reinterpret_cast<uint8_t *>(p),
			a_size
		);
	});

	typename aw_t::allocator_type aw_alloc(a);

	if (p->extra_disp == &disp_a) {
		auto e_ptr(reinterpret_cast<counted_ptr_extra_a *>(
			p->extra_storage
		));
		auto a_ptr(reinterpret_cast<aw_t *>(
			p->extra_storage + counted_ptr_extra_a::h_size
		));
		e_ptr->t_size = a_size;
		aw_t::allocator_traits::construct(
			aw_alloc, a_ptr, a
		);
		e_ptr->a_ref = dynamic_cast<counted_ptr_alloc_base *>(a_ptr);
	} else if (p->extra_disp == &disp_ae) {
		auto e_ptr(reinterpret_cast<counted_ptr_extra_ae *>(
			p->extra_storage
		));
		auto a_ptr(reinterpret_cast<aw_t *>(
			p->extra_storage + counted_ptr_extra_ae::h_size
		));
		e_ptr->t_size = a_size;
		e_ptr->extra_offset
		= counted_ptr_extra_ae::h_size + sizeof(aw_st);
		aw_t::allocator_traits::construct(
			aw_alloc, a_ptr, a
		);
		e_ptr->a_ref = dynamic_cast<counted_ptr_alloc_base *>(a_ptr);
	} else if (p->extra_disp == &disp_b) {
		auto e_ptr(reinterpret_cast<counted_ptr_extra_b *>(
			p->extra_storage
		));
		e_ptr->t_size = a_size;
	}

	up.release();
}

template <typename ValueType>
constexpr counted_ptr_disp<ValueType> counted_ptr_val<ValueType>::disp_a;

template <typename ValueType>
constexpr counted_ptr_disp<ValueType> counted_ptr_val<ValueType>::disp_ae;

template <typename ValueType>
constexpr counted_ptr_disp<ValueType> counted_ptr_val<ValueType>::disp_b;

}
}}
#endif

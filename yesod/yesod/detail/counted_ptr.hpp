/*
 * Copyright (c) 2010 - 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800)
#define UCPF_YESOD_DETAIL_COUNTED_PTR_OCT_31_2013_1800

#include <tuple>
#include <atomic>
#include <cstddef>
#include <typeinfo>

namespace ucpf { namespace yesod { namespace detail {

template <typename ValueType> struct counted_ptr_val;

template <typename ValueType>
struct counted_ptr_disp {
	size_t (*get_alloc_size)(counted_ptr_val<ValueType> *p);
	void (*destroy)(counted_ptr_val<ValueType> *p);
};

struct counted_ptr_alloc_base {
	virtual ~counted_ptr_alloc_base()
	{}

	virtual void destroy_a(void *p) = 0;
	virtual void destroy_t(void *p) = 0;
};

template <typename StorageType, typename Alloc>
struct counted_ptr_alloc_wrap : counted_ptr_alloc_base, Alloc {
	typedef counted_ptr_alloc_wrap           alloc_wrap_type;
	typedef StorageType                      storage_type;
	typedef typename StorageType::value_type value_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		alloc_wrap_type
	> allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		alloc_wrap_type
	> allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		value_type
	> value_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		value_type
	> value_allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		storage_type
	> storage_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		storage_type
	> storage_allocator_traits;

	typedef typename std::allocator_traits<Alloc>::template rebind_alloc<
		uint8_t
	> byte_allocator_type;

	typedef typename std::allocator_traits<Alloc>::template rebind_traits<
		uint8_t
	> byte_allocator_traits;

	counted_ptr_alloc_wrap(Alloc const &a_)
	: Alloc(a_)
	{}

	virtual ~counted_ptr_alloc_wrap()
	{}

	virtual void destroy_a(void *p)
	{
		auto x_p(reinterpet_cast<storage_type *>(p));
		auto x_value(x_p->get_ptr());
		auto sz(x_p->get_alloc_size(x_p));

		value_allocator_type value_alloc(*this);
		allocator_type self_alloc(*this);
		storage_allocator_type storage_alloc(*this);
		byte_allocator_type byte_alloc(*this);

		value_allocator_traits::destroy(value_alloc, x_value);
		allocator_traits::destroy(self_alloc, this);
		storage_allocator_traits::destroy(storage_alloc, x_p);

		byte_allocator_traits::deallocate(
			byte_alloc, reinterpet_cast<uint8_t *>(p), sz
		);
	}

	virtual void destroy_t(void *p, size_t sz)
	{
		auto x_p(reinterpet_cast<storage_type *>(p));
		auto x_value(x_p->get_ptr());
		auto sz(x_p->get_alloc_size(x_p));

		value_allocator_type value_alloc(*this);
		storage_allocator_type storage_alloc(*this);
		byte_allocator_type byte_alloc(*this);

		value_allocator_traits::destroy(value_alloc, x_value);
		storage_allocator_traits::destroy(storage_alloc, x_p);

		byte_allocator_traits::deallocate(
			byte_alloc, reinterpet_cast<uint8_t *>(p), sz
		);
	}
};

struct counted_ptr_extra_a {
	static constexpr size_t h_size = sizeof(std::aligned_storage<
		sizeof(counted_ptr_extra_a),
		std::alignment_of<counted_ptr_extra_a>::value
	>);
	size_t t_size;
	counted_ptr_alloc_base *a_ref;
};

struct counted_ptr_extra_ae {
	static constexpr size_t h_size = sizeof(std::aligned_storage<
		sizeof(counted_ptr_extra_ae),
		std::alignment_of<counted_ptr_extra_ae>::value
	>);
	size_t t_size;
	size_t e_offset;
	counted_ptr_alloc_base *a_ref;
};

struct counted_ptr_extra_b {
	static constexpr size_t h_size = sizeof(std::aligned_storage<
		sizeof(counted_ptr_extra_b),
		std::alignment_of<counted_ptr_extra_b>::value
	>);
	size_t t_size;
	size_t e_offset;
};

template <typename ValueType>
struct counted_ptr_val {
	typedef ValueType value_type;

	counted_ptr_val(counted_ptr_val const &) = delete;
	counted_ptr_val &operator=(counted_ptr_val const &) = delete;

	static counted_ptr_val *get_this(ValueType *ptr)
	{
		return ptr.load() - offsetof(counted_ptr_val, val);
	}

	template <typename Alloc, typename... Args>
	static counted_ptr_val *construct(
		Alloc const &a, size_t extra, Args&&... args
	)
	{
		typedef counted_ptr_alloc_wrap<
			counted_ptr_val, Alloc
		> aw_t;

		auto a_size(sizeof counted_ptr_val);
		counted_ptr_disp *disp(nullptr);

		if (!std::is_same<Alloc, std::allocator<void>>::value) {
			a_size += sizeof(std::aligned_storage<
				sizeof(Alloc),
				std::alignment_of<Alloc>::value
			>);

			if (extra) {
				a_size += counted_ptr_extra_ae::h_size;
				a_size += extra;
				disp = &counted_ptr_disp_ea;
			} else {
				a_size += counted_ptr_extra_a::h_size;
				disp = &counted_ptr_disp_a;
			}
		} else {
			if (extra) {
				a_size += counted_ptr_extra_b::h_size;
				a_size += extra;
				disp = &counted_ptr_disp_b;
			}
		}

		aw_t::byte_allocator_type byte_alloc(a);
		int restore_mode(0);

		std::unique_ptr<
			counted_ptr_val, std::function<void (counted_ptr_val *)>
		> r_ptr(
			reinterpet_cast<counted_ptr_val *>(
				aw_t::byte_allocator_traits::allocate(
					byte_alloc, a_size
				)
			), [](counted_ptr_val *p) -> void {
			}
		);
	}

	static void destroy(counted_ptr_val *v_ptr)
	{
		if (extra_disp)
			extra_disp->destroy(extra_storage);
		else {
			counted_ptr_alloc_wrap<
				counted_ptr_val, std::allocator<void>
			> aw(std::allocator<void>());

			aw.destroy_t(v_ptr);
		}
	}

	counted_ptr_val()
	: use_count(1)
	{}

	ValueType *get_ptr() const
	{
		return reinterpet_cast<ValueType *>(&val);
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
			extra_disp->get_alloc_size(this);
		else
			return sizeof(*this);
	}

private:
	typedef counted_ptr_val storage_type;

	static void destroy_a(storage_type *p)
	{
		auto x_a(reinterpet_cast<counted_ptr_extra_a *>(
			p->extra_storage
		));
		x_a->a_ref->destroy_a(p);
	}

	static void destroy_ae(storage_type *p)
	{
		auto x_a(reinterpet_cast<counted_ptr_extra_ae *>(
			p->extra_storage
		));
		x_a->a_ref->destroy_a(p);
	}

	static void destroy_b(storage_type *p)
	{
		auto x_a(reinterpet_cast<counted_ptr_extra_ae *>(
			p->extra_storage
		));

		counted_ptr_alloc_wrap<
			counted_ptr_val, std::allocator<void>
		> aw(std::allocator<void>());

		aw.destroy_t(v_ptr);
	}

	static constexpr counted_ptr_disp<ValueType> counted_ptr_disp_a = {
		.destroy = &destroy_a
	};

	static constexpr counted_ptr_disp<ValueType> counted_ptr_disp_ae = {
		.destroy = &destroy_ae
	};

	static constexpr counted_ptr_disp<ValueType> counted_ptr_disp_b = {
		.destroy = &destroy_b
	};

	std::aligned_storage<
		sizeof(ValueType),
		std::alignment_of<ValueType>::value
	> val;
	std::atomic_ulong use_count;
	counted_ptr_disp<ValueType> *extra_disp;
	uint8_t extra_storage[];
};

}}}
#endif

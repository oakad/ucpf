/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_5C4DD8F84769EE6834EE55981DA64A55)
#define HPP_5C4DD8F84769EE6834EE55981DA64A55

namespace ucpf { namespace yesod { namespace storage {
namespace detail {

template <typename ElementType, typename StorageType>
struct alloc_guard {
	alloc_guard(StorageType const &st_, std::size_t count_)
	: st(st_), count(count_ * sizeof(ElementType)), release(true),
	  p(st.allocate(count, alignof(ElementType)))
	{}

	~alloc_guard()
	{
		if (!release)
			return;

		st.deallocate(p, count, alignof(ElementType));
	}

	StorageType const &st;
	std::size_t count;
	bool release;
	typename StorageType::storage_pointer p;
};

template <typename ElementType, typename StorageType>
struct construct_guard {
	construct_guard(
		StorageType const &st_,
		typename StorageType::storage_pointer &p_,
		std::size_t count_
	)
	: st(st_), count(count_), pos(0), release(true), p(p_)
	{}

	~construct_guard()
	{
		if (!release || !pos)
			return;

		--pos;
		auto q(st.template storage_cast<ElementType>(p) + pos);
		while (true) {
			st.destroy(q);
			if (!pos)
				return;
			--pos;
			--q;
		}
	}

	StorageType const &st;
	std::size_t count;
	std::size_t pos;
	bool release;
	typename StorageType::storage_pointer &p;
};

}

template <typename ElementType, typename StorageType, typename... Args>
auto alloc(StorageType const &st, Args&&... args)
{
	detail::alloc_guard<ElementType, StorageType> a_g(st, 1);
	auto rv(st.construct(a_g.p, std::forward<Args>(args)...));
	a_g.release = false;
	return rv;
}

template <typename ElementType, typename StorageType, typename... Args>
auto alloc_n(StorageType const &st, std::size_t count, Args&&... args)
{
	detail::alloc_guard<ElementType, StorageType> a_g(st, count);
	detail::construct_guard<ElementType, StorageType> c_g(
		st, a_g.p, count
	);

	auto rv(st.template construct<ElementType>(
		a_g.p, std::forward<Args>(args)...)
	);
	auto q(a_g.p);

	for (++c_g.pos; c_g.pos < count; ++c_g.pos) {
		q += sizeof(ElementType);
		st.template construct<ElementType>(
			q, std::forward<Args>(args)...
		);
	}

	c_g.release = false;
	a_g.release = false;
	return rv;
}

template <typename ElementType, typename StorageType, typename Iterator>
auto alloc_copy(StorageType const &st, std::size_t count, Iterator first)
{
	detail::alloc_guard<ElementType, StorageType> a_g(st, count);
	detail::construct_guard<ElementType, StorageType> c_g(
		st, a_g.p, count
	);

	auto rv(st.template construct<ElementType>(a_g.p, *first));
	auto q(a_g.p);

	for (++c_g.pos; c_g.pos < count; ++c_g.pos) {
		q += sizeof(ElementType);
		++first;
		st.template construct<ElementType>(q, *first);
	}

	c_g.release = false;
	a_g.release = false;
	return rv;
}

template <typename StorageType, typename Pointer>
void free(StorageType const &st, Pointer p)
{
	typedef typename Pointer::element_type element_type;
	auto pp(st.storage_cast(p));
	st.destroy(p);
	st.deallocate(pp, sizeof(element_type), alignof(element_type));
}

template <typename StorageType, typename Pointer>
void free_n(
	StorageType const &st, Pointer p,
	typename StorageType::size_type count
)
{
	typedef typename Pointer::element_type element_type;
	auto pp(st.storage_cast(p));
	auto c(count);
	p += c - 1;
	for (; c; --c) {
		st.destroy(p);
		--p;
	}

	st.deallocate(pp, sizeof(element_type) * count, alignof(element_type));
}

}}}
#endif

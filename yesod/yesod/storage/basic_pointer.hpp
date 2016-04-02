/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F280719E8C1284BC4844DC65C2D1704E)
#define HPP_F280719E8C1284BC4844DC65C2D1704E

namespace ucpf { namespace yesod { namespace storage {

template <typename ValueType, typename StorageFacadeType, bool IsConst>
struct basic_pointer;

namespace detail {

template <typename StorageFacadeType, bool stateful = true>
struct basic_pointer_address {
	StorageFacadeType *storage() const
	{
		return sfp;
	}

	typename StorageFacadeType::address_type value;
	StorageFacadeType *sfp;
};

template <typename StorageFacadeType>
struct basic_pointer_address<StorageFacadeType, false> {
	static StorageFacadeType sfp;

	StorageFacadeType *storage() const
	{
		return &sfp;
	}

	typename StorageFacadeType::address_type value;
};

template <typename StorageFacadeType>
StorageFacadeType basic_pointer_address<StorageFacadeType, false>::sfp;

template <typename StorageFacadeType, bool stateful = true>
struct basic_pointer_access {
	template <typename ValueType, bool IsConst>
	static void assign(
		basic_pointer<ValueType, StorageFacadeType, IsConst> &p,
		typename StorageFacadeType::address_type addr,
		StorageFacadeType *sfp
	)
	{
		p.addr.value = addr;
		p.addr.sfp = sfp;
	}

	template <typename ValueType, bool IsConst>
	static  typename StorageFacadeType::address_type address(
		basic_pointer<ValueType, StorageFacadeType, IsConst> p
	)
	{
		return p.addr.value;
	}
};

template <typename StorageFacadeType>
struct basic_pointer_access<StorageFacadeType, false> {
	template <typename ValueType, bool IsConst>
	static void assign(
		basic_pointer<ValueType, StorageFacadeType, IsConst> &p,
		typename StorageFacadeType::address_type addr,
		StorageFacadeType *sfp
	)
	{
		p.addr.value = addr;
	}

	template <typename ValueType, bool IsConst>
	static  typename StorageFacadeType::address_type address(
		basic_pointer<ValueType, StorageFacadeType, IsConst> const &p
	)
	{
		return p.addr.value;
	}
};

}

template <typename ElementType, typename StorageFacadeType, bool IsConst>
struct basic_pointer {
	typedef std::remove_cv_t<ElementType> element_type;
	typedef typename std::conditional<
		IsConst, element_type const &, element_type &
	>::type reference_type;
	typedef typename std::conditional<
		IsConst, element_type const *, element_type *
	>::type pointer_type;
	typedef typename StorageFacadeType::size_type size_type;
	typedef typename StorageFacadeType::difference_type difference_type;

	template <typename StorageFacadeType_, bool stateful>
	friend struct detail::basic_pointer_access;

	explicit operator bool() const
	{
		return addr.value ? true : false;
	}

	reference_type operator*()
	{
		return *reinterpret_cast<pointer_type>(addr.storage()->access(
			addr.value, sizeof(element_type)
		));
	}

	pointer_type operator->()
	{
		return reinterpret_cast<pointer_type>(addr.storage()->access(
			addr.value, sizeof(element_type)
		));
	}

	basic_pointer operator++(int)
	{
		basic_pointer rv;
		pointer_access::assign(
			rv, addr.storage()->access(
				addr.value, sizeof(element_type), 1
			), addr.storage()
		);
		return rv;
	}

	basic_pointer operator--(int)
	{
		basic_pointer rv;
		pointer_access::assign(
			rv, addr.storage()->access(
				addr.value, sizeof(element_type), -1
			), addr.storage()
		);
		return rv;
	}

	basic_pointer &operator--()
	{
		addr.value = addr.storage()->access(
			addr.value, sizeof(element_type), -1
		);
		return *this;
	}

	basic_pointer &operator+=(difference_type n)
	{
		addr.value = addr.storage()->access(
			addr.value, sizeof(element_type), n
		);
		return *this;
	}

	basic_pointer &operator-=(difference_type n)
	{
		addr.value = addr.storage()->access(
			addr.value, sizeof(element_type), -n
		);
		return *this;
	}

	basic_pointer operator+(difference_type n)
	{
		basic_pointer rv;
		pointer_access::assign(
			rv, addr.storage()->access(
				addr.value, sizeof(element_type), n
			), addr.storage()
		);
		return rv;
	}

private:
	typedef detail::basic_pointer_access<
		StorageFacadeType, StorageFacadeType::is_stateful
	> pointer_access;

	detail::basic_pointer_address<
		StorageFacadeType, StorageFacadeType::is_stateful
	> addr;
};

}}}
#endif

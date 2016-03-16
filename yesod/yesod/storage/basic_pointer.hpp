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
	StorageFacadeType &storage() const
	{
		return *sfp;
	}

	typename StorageFacadeType::address_type value;
	StorageFacadeType *sfp;
};

template <typename StorageFacadeType>
struct basic_pointer_address<StorageFacadeType, false> {
	static StorageFacadeType sfp;

	StorageFacadeType &storage() const
	{
		return sfp;
	}

	typename StorageFacadeType::address_type value;
};

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
		basic_pointer<ValueType, StorageFacadeType, IsConst> p
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
	static constexpr size_type value_size = sizeof(std::aligned_storage_t<
		sizeof(element_type), alignof(element_type)
	>);

	template <typename StorageFacadeType_, bool stateful>
	friend struct detail::basic_pointer_access;

	explicit operator bool() const
	{
		return addr.value ? true : false;
	}

	reference_type operator*()
	{
		printf("op2\n");
		return *reinterpret_cast<pointer_type>(addr.storage().access(
			addr.value, value_size
		));
	}

	pointer_type operator->()
	{
		printf("op3\n");
		return reinterpret_cast<pointer_type>(addr.storage().access(
			addr.value, value_size
		));
	}

private:
	detail::basic_pointer_address<
		StorageFacadeType, StorageFacadeType::is_stateful
	> addr;
};

}}}
#endif

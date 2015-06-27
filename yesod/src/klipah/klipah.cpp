/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <yesod/bitops.hpp>
#include <yesod/allocator/system.hpp>

#include "buddy_alloc.hpp"

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {

struct alloc_access {
	void *allocate(std::size_t sz_)
	{
		auto sz(normalize_size(sz_));
		auto a_ord(yesod::ffs(sz) - 1);

		if (a_ord >= max_medium_size_order) {
			//return system::allocate(sz);
			printf("big alloc %zd\n", sz);
			return nullptr;
		} else if (a_ord >= max_small_size_order)
			return allocate_medium(sz);
		else {
			auto x_cls(((sz >> a_ord) - 1) >> 1);
			return small_size_classes::value[x_cls]->allocate(
				a_ord
			);
		}
	}

	void deallocate(void *p, std::size_t sz_)
	{
		auto sz(normalize_size(sz_));
		auto a_ord(yesod::ffs(sz) - 1);

		if (a_ord >= max_medium_size_order)
			system::deallocate(p, sz);
		else if (a_ord >= max_small_size_order)
			deallocate_medium(p, sz);
		else {
			auto x_cls(((sz >> a_ord) - 1) >> 1);
			small_size_classes::value[x_cls]->deallocate(
				p, a_ord
			);
		}
	}

private:
	constexpr static std::size_t min_size_order = 3;
	constexpr static std::size_t max_small_size_order = 18;
	constexpr static std::size_t small_size_incr_order_delta = 6;
	constexpr static std::size_t medium_size_delta
	= max_small_size_order - small_size_incr_order_delta;
	constexpr static std::size_t max_medium_size_order = 22;
	constexpr static std::size_t small_size_class_cnt = 64;

	constexpr static std::size_t normalize_size(std::size_t sz)
	{
		auto ord(yesod::fls(sz));

		auto r_ord(
			ord <= (min_size_order + small_size_incr_order_delta)
			? min_size_order : (ord - small_size_incr_order_delta)
		);

		return (sz | ((std::size_t(1) << r_ord) - 1)) + 1;
	}

	void *allocate_medium(std::size_t sz)
	{
		printf("medium alloc %zd\n", sz);
		return nullptr;
	}

	void *deallocate_medium(void *p, std::size_t sz)
	{
	}

	template <std::size_t BaseSizeOrd>
	struct buddy_alloc_entry {
		typedef buddy_alloc<BaseSizeOrd * 2 + 1, 3, 1> ba_type;

		static constexpr ba_type ba = {};
		constexpr static buddy_alloc_base const *impl = &ba;
	};

	template <typename T>
	struct array_apply;

	template <typename T, T... Tn>
	struct array_apply<std::integer_sequence<T, Tn...>> {
		static constexpr buddy_alloc_base const * value[
			sizeof...(Tn)
		] = {
			buddy_alloc_entry<Tn>::impl...
		};
	};

	typedef array_apply<std::make_integer_sequence<
		std::size_t, small_size_class_cnt
	>> small_size_classes;
};

template <std::size_t BaseSizeOrd>
constexpr typename alloc_access::buddy_alloc_entry<BaseSizeOrd>::ba_type
alloc_access::buddy_alloc_entry<BaseSizeOrd>::ba;

template <typename T, T... Tn>
constexpr buddy_alloc_base const * alloc_access::array_apply<
	std::integer_sequence<T, Tn...>
>::value[sizeof...(Tn)];

}}}}

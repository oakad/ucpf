/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_15FA41FAE8EC6D4B507276C17D145CD8)
#define HPP_15FA41FAE8EC6D4B507276C17D145CD8

#include <yesod/stored_string.hpp>

namespace ucpf { namespace yesod { namespace string_map {
namespace detail {

template <
	typename ValueType, typename KeyCodecType, typename StoreType,
	typename StorageType, typename StoreConfig, typename MapConfig
>
struct mutable_map_impl {
	struct iterator {
	};

	struct const_iterator {
	};

	iterator end()
	{
		return iterator{};
	}

	const_iterator cend()
	{
		return const_iterator{};
	}

	template <typename KeyIterator>
	auto find(
		KeyIterator first, KeyIterator const &last
	) const -> const_iterator;

	template <typename KeyIterator>
	auto find(
		KeyIterator first, KeyIterator const &last
	) -> iterator
	{
		return iterator(
			const_cast<mutable_map_impl const *>(this)->find(
				first, last
			)
		);
	}

	template<typename KeyIterator, typename Args...>
	auto emplace(
		KeyIterator first, KeyIterator const &last, Args &&...args
	) -> std::pair<iterator, bool>;

protected:
	struct default_config {
	};

	typedef std::conditional<
		std::is_same<ConfigType, void>::value,
		default_config, MapConfig
	>::type config_type;

	mutable_map_impl()
	: root_offset(0)
	{}

	mutable_map_impl(StorageType const &st)
	: root_offset(0), store(st)
	{}

	mutable_map_impl(KeyCodecType const &kc, StorageType const &st)
	: root_offset(0), codec(kc), store(st)
	{}

	typedef typename StorageType::size_type size_type;
	static constexpr invalid_pos = std::numeric_limits<size_type>::max();

	struct node {
		enum KeyMatchState {
			MISMATCH = 0,
			PARTIAL,
			MATCH
		};

		node()
		: base_offset(invalid_pos)
		{}

		virtual bool has_value() const
		{
			return false;
		}

		template <typename KeyIterator>
		KeyMatchState match_key(
			KeyIterator &first, KeyIterator const &last
		) const;

		size_type base_offset;
		stored_string<KeyCodecType::char_type, StorageType> key;

	protected:
		~node()
		{}
	};

	struct value_node : node {
		template <typename Args...>
		value_node(Args &&...args)
		: value(std::forward<Args>(args)...)
		{}

		virtual bool has_value() const
		{
			return true;
		}

		ValueType value;

	protected:
		~value_node()
		{}
	};

	typedef typename StorageType::template pointer<node> node_ptr_type;

	struct cell {
		enum ChildType {
			NAC = 0,
			EDGE,
			NODE
		};

		cell()
		: base_offset(invalid_pos), check(0)
		{}

		ChildType child_of(size_type pos) const
		{
			if ((check >> 1) != pos)
				return ChildType::NAC;

			if (check & 1)
				return ChildType::NODE;

			if (base_offset == invalid_pos)
				return ChildType::EDGE;

			return ChildType::NAC;
		}

		union {
			size_type base_offset;
			node_ptr_type base_ptr;
			
		};
		size_type check;
	};

	size_type root_offset;
	StoreType::template impl_type<
		cell, StorageType, StoreConfig
	> store;
	KeyCodecType codec;
};
}
}}}
#endif

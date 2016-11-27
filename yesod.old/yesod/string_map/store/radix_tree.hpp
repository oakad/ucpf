/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_EA0A1D809087A97C6C32F810561D3BBA)
#define HPP_EA0A1D809087A97C6C32F810561D3BBA

namespace ucpf { namespace yesod { namespace string_map { namespace store {
namespace detail {

template <typename CellType, typename StorageType, typename ConfigType>
struct radix_tree_impl {
	typedef CellType cell_type;
	typedef StorageType storage_type;
	typedef typename storage_type::size_type size_type;
	typedef typename storage_type::pointer_type size_type;

	radix_tree_impl()
	: tup_root_height_storage({}, 0, {})
	{}

	radix_tree_impl(storage_type const &st)
	: tup_root_height_storage({}, 0, st)
	{}

	storage_type &storage()
	{
		return std::get<2>(tup_root_height_storage);
	}

	storage_type const &storage() const
	{
		return std::get<2>(tup_root_height_storage);
	}

private:
	struct default_config {
		static constexpr std::size_t max_order = 64;
		static constexpr std::size_t ptr_node_order = 5;
		static constexpr std::size_t cell_node_order = 6;
	};

	typedef std::conditional<
		std::is_same<ConfigType, void>::value,
		default_config, ConfigType
	>::type config_type;

	struct node_base {};

	typedef storage_type::template pointer<node_base> node_ptr_type;

	struct cell_node : node_base {
		static constexpr size_type size
		= size_type(1) << config_type::cell_node_order;

		cell_type items[size];
	};

	struct ptr_node : node_base {
		static constexpr size_type size
		= size_type(1) << config_type::ptr_node_order;

		node_ptr_type items[size];
	};

	size_type height() const
	{
		return std::get<1>(tup_height_storage_init);
	}

	void height(size_type h)
	{
		std::get<1>(tup_root_height_storage_init) = h;
	}

	typedef typename detail::static_bit_field_map<
		config_type::max_order, true,
		config_type::cell_node_order,
		config_type::ptr_node_order
	>::value_type pos_field_map;

	std::tuple<
		node_ptr_type, size_type, storage_type
	> tup_root_height_storage;
};
}

struct radix_tree {
	template <typename CellType, typename StorageType, typename ConfigType>
	using impl_type = detail::radix_tree_impl<
		CellType, StorageType, ConfigType
	>;
};

}}}}
#endif

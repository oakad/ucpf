/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UBB_MEMFD_DEC_05_2013_1545)
#define UBB_MEMFD_DEC_05_2013_1545

#include <ext/string_map.hpp>
#include <cstring>
#include <mutex>
#include <map>

namespace ubb {

class memfd {
	struct data_node {
		data_node(size_t size_)
		: size(size_)
		{}

		virtual ~data_node()
		{}

		virtual size_t read(char *buf, size_t count, off_t offset) const
		{
			return 0;
		}

		virtual size_t write(
			char const *buf, size_t count, off_t offset
		)
		{
			return 0;
		}

		virtual void truncate(off_t length)
		{}

		size_t size;
	};

	struct ro_data_node : data_node {
		ro_data_node(char const *data_, size_t size_)
		: data_node(size_), data(data_)
		{}

		virtual ~ro_data_node()
		{}

		virtual size_t read(char *buf, size_t count, off_t offset) const
		{
			auto e_off(std::min(size_t(offset), size));
			auto e_sz(std::min(count, size - e_off));
			if(e_sz) 
				std::memcpy(buf, data + e_off, e_sz);

			return e_sz;
		}

		char const *data;
	};

	struct rw_data_node : data_node {
		rw_data_node()
		: data_node(0), block_order(12)
		{}

		virtual size_t read(
			char *buf, size_t count, off_t offset
		) const;
		virtual size_t write(
			char const *buf, size_t count, off_t offset
		);
		virtual void truncate(off_t length);

		int block_order;
		mutable std::mutex lock;
		std::map<off_t, char *> data;
	};

	static struct memfd_base {
		typedef std::shared_ptr<data_node> node_type;

		node_type open(std::string const &name)
		{
			std::lock_guard<std::mutex> l_g(lock);
			node_type *rv(files.find(name));
			if (rv)
				return *rv;
			else
				return node_type();
		}

		bool create(std::string const &name, node_type &node_)
		{
			std::lock_guard<std::mutex> l_g(lock);
			auto v(files.at(name));
			if (!v) {
				v.swap(node_);
				return true;
			} else
				return false;
		}

		std::mutex lock;
		ext::string_map<char, node_type> files;
	} base;

	memfd_base::node_type node;

public:
	template <typename T, bool ReadOnly>
	static memfd create(std::string const &name, T const &data_)
	{
		memfd rv;
		memfd_base::node_type n_data;

		if (ReadOnly) {
			auto s_data(std::make_shared<ro_data_node>(
				data_.data, data_.size()
			));
			n_data = std::dynamic_pointer_cast<data_node>(s_data);
		} else {
			auto s_data(std::make_shared<rw_data_node>());
			s_data->write(data_.data, data_.size, 0);
			n_data = std::dynamic_pointer_cast<data_node>(s_data);
		}

		if (base.create(name, n_data))
			rv.node = n_data;
		return rv;
	}

	static memfd create(std::string const &name)
	{
		memfd rv;
		auto n_data(std::dynamic_pointer_cast<data_node>(
			std::make_shared<rw_data_node>()
		));
		if (base.create(name, n_data))
			rv.node = n_data;

		return rv;
	}

	static void remove(std::string const &name)
	{
		std::lock_guard<std::mutex> l_g(base.lock);
		base.files.remove(name);
	}

	memfd()
	{}

	memfd(std::string const &name)
	: node(base.open(name))
	{}

	size_t read(void *buf, size_t count, off_t offset) const
	{
		return node ? node->read(
			reinterpret_cast<char *>(buf), count, offset
		) : 0;
	}

	size_t write(void const *buf, size_t count, off_t offset)
	{
		return node ? node->write(
			reinterpret_cast<char const *>(buf), count, offset
		) : 0;
	}

	void truncate(off_t length)
	{
		if (node)
			node->truncate(length);
	}

	struct stat {
		size_t size;
		bool read_only;
		bool exists;
	};

	stat fstat() const
	{
		stat rv({0, (bool)node, true});

		if (rv.exists) {
			auto x_node(std::dynamic_pointer_cast<rw_data_node>(
				node
			));
			if (x_node) {
				std::lock_guard<std::mutex> l_g(x_node->lock);
				rv.size = node->size;
				rv.read_only = false;
			} else
				rv.size = node->size;
		}
		return rv;
	}
};

}

#endif
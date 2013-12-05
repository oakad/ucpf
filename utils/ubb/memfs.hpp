/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UBB_MEMFS_DEC_05_2013_1545)
#define UBB_MEMFS_DEC_05_2013_1545

namespace ubb {

struct memfd {
	template <typename T, bool ReadOnly>
	static memfd create(std::string const &name, T const &data_)
	{
		memfd rv;

		if (ReadOnly) {
			auto n_data(std::make_shared<ro_data_node>(
				data_.data, data_.size()
			));
			if (base.add_node(name, n_data))
				rv.node = n_data;
		} else {
			auto n_data(std::make_shared<rw_data_node>(
				data_.size()
			));
			n_data->write(data_.data, data_.size, 0);
			if (base.add_node(name, n_data))
				rv.node = n_data;
		}
		return rv;
	}

	static memfd create(std::string const &name)
	{
		auto n_data(std::make_shared<rw_data_node>(
			data_.size()
		));
		n_data->write(data_.data, data_.size, 0);
		rv.node = n_data;
	}

	memfd()
	{}

	memfd(std::string const &name)
	: node(base::get_node(name))
	{}

	size_t read(void *buf, size_t count, off_t offset);
	size_t write(void const *buf, size_t count, off_t offset);

private:
	struct data_node {
		virtual ~data_node()
		{}
	};

	struct ro_data_node : data_node {
		ro_data_node(char const *data_, size_t size_)
		: data(data_), size(size_)
		{}

		virtual ~ro_data_node()
		{}

		char const *data;
		size_t size;
	};

	struct rw_data_node : data_node {
	};

	static struct memfd_base {
		std::shared_ptr<data_node> get_node(std::string const &name);
		bool add_node(
			std::string const &name,
			std::shared_ptr<data_node> node_
		);

	} base;


	std::shared_ptr<data_node> node;
};

}

#endif
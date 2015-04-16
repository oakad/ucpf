/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_6668D74F286DF3886763E7099335A448)
#define HPP_6668D74F286DF3886763E7099335A448

#include <functional>
#include <system_error>
#include <zivug/arch/io/descriptor.hpp>

namespace ucpf { namespace zivug { namespace io {

struct address_base {
	virtual std::size_t size() const = 0;

	virtual std::size_t data(char *buf) const = 0;

	virtual std::size_t printable(
		std::function<void (char const *, char const *)> &&consumer
	) const = 0;
};

struct address_filter {
	virtual bool accept(address_base const &peer)
	{
		return true;
	}
};

struct address_family {
	constexpr address_family()
	{}

	address_family(address_family const &other) = delete;
	address_family &operator=(address_family const &other) = delete;

	static std::pair<descriptor, address_family const *> make_descriptor(
		char const *first, char const *last
	);

	virtual void set_option(
		descriptor const &d, char const *first, char const *last
	) const;

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const
	{
		throw std::system_error(
			EADDRNOTAVAIL, std::system_category()
		);
	}

	virtual void connect(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const
	{
		throw std::system_error(
			EADDRNOTAVAIL, std::system_category()
		);
	}

	virtual void listen(descriptor const &d, int backlog) const
	{
		throw std::system_error(
			EOPNOTSUPP, std::system_category()
		);
	}

	virtual descriptor accept(
		descriptor const &d, address_filter &flt
	) const
	{
		throw std::system_error(
			EOPNOTSUPP, std::system_category()
		);
	}

	virtual std::size_t addr_size() const
	{
		return 0;
	}

protected:
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const
	{
		throw std::system_error(
			EAFNOSUPPORT, std::system_category()
		);
	}
};

}}}
#endif

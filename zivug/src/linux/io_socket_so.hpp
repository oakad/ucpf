/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_CB93DAD73A115C2F9BF3383000F84803)
#define HPP_CB93DAD73A115C2F9BF3383000F84803

namespace ucpf { namespace zivug { namespace io { namespace detail {

struct string_tag {
};

struct option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;

	virtual int get_int(descriptor const &d) const = 0;
};

template <typename ValueType, int Level, int OptName>
struct option : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}

	virtual int get_int(descriptor const &d) const
	{
		throw std::system_error(
			EINVAL, std::system_category()
		);
	}
};

template <int Level, int OptName>
struct option<int, Level, OptName> : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		using ucpf::mina::detail::from_ascii_decimal_converter;
		int x_val(0);

		if (!from_ascii_decimal_converter<int, false>::parse_signed(
			first, last, x_val
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto rc(::setsockopt(
			d.native(), Level, OptName, &x_val, sizeof(x_val)
		));

		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}

	virtual int get_int(descriptor const &d) const
	{
		int x_val(0);
		::socklen_t x_len(sizeof(x_val));
		auto rc(::getsockopt(
			d.native(), Level, OptName, &x_val, &x_len
		));

		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);

		return x_val;
	}
};

template <int Level, int OptName>
struct option<string_tag, Level, OptName> : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		auto rc(::setsockopt(
			d.native(), Level, OptName, first, last - first
		));

		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

template <typename ValueType, int Level, int OptName>
struct option_entry {
	constexpr static option<ValueType, Level, OptName> sock_option = {};
	constexpr static option_base const *impl = &sock_option;
};

template <typename ValueType, int Level, int OptName>
constexpr option<ValueType, Level, OptName> option_entry<
	ValueType, Level, OptName
>::sock_option;

struct socket_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;
};

template <int Level>
struct socket_level : socket_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

}}}}
#endif

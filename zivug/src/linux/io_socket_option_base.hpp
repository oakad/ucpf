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

template <typename ValueType>
struct ro_tag {
	typedef ValueType value_type;
};

struct option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;
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

struct option_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;
};

template <int Level>
struct option_level : option_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};


template <int Level>
struct option_level_entry {
	constexpr static option_level<Level> sock_option_level = {};
	constexpr static option_level_base const *impl = &sock_option_level;
};

template <int Level>
constexpr option_level<Level> option_level_entry<Level>::sock_option_level;

}}}}
#endif

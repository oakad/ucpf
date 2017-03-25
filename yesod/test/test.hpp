/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_F5B061E5D859AF9ED545F3614F19D749)
#define HPP_F5B061E5D859AF9ED545F3614F19D749

#include <yesod/concurrent/thread.hpp>
#include <yesod/string_utils/charseq_adaptor.hpp>
#include <yesod/string_utils/ascii_hex_formatter.hpp>

#include <iostream>
#include <experimental/tuple>
#include <experimental/string_view>

#include <sys/syscall.h>

namespace ucpf::yesod::test {

typedef std::basic_string<uint8_t> ustring;
typedef std::experimental::basic_string_view<uint8_t> ustring_view;

template <typename Unused = void>
struct tls_diag {
	static std::ostream &dump(std::ostream &os)
	{
		for (size_t c(0); c < instance.pos; ++c)
			os << instance.data[c];

		return os;
	}

	static void reset()
	{
		for (size_t c(0); c < instance.pos; ++c) {
			free(instance.data[c]);
			instance.data[c] = nullptr;
		}
		instance.pos = 0;
	}

	template <typename... Args>
	static void print(char const *format, Args &&...args)
	{
		if (instance.pos >= depth)
			return;
		asprintf(
			&instance.data[instance.pos++], format,
			std::forward<Args>(args)...
		);
	}

private:
	constexpr static size_t depth = 20;
	char *data[depth] = {nullptr};
	size_t pos = 0;
	static thread_local tls_diag instance;
};

template <typename Unused>
thread_local tls_diag<Unused> tls_diag<Unused>::instance;

template <typename... Args>
std::ostream &print_diag(
	std::ostream &os, void const *p, char const *format_, Args &&...args
)
{
	constexpr static char const *format_prefix = "%p[%d]: ";
	auto fsz(strlen(format_));
	char format[9 + fsz];
	memcpy(format, format_prefix, 8);
	memcpy(format + 8, format_, fsz);
	format[8 + fsz] = 0;
	char *data_;
	asprintf(
		&data_, format, p, ::syscall(SYS_gettid),
		std::forward<Args>(args)...
	);
	auto deleter = [](char *p)
	{
		free(p);
	};
	std::unique_ptr<char, decltype(deleter)> data(data_, deleter);
	os << data_;
	return os;
}

static inline uint8_t const *operator ""_us(char const *s, size_t len)
{
	return reinterpret_cast<uint8_t const *>(s);
}

std::ostream &operator<<(std::ostream &os, ustring const &s)
{
	os << reinterpret_cast<char const *>(s.c_str());
	return os;
}

template <typename CharSeq>
std::ostream &print_hex(std::ostream &os, CharSeq &&seq_)
{
	auto seq(string_utils::charseq_adaptor<CharSeq>::apply(
		std::forward<CharSeq>(seq_)
	));
	std::ostream_iterator<char> iter(os);
	string_utils::ascii_hex_formatter<
		decltype(iter), decltype(seq)
	>::apply(iter, std::move(seq));
	return os;
}

template <typename CharSeq>
std::ostream &print_hex(std::ostream &os, CharSeq &&seq_, size_t length)
{
	auto seq(string_utils::charseq_adaptor<CharSeq>::apply(
		std::forward<CharSeq>(seq_), length
	));
	std::ostream_iterator<char> iter(os);
	string_utils::ascii_hex_formatter<
		decltype(iter), decltype(seq)
	>::apply(iter, std::move(seq));
	return os;
}

template <size_t N>
void check_all(std::array<boost::test_tools::assertion_result, N> const &res)
{
	for (auto &r: res)
		BOOST_TEST(r);
}

}

namespace boost::test_tools::tt_detail {

template <>
struct print_log_value<ucpf::yesod::test::ustring> {
	void operator()(std::ostream &os, ucpf::yesod::test::ustring const &s)
	{
		os.write(
			reinterpret_cast<char const *>(s.data()), s.size()
		);
	}
};

template <>
struct print_log_value<ucpf::yesod::test::ustring_view> {
	void operator()(
		std::ostream &os, ucpf::yesod::test::ustring_view const &s
	)
	{
		os.write(
			reinterpret_cast<char const *>(s.data()), s.size()
		);
	}
};

typedef std::tuple<
	::boost::test_tools::assertion_result,
	::boost::unit_test::lazy_ostream,
	::boost::unit_test::const_string,
	std::size_t,
	::boost::test_tools::tt_detail::tool_level,
	::boost::test_tools::tt_detail::check_type,
	std::size_t
> assertion_report_arg_t;

typedef std::vector<
	::boost::test_tools::tt_detail::assertion_report_arg_t
> local_assertion_store_t;

}

#define BOOST_TEST_DECLARE_LOC_STORE()                  \
::boost::test_tools::tt_detail::local_assertion_store_t \
boost_test_assertions                                   \
/**/

#define BOOST_TEST_DECLARE_LOC_STORE_REF()              \
::boost::test_tools::tt_detail::local_assertion_store_t \
&boost_test_assertions                                  \
/**/

#define BOOST_TEST_REPORT_LOCALS()                                          \
do {                                                                        \
	for (auto &t: boost_test_assertions) {                              \
		std::experimental::fundamentals_v1::apply(                  \
			::boost::test_tools::tt_detail::report_assertion, t \
		);                                                          \
	}                                                                   \
	boost_test_assertions.clear();                                      \
} while(::boost::test_tools::tt_detail::dummy_cond())                       \
/**/

#define BOOST_TEST_TOOL_ET_LOC_IMPL(P, level)                          \
do {                                                                   \
	auto res(BOOST_TEST_BUILD_ASSERTION(P).evaluate());            \
	if (res)                                                       \
		break;                                                 \
	boost_test_assertions.emplace_back(                            \
		std::move(res),                                        \
		BOOST_TEST_LAZY_MSG(BOOST_TEST_STRINGIZE(P)),          \
		BOOST_TEST_L(__FILE__),                                \
		static_cast<std::size_t>(__LINE__),                    \
		::boost::test_tools::tt_detail::level,                 \
		::boost::test_tools::tt_detail::CHECK_BUILT_ASSERTION, \
		0                                                      \
	);                                                             \
} while(::boost::test_tools::tt_detail::dummy_cond())                  \
/**/

#define BOOST_TEST_TOOL_ET_LOC_IMPL_EX(P, level, arg)                    \
do {                                                                     \
	auto res(::boost::test_tools::tt_detail::assertion_evaluate(     \
		BOOST_TEST_BUILD_ASSERTION(P)                            \
	) << arg);                                                       \
	if (res)                                                         \
		break;                                                   \
	boost_test_assertions.emplace_back(                              \
		std::move(res),                                          \
		::boost::test_tools::tt_detail::assertion_text(          \
			BOOST_TEST_LAZY_MSG(BOOST_TEST_STRINGIZE(P)),    \
			BOOST_TEST_LAZY_MSG(arg)                         \
		), BOOST_TEST_L(__FILE__),                               \
		static_cast<std::size_t>(__LINE__),                      \
		::boost::test_tools::tt_detail::level,                   \
		::boost::test_tools::tt_detail::assertion_type() << arg, \
		0                                                        \
	);                                                               \
} while(::boost::test_tools::tt_detail::dummy_cond())                    \
/**/

#define BOOST_TEST_TOOL_LOC(level, P)         \
	BOOST_TEST_TOOL_ET_LOC_IMPL(P, level) \
/**/

#define BOOST_TEST_TOOL_LOC_EX(level, P, ...)                 \
	BOOST_TEST_TOOL_ET_LOC_IMPL_EX(P, level, __VA_ARGS__) \
/**/

#define BOOST_TEST_LOC( ... ) BOOST_TEST_INVOKE_IF_N_ARGS(     \
	2, BOOST_TEST_TOOL_LOC, BOOST_TEST_TOOL_LOC_EX, CHECK, \
	__VA_ARGS__                                            \
)                                                              \
/**/

#endif

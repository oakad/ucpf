/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_NP_PACKAGER_20140522T1620)
#define UCPF_MINA_NP_PACKAGER_20140522T1620

#include <initializer_list>

namespace ucpf { namespace mina {

template <typename StoreType, typename TempStoreType>
struct np_packager {
	typedef std::initializer_list<char const *> name_pack_type;

	template <typename ...Args>
	np_packager &operator()(name_pack_type &&names, Args &&...args)
	{
		inspect(
			names.begin(), std::forward<name_pack_type>(names),
			std::forward<Args>(args)...
		);
		return *this;
	}

private:
	void inspect(
		typename name_pack_type::iterator iter,
		name_pack_type &&names
	)
	{}

	template <typename T0, typename ...Tn>
	void inspect(
		typename name_pack_type::iterator iter,
		name_pack_type &&names, T0 &&a0, Tn &&...an
	)
	{
		if (iter != names.end()) {
			std::cout << "name " << *iter << " value " << a0 << '\n';
			++iter;
		} else
			std::cout << "name <none> " << " value " << a0 << '\n';

		inspect(
			iter, std::forward<name_pack_type>(names),
			std::forward<Tn>(an)...
		);
	}
};

}}

#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define MINA_DETAIL_STRINGIZE_OP(s, data, elem) BOOST_PP_STRINGIZE(elem)

#define MINA_DETAIL_STRINGIZE_PACK(...)               \
BOOST_PP_SEQ_ENUM(                                    \
	BOOST_PP_SEQ_TRANSFORM(                       \
		MINA_DETAIL_STRINGIZE_OP, 0,          \
		BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
	)                                             \
)

#define MINA_NPP(...) {MINA_DETAIL_STRINGIZE_PACK(__VA_ARGS__)}, __VA_ARGS__

#endif

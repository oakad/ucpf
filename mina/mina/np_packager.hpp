/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_NP_PACKAGER_20140522T1620)
#define UCPF_MINA_NP_PACKAGER_20140522T1620

#include <utility>
#include <initializer_list>
#include <mina/detail/is_composite.hpp>

namespace ucpf { namespace mina {

template <typename StoreType>
struct np_packager {
	typedef std::initializer_list<char const *> name_pack_type;

	template <typename ...Args>
	np_packager(Args &&...args)
	: store(std::forward<Args>(args)...)
	{}

	template <typename ...Args>
	bool save(name_pack_type &&names, Args &&...args)
	{
		if (store.start_save()) {
			unpack = false;
			inspect(
				names.begin(),
				std::forward<name_pack_type>(names),
				std::forward<Args>(args)...
			);
			store.end_save();
			return true;
		}
		return false;
	}

	template <typename ...Args>
	bool restore(name_pack_type &&names, Args &&...args)
	{
		if (store.start_scan()) {
			unpack = false;
			inspect(
				names.begin(),
				std::forward<name_pack_type>(names),
				std::forward<Args>(args)...
			);
			store.merge_scan();
		}

		if (store.start_restore()) {
			unpack = true;
			inspect(
				names.begin(),
				std::forward<name_pack_type>(names),
				std::forward<Args>(args)...
			);
			store.end_restore();
			return true;
		}
		return false;
	}

	/* Will be nice to edit calls to this method via compiler plugin
	 * and get rid of MINA_NPP macro below. */
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
	template <typename T, bool Composite = true>
	struct cls
	{
		static void apply(
			np_packager &self, char const *name, T &&value
		)
		{
			self.inspect_c(name, std::forward<T>(value));
		}
	};

	template <typename T>
	struct cls<T, false>
	{
		static void apply(
			np_packager &self, char const *name, T &&value
		)
		{
			self.inspect_s(name, std::forward<T>(value));
		}
	};

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
		char const *name(nullptr);

		if (iter != names.end()) {
			name = *iter;
			++iter;
		}

		cls<
			T0, detail::is_composite<
				typename std::remove_reference<T0>::type,
				np_packager
			>::value
		>::apply(*this, name, std::forward<T0>(a0));

		inspect(
			iter, std::forward<name_pack_type>(names),
			std::forward<Tn>(an)...
		);
	}

	template <typename T>
	void inspect_s(char const *name, T &&value)
	{
		store.sync_value(name, std::forward<T>(value));
	}

	template <typename T>
	void inspect_c(char const *name, T &&value)
	{
		store.push_level(name);
		value.mina_pack(*this, unpack);
		store.pop_level();
	}

	bool unpack;
	StoreType store;
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

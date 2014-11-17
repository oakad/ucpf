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
#include <yesod/is_sequence.hpp>
#include <mina/detail/is_composite.hpp>

namespace ucpf { namespace mina {
namespace detail {

template <typename Td, typename T, typename Packager>
struct pack_adaptor {
	template <typename U, bool Composite = false>
	struct dispatch {
		static void save_value(
			Packager &p, char const *name, U &&value
		)
		{
			p.save_value(name, std::forward<U>(value));
		}

		static void restore_value(
			Packager &p, char const *name, U &&value
		)
		{
			p.restore_value(name, std::forward<U>(value));
		}
	};

	template <typename U>
	struct dispatch<U, true> {
		static void save_value(
			Packager &p, char const *name, U &&value
		)
		{
			p.descend(name);
			value.mina_pack(p, false);
			p.ascend(false);
		}

		static void restore_value(
			Packager &p, char const *name, U &&value
		)
		{
			p.descend(name);
			value.mina_pack(p, true);
			p.ascend(false);
		}
	};

	static void save_value(
		Packager &p, char const *name, T &&value
	)
	{
		dispatch<
			T, detail::is_composite<Td, Packager>::value
		>::save_value(p, name, std::forward<T>(value));
	}

	static void restore_value(
		Packager &p, char const *name, T &&value
	)
	{
		dispatch<
			T, detail::is_composite<Td, Packager>::value
		>::restore_value(p, name, std::forward<T>(value));
	}
};

template <typename Tv, std::size_t N, typename T, typename Packager>
struct pack_adaptor<Tv [N], T, Packager> {
	static void save_value(
		Packager &p, char const *name, T &&value
	)
	{
		p.descend(name);
		for (std::size_t c(0); c < N; ++c)
			p.inspect_s(nullptr, value[c]);

		p.ascend(false);
	}

	static void restore_value(
		Packager &p, char const *name, T &&value
	)
	{
		p.descend(name);
		for (std::size_t c(0); c < N; ++c)
			p.inspect_s(nullptr, value[c]);

		p.ascend(false);
	}
};

template <typename Tv, typename Alloc, typename T, typename Packager>
struct pack_adaptor<std::vector<Tv, Alloc>, T, Packager> {
	static void save_value(
		Packager &p, char const *name, T &&value
	)
	{
		p.descend(name);
		for (auto &v: value)
			p.inspect_s(nullptr, v);

		p.ascend(false);
	}

	static void restore_value(
		Packager &p, char const *name, T &&value
	)
	{
		p.descend(name);
		std::size_t c(0);
		while (p.probe_name(nullptr))
			++c;

		p.ascend(true);
		value.clear();
		value.resize(c);
		p.descend(name);
		for (auto &v: value)
			p.inspect_s(nullptr, v);

		p.ascend(false);
	}
};

}

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
	template <typename Td, typename T, typename Packager>
	friend struct detail::pack_adaptor;

	template <typename T0>
	void inspect_s(char const *name, T0 &&a0)
	{
		if (unpack)
			detail::pack_adaptor<
				typename std::remove_reference<T0>::type,
				T0, np_packager
			>::restore_value(*this, name, std::forward<T0>(a0));
		else
			detail::pack_adaptor<
				typename std::remove_reference<T0>::type,
				T0, np_packager
			>::save_value(*this, name, std::forward<T0>(a0));
	}

	void inspect(
		typename name_pack_type::const_iterator iter,
		name_pack_type &&names
	)
	{}

	template <typename T0, typename ...Tn>
	void inspect(
		typename name_pack_type::const_iterator iter,
		name_pack_type &&names, T0 &&a0, Tn &&...an
	)
	{
		char const *name(nullptr);

		if (iter != names.end()) {
			name = *iter;
			++iter;
		}

		inspect_s(name, std::forward<T0>(a0));

		inspect(
			iter, std::forward<name_pack_type>(names),
			std::forward<Tn>(an)...
		);
	}

	template <typename T>
	void save_value(char const *name, T &&value)
	{
		store.save_value(name, std::forward<T>(value));
	}

	template <typename T>
	void restore_value(char const *name, T &&value)
	{
		store.restore_value(name, std::forward<T>(value));
	}

	bool probe_name(char const *name)
	{
		return store.probe_name(name);
	}

	void descend(char const *name)
	{
		store.descend(name);
	}

	void ascend(bool release_auto_name)
	{
		store.ascend(release_auto_name);
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

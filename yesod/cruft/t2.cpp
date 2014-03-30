#include <array>
#include <tuple>
#include <vector>
#include <list>
#include <iostream>

#include <yesod/mpl/package.hpp>
#include <yesod/mpl/package_range_c.hpp>
#include <yesod/iterator/range.hpp>
#include <yesod/iterator/joined_range.hpp>


#include "../test/test_demangle.hpp"

namespace t = ucpf::yesod::test;

namespace ucpf { namespace yesod { namespace iterator {

struct slice_base {
	virtual ~slice_base() {}

	virtual void print() = 0;
};

template <typename T, size_t N>
struct slice {
	static constexpr size_t index = N;

	slice() = default;

	slice(T tt_)
	: tt(tt_)
	{}

	virtual void print()
	{
		std::cout << "xx " << t::demangle<T>() << " vv " << tt
		<< " ii " << index << '\n';
	}

	T tt;
};

template <typename... Tn>
struct ss {
	typedef typename mpl::package_range_c<size_t, 0, sizeof...(Tn)>::type indices;

	struct pack_type {
		template <typename T, T... Cn>
		struct apply {
			typedef std::tuple<slice<Tn, Cn>...> type;
		};
	};

	typename mpl::apply_pack_c<indices, pack_type::template apply>::type pack;
};

void t1()
{
	ss<int, short, float> aa;
	std::get<0>(aa.pack).print();
	std::get<1>(aa.pack).print();
	std::get<2>(aa.pack).print();

	std::array<int, 3> xx = {{1, 2, 3}};
	std::list<int> yy = {4, 5, 6};
	std::vector<int> zz = {7, 8, 9};

	auto xr(make_joined_range(xx, yy, zz));
}

}}}

int main()
{
	ucpf::yesod::iterator::t1();
	return 0;
}

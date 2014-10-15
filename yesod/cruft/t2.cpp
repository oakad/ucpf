#include <array>
#include <tuple>
#include <vector>
#include <list>
#include <iostream>

#include <yesod/mpl/package.hpp>
#include <yesod/mpl/package_range_c.hpp>
#include <yesod/mpl/repeat.hpp>
#include <yesod/mpl/size.hpp>
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
/*
	ss<int, short, float> aa;
	std::get<0>(aa.pack).print();
	std::get<1>(aa.pack).print();
	std::get<2>(aa.pack).print();

	std::array<int, 3> xx = {{1, 2, 3}};
	std::list<int> yy = {4, 5, 6};
	std::vector<int> zz = {7, 8, 9};

	auto xr(make_joined_range(xx, yy, zz));
*/
	auto s0(str("abcd"));
	std::string s1("efgh");

	auto j0(make_joined_range(s0, s1));

	for (auto cc: j0)
		std::cout << cc;

	std::cout << '\n';
	std::cout << *(j0.begin() + 5) << '\n';
	std::string ref0(j0.begin(), j0.end());
	std::cout << "s " << j0.size() << " " << ref0.size() << " " << j0.end() - j0.begin() << " " << ref0 << '\n';
}

void t2()
{
	typedef mpl::package<int, char, long> p0;
	typedef mpl::package<int &, char &, long &> p1;

	std::cout << "p0 " << t::demangle<p0>() << '\n';
	std::cout << "p1 " << t::demangle<p1>() << '\n';

	typedef typename mpl::join_pack<p0, p1>::type p2;
	std::cout << "p2 " << t::demangle<p2>() << '\n';

	std::cout << "xx " << t::demangle<typename mpl::repeat<int, 5>::type>() << '\n';


	typedef typename mpl::at_c_indices<
		mpl::at_c_value<p1>::template at_c,
		typename mpl::package_range_c<long, 2, 3>::type
	>::type p3;
	std::cout << "p3 " << t::demangle<p3>() << '\n';
	std::cout << mpl::size<p3>::type::value << '\n';
}

}}}

int main()
{
	//ucpf::yesod::iterator::t1();
	ucpf::yesod::iterator::t2();
	return 0;
}

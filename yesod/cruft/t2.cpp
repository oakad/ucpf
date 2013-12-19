#include <vector>
#include <iostream>

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/remove.hpp>
#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/value_cast.hpp>

#include "../test/test_demangle.hpp"

namespace t = ucpf::yesod::test;

//make_value
//same_as

namespace ucpf { namespace yesod { namespace mpl {

template <typename T0, typename T1, typename T2>
struct xx {
	typedef package<T0, T1, T2> x_pack;
	typedef list<T0, T1, T2> x_type;
};

struct xx_impl {
	template <typename T0, typename T1, typename T2>
	struct apply {
		typedef xx<T0, T1, T2> type;
	};
};

//template <typename T0, typename T1, typename T2>
//void t2(xx<T0, T1, T2> x_arg)
template <typename Pack>
void t2(typename apply<unpack_args<xx_impl>, Pack>::type x_arg)
{
	auto rv(value_cast<
		typename decltype(x_arg)::x_type, std::vector<int>
	>());
	printf("%d, %d, %d\n", rv[0], rv[1], rv[2]);

	//typedef package<T0, T1, T2> p0;
	//typedef typename apply<unpack_args<xx_impl>, p0>::type xy;
	//typedef apply_wrap<x_unpack<xx_impl>, p0> xy;
	//std::cout << "m1.1: " << t::demangle<xy>() << '\n';
}

void t1()
{
	typedef xx<int_<1>, int_<2>, int_<3>> xa;
	typedef xx<int_<9>, int_<8>, int_<7>> xb;
	t2<typename xa::x_pack>(xa());
	t2<typename xb::x_pack>(xb());
}

}}}


int main()
{
	ucpf::yesod::mpl::t1();
	return 0;
}

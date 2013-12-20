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

template <typename... Tn>
struct aa {
	typedef package<Tn...> x_pack;
	typedef list<Tn...> x_type;
};

template <typename... Tn>
void t3(aa<Tn...> x_arg)
{
	auto rv(value_cast<
		typename decltype(x_arg)::x_type, std::vector<int>
	>());

	typedef typename std::conditional<
		(sizeof...(Tn) > 3),
		typename apply_wrap<arg<3>, Tn...>::type,
		int_<-1>
	>::type z1;
	typedef typename std::conditional<
		(sizeof...(Tn) > 4),
		typename apply_wrap<arg<4>, Tn...>::type,
		int_<-2>
	>::type z2;

	rv.push_back(z1::value);
	rv.push_back(z2::value);
	for (int v : rv)
		printf("%d ", v);


	printf("\n");
}

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
	typedef aa<int_<1>, int_<2>, int_<3>> xa;
	typedef aa<int_<1>, int_<2>, int_<3>, int_<4>> xb;
	typedef aa<int_<1>, int_<2>, int_<3>, int_<4>, int_<5>> xc;
	typedef aa<int_<9>, int_<8>, int_<7>, int_<6>, int_<5>, int_<4>> xd;
	t3(xa());
	t3(xb());
	t3(xc());
	t3(xd());
}

}}}


int main()
{
	ucpf::yesod::mpl::t1();
	return 0;
}

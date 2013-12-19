#include <iostream>

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/remove.hpp>
#include <yesod/mpl/comparison.hpp>

#include <boost/mpl/x11/fold.hpp>
#include <boost/mpl/x11/list.hpp>
#include <boost/mpl/x11/size.hpp>
#include <boost/mpl/x11/equal.hpp>
#include <boost/mpl/x11/remove.hpp>
#include <boost/mpl/x11/comparison.hpp>

#include "../test/test_demangle.hpp"

namespace t = ucpf::yesod::test;

//make_value
//same_as

namespace ucpf { namespace yesod { namespace mpl {

void t1()
{
	typedef list_c<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9> numbers;
	typedef list_c<int, 4, 3, 2, 1, 0>::type answer;
	typedef remove_if<
		numbers, greater<arg<-1>, int_<4>>, front_inserter<list_c<int>>
	>::type result;
	typedef detail::equal_impl<
		result, answer, std::is_same<arg<-1>, arg<-1>>
	> e1;
	typedef equal<answer, result> e2;

	std::cout << "\n\n";
	std::cout << "m1.1 " << t::demangle<result>() << "\n\n";
	std::cout << "m1.2 " << t::demangle<answer>() << "\n\n";
	std::cout << "m1.3 " << size<result>::value << "\n\n";
	std::cout << "m1.4 " << t::demangle<typename e1::result_>() << "\n\n";
	std::cout << "m1.5 " << t::demangle<typename e1::type>() << "\n\n";
}
}}}

namespace boost { namespace mpl { namespace x11 {

void t2()
{
	typedef list_c<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9> numbers;
	typedef list_c<int, 4, 3, 2, 1, 0>::type answer;
	typedef remove_if<
		numbers, greater<arg<-1>, int_<4>>, front_inserter<list_c<int>>
	>::type result;

	std::cout << "m2.1 " << t::demangle<result>() << std::endl;
	std::cout << "m2.2 " << t::demangle<answer>() << std::endl;
	std::cout << "m2.3 " << size<result>::value << std::endl;
}

}}}

int main()
{
	boost::mpl::x11::t2();
	ucpf::yesod::mpl::t1();
	return 0;
}

#if 0
BOOST_AUTO_TEST_CASE(fold_1)
{
	typedef list_c<int, 5, -1, 0, -7, -2, 0, -5, 4> numbers;
	typedef list_c<int, -1, -7, -2, -5> negatives;
	typedef reverse_fold<
		numbers, list_c<int>, if_<
			less<arg<1>, int_<0>>, push_front<arg<0>, arg<1>>,
			arg<0>
	>>::type result;

	BOOST_CHECK((equal<                                 \
		result, negatives, equal_to<arg<0>, arg<1>> \
	>::value));
}
#endif

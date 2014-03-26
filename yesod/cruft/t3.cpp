#include <array>
#include <iostream>

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/find.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/remove.hpp>
#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/value_cast.hpp>

#include "../test/test_demangle.hpp"

namespace t = ucpf::yesod::test;

namespace ucpf { namespace yesod { namespace mpl {

template <typename T, T N>
struct fibonacci_seq {
	template <typename...>
	struct apply;

	template <typename U, U... Cn, U C0, U C1, U Cp>
	struct apply<
		package_c<U, Cn...>, package_c<U, C0, C1, Cp>
	> : apply<
		package_c<U, Cn..., C0 + C1>,
		typename std::conditional<
			Cp != 0, package_c<U, C1, C0 + C1, Cp - 1>, void
		>::type
	> {};

	template <typename U, U... Cn>
	struct apply<
		package_c<U, Cn...>, void
	> {
		typedef package_c<U, Cn...> type;
	};

	typedef typename apply<
		package_c<T>, package_c<T, 0, 1, N - 1>
	>::type type;
};

template <typename T, T N>
struct fibonacci_val {
	template <typename U, U... Cn>
	struct array_values {
		static constexpr std::array<
			U, sizeof...(Cn)
		> value = {{
			Cn...
		}};
	};

	struct apply_values {
		template <typename U, U... Cn>
		struct apply {
			typedef array_values<U, Cn...> type;
		};
	};

	typedef typename fibonacci_seq<T, N>::type package_type;
	typedef typename apply_pack_c<
		package_type, apply_values::template apply
	>::type array_type;

	static constexpr decltype(array_type::value) value = array_type::value;
};

template <typename T, T N>
constexpr decltype(fibonacci_val<T, N>::array_type::value)
fibonacci_val<T, N>::value;

//template <typename T, T... Cn>
//constexpr std::array<T, sizeof...(Cn)> array_values<T, Cn...>::value;

void t1()
{
	//static constexpr int depth = 40;
	//static_assert((sizeof(short) < 8) ? (depth < 45) : true, "aaargh!");
	//typedef typename fibonacci_seq<size_t, 12>::type f_pack;
	//std::cout << "xx " << t::demangle<f_pack>() << '\n';

	//typedef typename apply_pack_c<f_pack, apply_values::template apply>::type jj;

	typedef vector_c<int, 1, 2, 3> v0;
	typedef typename copy<v0, back_inserter<package<>>>::type l0;

	std::cout << "xx " << t::demangle<l0>() << '\n';
	
	//typedef fibonacci_val<size_t, 24> jj;
	typedef value_cast<int, l0> jj;
	for (auto d: jj::value)
		printf(" %zd,", d);
	printf("\n");

}

}}}


int main()
{
	ucpf::yesod::mpl::t1();
	return 0;
}

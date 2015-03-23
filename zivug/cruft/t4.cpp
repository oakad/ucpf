#include <array>
#include <utility>
#include <cstdio>

#include <yesod/mpl/package_range_c.hpp>
#include <yesod/mpl/value_cast.hpp>

namespace ym = ucpf::yesod::mpl;


struct pp_base {
	virtual void print() = 0;
};

template <int A>
struct pp {
	virtual void print()
	{
		printf("xx %d\n", A);
	}
};

struct xx {
	constexpr static int aa[] = {7, 8, 9, 10, 11};
};

constexpr static int bb[] = {46, 46, 456, 23};

template <typename T, T const &v>
struct arr_op {
	template <typename Ix, Ix... Cn>
	struct  apply {
		typedef ym::package_c<int, v[Cn]...> type;
	};
};

template <
	typename T0, T0 const &v0,
	template <typename T1, T1 const &v1> class Op
> struct transform_array;

template <
	typename U, std::size_t N, U const (&arr)[N],
	template <typename T, T const &v> class Op
> struct transform_array<U const [N], arr, Op> {
	typedef typename ym::package_range_c<
		std::size_t, 0, N
	>::type index_type;

	typedef ym::value_cast<
		typename ym::apply_pack_c<
			typename ym::package_range_c<std::size_t, 0, N>::type,
			Op<U const [N], arr>::template apply
		>::type
	> value_type;

};

struct arr_x {

	template <typename Ix, Ix... Cn>
	struct apply {
		typedef ym::package_c<int, xx::aa[Cn]...> type;
	};

	template <typename T>
	struct  idx_pack {
		typedef ym::package_c<std::size_t> type;
	};
	
	template <typename U, std::size_t N>
	struct  idx_pack<U const [N]> {
		typedef typename ym::package_range_c<
			std::size_t, 0, N
		>::type type;
	};

	typedef typename ym::apply_pack_c<
		typename idx_pack<decltype(xx::aa)>::type, //apply
		arr_op<decltype(xx::aa), xx::aa>::apply
	>::type out_pack;

	typedef ym::value_cast<out_pack> idx_val;	
};

int main(int argc, char **argv)
{
	//typedef typename arr_x::idx_val at;
	//typedef typename transform_array<decltype(xx::aa), xx::aa, arr_op>::value_type at;
	typedef typename transform_array<decltype(bb), bb, arr_op>::value_type at;

	for (auto d: at::value)
		printf("dd %zd\n", d);

	return 0;
}
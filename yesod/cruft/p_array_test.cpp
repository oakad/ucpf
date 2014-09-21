#include <yesod/detail/placement_array.hpp>

#include <yesod/mpl/package.hpp>
#include <yesod/mpl/package_range_c.hpp>

#include <test/test_demangle.hpp>

using ucpf::yesod::test::demangle;
using ucpf::yesod::detail::placement_array;

using ucpf::yesod::mpl::package;
using ucpf::yesod::mpl::package_c;
using ucpf::yesod::mpl::package_range_c;


struct policy {
	constexpr static std::array<
		std::size_t, 2
	> ptr_node_order = {{9, 3}};
	constexpr static std::array<
		std::size_t, 5
	> data_node_order = {{32, 16, 8, 4, 2}};
};

template <typename P>
struct expand_a {
	template <std::size_t C>
	using num = std::integral_constant<std::size_t, C>;

	typedef typename package_range_c<
		std::size_t, 0, P::value.size()
	>::type idx;

	template <typename...>
	struct apply;

	template <template <typename U, U... Un> class Pack, typename U, U... Un>
	struct apply<Pack<U, Un...>> {
		typedef package<num<P::value[Un]>...> type;
	};

	typedef typename apply<idx>::type type;
};

struct node_base {
};

template <
	typename T,  std::size_t OrdId, std::size_t MaxOrdId,
	std::array<std::size_t, MaxOrdId> const *arr
> struct node : node_base {
	typedef node<T, OrdId, MaxOrdId, arr> this_node_type;

	typedef typename std::conditional<
		OrdId == 0,
		node<T, OrdId, MaxOrdId, arr>,
		node<T, OrdId - 1, MaxOrdId, arr>
	>::type prev_node_type;

	typedef typename std::conditional<
		OrdId == (MaxOrdId - 1),
		node<T, OrdId, MaxOrdId, arr>,
		node<T, OrdId + 1, MaxOrdId, arr>
	>::type next_node_type;

	constexpr static std::size_t apparent_order = (*arr)[0];
	constexpr static std::size_t real_order = (*arr)[OrdId];

	static_assert(
		std::is_same<prev_node_type, this_node_type>::value
		|| (prev_node_type::real_order > real_order),
		"prev_node_type::real_order > real_order"
	);
	static_assert(
		std::is_same<next_node_type, this_node_type>::value
		|| (next_node_type::real_order < real_order),
		"next_node_type::real_order < real_order"
	);
};
/*
template <typename T, typename Policy, std::size_t MaxOrdId>
struct node<T, Policy, 0, MaxOrdId> : node_base {
	constexpr static std::size_t val = Policy::ptr_node_order[0];
};
*/


int main()
{
	typedef node<
		int, 0, policy::ptr_node_order.size(), &policy::ptr_node_order
	> x_node1;
	typedef node<
		int, 0, policy::data_node_order.size(), &policy::data_node_order
	> x_node2;
	printf("xx %zd\n", x_node1::real_order);
	printf("xy %zd\n", x_node2::real_order);


	typedef placement_array<int, 64> xa0;

	struct null_pred {
		static bool test(void const *v)
		{
			return v != 0;
		}
	};

	typedef placement_array<void *, 64, null_pred> xa1;

	struct vv {
		vv(int v_)
		: v(v_)
		{}

		virtual ~vv()
		{
			printf("vv %d\n", v);
		}

		int v;
	};

	typedef placement_array<vv, 64> xa2;

	typedef std::allocator<void> alc;

	printf("xa0 %d, %d, %zd\n", std::is_trivial<xa0>::value,
	std::is_standard_layout<xa0>::value, sizeof(xa0));

	printf("xa1 %d, %d, %zd\n", std::is_trivial<xa1>::value,
	std::is_standard_layout<xa1>::value, sizeof(xa1));

	printf("xa2 %d, %d, %zd\n", std::is_trivial<xa2>::value,
	std::is_standard_layout<xa2>::value, sizeof(xa2));

	xa2 xxa2;

	xxa2.init(alc());
	xxa2.emplace_at(alc(), 7, 7);
	xxa2.emplace_at(alc(), 13, 13);
	xxa2.emplace_at(alc(), 56, 56);

	xxa2.for_each(0, [](size_t pos, vv const &v) -> bool {
		printf("ff %d\n", v.v);
		return false;
	});

	xxa2.for_each(20, [](size_t pos, vv const &v) -> bool {
		printf("ff1 %d\n", v.v);
		return false;
	});

	xxa2.destroy(alc());
	return 0;
}

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
	constexpr static std::array<std::size_t, 1> value = {{
		8
	}};
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

int main()
{
	printf("pp %s\n", demangle<typename expand_a<policy>::type>().c_str());
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

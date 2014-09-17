#include <yesod/detail/placement_array.hpp>

using ucpf::yesod::detail::placement_array;

constexpr std::size_t max_height(
	std::size_t bits, std::size_t d_ord, std::size_t p_ord
)
{
	return ((bits - d_ord) / p_ord) + (
		(bits - d_ord) % p_ord ?  2 : 1
	);
}

template <std::size_t MH = max_height(32, 8, 4)>
constexpr std::size_t[MH] expand(uint32_t v, int h = 0)
{
	// 8, 4,  4,  4,  4,  4, 4
	// 0, 12, 16, 20, 24, 28,
}

int main()
{
	printf("mh %zd\n", max_height(32, 8, 4));
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

#include <yesod/detail/placement_array.hpp>

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/package.hpp>
#include <yesod/mpl/push_pop.hpp>
#include <yesod/mpl/front_back.hpp>
#include <yesod/mpl/value_cast.hpp>
#include <test/test_demangle.hpp>

using ucpf::yesod::detail::placement_array;

constexpr std::size_t max_height(
	std::size_t bits, std::size_t d_ord, std::size_t p_ord
)
{
	return ((bits - d_ord) / p_ord) + (
		(bits - d_ord) % p_ord ?  2 : 1
	);
}

using ucpf::yesod::mpl::front;
using ucpf::yesod::mpl::package;
using ucpf::yesod::mpl::package_c;
using ucpf::yesod::mpl::pop_front;
using ucpf::yesod::mpl::push_back;
using ucpf::yesod::mpl::value_cast;
using ucpf::yesod::test::demangle;

template <std::size_t BitCount, std::size_t... Fn>
struct static_bit_field_map {
	template <std::size_t C>
	using num = std::integral_constant<std::size_t, C>;

	template <std::size_t Bc, std::size_t Sc>
	struct entry {
		constexpr static std::pair<
			std::size_t, std::size_t
		> value = {Bc, Sc};
	};

	typedef typename std::conditional<
		sizeof...(Fn),
		package<num<Fn>...>,
		package<num<BitCount>>
	>::type f_pack;

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F,
		typename Pack, typename... Tn
	> struct apply_head;

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F,
		typename Pack, typename... Tn
	> struct apply_head<Bc, Sc, F, Pack, package<Tn...>> : apply_head<
		F <= Bc ? Bc - F : 0, F <= Bc ? Sc + F : BitCount,
		front<Pack>::type::value, typename pop_front<Pack>::type,
		typename push_back<
			package<Tn...>, entry<F <= Bc ? F : F - Bc, Sc>
		>::type
	> {};

	template <std::size_t F, typename Pack, typename... Tn>
	struct apply_head<0, BitCount, F, Pack, package<Tn...>> {
		//static_assert(false, "bit count exceeded");
	};

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F, typename... Tn
	> struct apply_head<
		Bc, Sc, F, package<>, package<Tn...>
	> {
		typedef typename push_back<
			package<Tn...>, entry<F <= Bc ? F : F - Bc, Sc>
		>::type type;
		constexpr static std::size_t bit_count = F <= Bc ? Bc - F : 0;
		constexpr static std::size_t shift = Sc + (F <= Bc ? F : Bc);
		constexpr static std::size_t next_field = F;
	};

	typedef apply_head<
		BitCount, 0, front<f_pack>::type::value,
		typename pop_front<f_pack>::type, package<>
	> head_type;

};

template <std::size_t BitCount, std::size_t DataBits, std::size_t PtrBits>
struct node_offset_tbl {
	template <std::size_t C>
	using num = std::integral_constant<std::size_t, C>;

	template <std::size_t Bc, std::size_t Sc>
	struct entry {
		constexpr static std::pair<
			std::size_t, std::size_t
		> value = {Bc, Sc};
	};

	template <std::size_t Bc, std::size_t Sc, typename... Tn>
	struct apply;

	template <std::size_t Bc, std::size_t Sc, typename... Tn>
	struct apply<Bc, Sc, package<Tn...>> : apply<
		(PtrBits <= Bc) ? Bc - PtrBits : 0,
		(PtrBits <= Bc) ? Sc + PtrBits : BitCount,
		typename push_back<package<Tn...>, entry<
			(PtrBits <= Bc) ? PtrBits : Bc, Sc
		>>::type
	> {};

	template <typename... Tn>
	struct apply<0, BitCount, package<Tn...>> {
		typedef package<Tn...> type;
	};

	typedef typename apply<
		BitCount - DataBits, DataBits, package<entry<DataBits, 0>>
	>::type type;

	typedef value_cast<
		std::pair<std::size_t, std::size_t>, type
	> value_type;
};

int main()
{
	typedef typename static_bit_field_map<32, 10, 12, 5>::head_type ht;
	printf("xx %s\n", demangle<typename ht::type>().c_str());

	/*
	typedef typename node_offset_tbl<32, 15, 6>::value_type n32_tbl;

	
	for (auto p: n32_tbl::value)
		printf("yy %zd, %zd\n", p.first, p.second);
	*/
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

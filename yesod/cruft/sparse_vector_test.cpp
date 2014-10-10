#include <yesod/sparse_vector.hpp>
#include <iostream>
#include <test/test_demangle.hpp>

using ucpf::yesod::bitset;
using ucpf::yesod::sparse_vector;
using ucpf::yesod::sparse_vector_default_policy;
using ucpf::yesod::detail::compressed_array;
using ucpf::yesod::test::demangle;

struct pair_type {
	uintptr_t base;
	uintptr_t check;

	static pair_type make(uintptr_t base_, uintptr_t check_)
	{
		return pair_type{base_, check_};
	}
};

namespace std {

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(
	std::basic_ostream<CharType, TraitsType> &os, pair_type x
)
{
	os << os.widen('b') << os.widen(':') << os.widen(' ') << x.base;
	os << os.widen(',') << os.widen(' ');
	os << os.widen('c') << os.widen(':') << os.widen(' ') << x.check;
        return os;
}

}

struct pair_valid_pred
{
	static bool test(pair_type const &p)
	{
		return p.check != 0;
	}
};
/*
struct trie_vector_policy {
	typedef std::allocator<void> allocator_type;

	constexpr static std::array<std::size_t, 2> ptr_node_order = {{2, 3}};

	constexpr static std::array<
		std::size_t, 3
	> data_node_order = {{4, 5, 6}};

	typedef pair_valid_pred value_valid_pred;
};
*/

struct x_policy : sparse_vector_default_policy {
	typedef pair_valid_pred value_valid_pred;
};

#define X_PLACE(t, v) t.emplace_at(v, pair_type::make(v, v))

int main(int argc, char **argv)
{
	sparse_vector<pair_type, x_policy> trie;

	//trie.dump(std::cout);

	auto const &x_trie(trie);
/*
	x_trie.for_each(
		77, [](
			typename decltype(trie)::size_type pos, pair_type const &r
		) -> bool {
			printf("ff %zd - b %zd c %zd\n", pos, r.base, r.check);
			return false;
		}
	);
*/
	std::size_t base = 10000;

	for (auto c(1); c < 10; ++c)
		X_PLACE(trie, c);

	for (auto c(base); c < (40 + base); ++c)
		X_PLACE(trie, c);

	trie.dump(std::cout);
	auto up(trie.utilization());
	printf("ut1 %zd/%zd, %f\n", up.first, up.second, double(up.second) / up.first);
	printf("-----\n");

	for (auto c(1); c < 10; ++c)
		trie.erase(c);

	trie.dump(std::cout);
	up = trie.utilization();
	printf("ut2 %zd/%zd, %f\n", up.first, up.second, double(up.second) / up.first);
	printf("-----\n");

	trie.shrink_to_fit();
	trie.dump(std::cout);
	up = trie.utilization();
	printf("ut3 %zd/%zd, %f\n", up.first, up.second, double(up.second) / up.first);

	return 0;
}

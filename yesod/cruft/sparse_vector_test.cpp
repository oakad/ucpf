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

	X_PLACE(trie, 344);
	X_PLACE(trie, 258);
	X_PLACE(trie, 223);
	X_PLACE(trie, 116);
	X_PLACE(trie, 498);
	X_PLACE(trie, 230);
	X_PLACE(trie, 153);
	X_PLACE(trie, 448);
	X_PLACE(trie, 52);
	X_PLACE(trie, 44);
	X_PLACE(trie, 320);
	X_PLACE(trie, 138);
	X_PLACE(trie, 213);
	X_PLACE(trie, 477);
	X_PLACE(trie, 76);
	X_PLACE(trie, 446);
	X_PLACE(trie, 129);
	X_PLACE(trie, 425);
	X_PLACE(trie, 15);
	X_PLACE(trie, 242);
	X_PLACE(trie, 253);
	X_PLACE(trie, 248);
	X_PLACE(trie, 470);
	X_PLACE(trie, 433);
	X_PLACE(trie, 358);
	X_PLACE(trie, 86);
	X_PLACE(trie, 301);
	X_PLACE(trie, 27);
	X_PLACE(trie, 387);

	printf("-4---------\n");
	trie.dump(std::cout);

	auto r(*trie.ptr_at(344));
	printf("b %zd c %zd\n", r.base, r.check);
	r = *trie.ptr_at(258);
	printf("b %zd c %zd\n", r.base, r.check);

	auto const &x_trie(trie);
	x_trie.for_each(
		77, [](
			typename decltype(trie)::size_type pos, pair_type const &r
		) -> bool {
			printf("ff %zd - b %zd c %zd\n", pos, r.base, r.check);
			return false;
		}
	);

	printf("vv %d, %zd\n", 470, trie.find_vacant(470));
	printf("vv %d, %zd\n", 475, trie.find_vacant(475));
	printf("vv %d, %zd\n", 477, trie.find_vacant(477));
	printf("vv %d, %zd\n", 500, trie.find_vacant(500));
	
	return 0;
}

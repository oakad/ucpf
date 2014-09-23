#include <yesod/sparse_vector.hpp>
#include <iostream>

using ucpf::yesod::bitset;
using ucpf::yesod::sparse_vector;
using ucpf::yesod::detail::compressed_array;

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

struct trie_vector_policy {
	typedef std::allocator<void> allocator_type;

	constexpr static std::array<std::size_t, 1> ptr_node_order = {{4}};

	constexpr static std::array<std::size_t, 1> data_node_order = {{8}};

	typedef pair_valid_pred value_valid_pred;
};

int main(int argc, char **argv)
{
	compressed_array<int, 11, 7, void> ca0;

	ca0.init(std::allocator<void>());
	ca0.index_set(112, 0x56);
	ca0.index_set(578, 0x75);
	ca0.index_set(1325, 0x6e);

	printf("aa %zx, %zx, %zx\n", ca0.index_get(112), ca0.index_get(578), ca0.index_get(1325));
	//sparse_vector<pair_type, trie_vector_policy> trie;


	/*
	trie.emplace_at(4096, pair_type::make(4096, 4096));
	printf("-----------\n");
	trie.emplace_at(96, pair_type::make(96, 96));
	//trie.emplace_at(1024, pair_type::make(1024, 1024));
	printf("-----------\n");
	trie.dump(std::cout);
	*/
	/*
	auto r(trie.at(4096));
	printf("b %zd c %zd\n", r.base, r.check);
	r = trie.at(1024);
	printf("b %zd c %zd\n", r.base, r.check);
	*/
	return 0;
}

#include <yesod/sparse_vector.hpp>
#include <iostream>

using ucpf::yesod::sparse_vector;

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

	constexpr static size_t ptr_node_order = 4;

	constexpr static size_t data_node_order = 8;

	typedef pair_valid_pred value_valid_pred;
};

int main(int argc, char **argv)
{
	sparse_vector<pair_type, trie_vector_policy> trie;

	trie.emplace_at(4096, pair_type::make(4096, 4096));
	printf("-----------\n");
	trie.emplace_at(96, pair_type::make(96, 96));
	//trie.emplace_at(1024, pair_type::make(1024, 1024));
	printf("-----------\n");
	trie.dump(std::cout);
	/*
	auto r(trie.at(4096));
	printf("b %zd c %zd\n", r.base, r.check);
	r = trie.at(1024);
	printf("b %zd c %zd\n", r.base, r.check);
	*/
	return 0;
}

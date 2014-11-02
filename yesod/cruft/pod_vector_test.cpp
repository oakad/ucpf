#include <yesod/pod_sparse_vector.hpp>
#include <iostream>
#include <test/test_demangle.hpp>
#include <random>
#include <yesod/allocator/debug.hpp>
#include <unordered_set>

using ucpf::yesod::pod_sparse_vector;
using ucpf::yesod::pod_sparse_vector_default_policy;
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

int main(int argc, char **argv)
{
	pod_sparse_vector<pair_type> trie;

	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	constexpr static std::size_t max_value = 1000000;
	constexpr static std::size_t count = 40;
	std::unordered_set<uintptr_t> s0;

	trie.for_each_pos(30, 50, [&s0](auto pos, auto &p) -> void {
		p.base = pos;
		p.check = pos;
		s0.emplace(pos);
	});

	trie.for_each_pos(
		661539, 661588, [&s0](auto pos, auto &p) -> void {
			p = pair_type::make(pos, 8);
			s0.emplace(pos);
		}
	);

	printf("==============\n");
	trie.dump(std::cout);
	printf("==============\n");
	trie.for_each(0, [&s0](auto pos, auto &p) -> bool {
		if (p.base) {
			if (p.base != pos)
				printf("break 1 %zd, %zd\n", pos, p.base);
			if (!s0.count(p.base))
				printf("break 2 %zd\n", pos, p.base);
		}
		return false;
	});

	auto ut(trie.utilization([](auto &p) -> bool {
		return p.base != 0;
	}));

	printf(
		"ut all %zd, used %zd, rat %f\n",
		ut.first, ut.second, double(ut.second)/ut.first
	);
	return 0;
}

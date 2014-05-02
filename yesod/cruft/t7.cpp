#include <array>
#include <tuple>
#include <bitset>
#include <iostream>
#include <yesod/dynamic_bitset.hpp>
#include <yesod/allocator/policy/fibonacci.hpp>

using ucpf::yesod::dynamic_bitset;

int main()
{
	dynamic_bitset<> b;

	b.set(10);
	b.set(20);
	b.set(30);
	b.dump(std::cout);
	b.set(105);
	b.dump(std::cout);
	printf("xx %d\n", !b.test(15) && !b.test(25) && !b.test(35) && !b.test(105));
	printf("yy %d\n", b.test(10) && b.test(20) && b.test(30));
	typedef ucpf::yesod::allocator::policy::fibonacci aap;
	printf("aap %zd, %ud\n", aap::alloc_size::value.size(), aap::alloc_size::value.back());
	printf("aap1 %zd, %zd, %zd\n", aap::best_size(20000), aap::best_size(4000000), aap::best_size(10000000000ull));
	return 0;
}

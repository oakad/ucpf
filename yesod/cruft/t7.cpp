#include <array>
#include <tuple>
#include <bitset>
#include <iostream>
#include <yesod/dynamic_bitset.hpp>

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
	return 0;
}

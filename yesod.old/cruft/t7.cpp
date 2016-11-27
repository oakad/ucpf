#include <yesod/bitset.hpp>
#include <cstdio>

using ucpf::yesod::bitset;

int main()
{
	bitset<128> b;
	b.reset();
	b.set(0);
	b.set(5);
	b.set(32);
	b.set(38);
	b.set(56);
	b.set(100);

	b.for_each_zero(55, [](typename decltype(b)::size_type pos) -> bool {
		printf("xx %zd\n", pos);
		return false;
	});

	printf("aa 33 %zd\n", b.find_first_one(33));
	printf("aa 38 %zd\n", b.find_first_one(38));
	printf("aa 100 %zd\n", b.find_first_one(100));
	printf("aa 90 %zd\n", b.find_first_one(90));
	printf("aa 105 %zd\n", b.find_first_one(105));
	printf("aa 100 %zd\n", b.find_first_zero(100));
	return 0;
}

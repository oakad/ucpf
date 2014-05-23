
#include <iterator>
#include <vector>
#include <yesod/is_sequence.hpp>
#include <mina/detail/is_composite.hpp>


struct aaa {
};

struct bbb {
	void begin(int xx)
	{
	}
};

struct ccc {
	template <typename P>
	void mina_pack(P &a, bool b)
	{
	}
};

struct ddd {
	void mina_pack(double &a, bool b)
	{
	}
};


int main(int argc, char **argv)
{
	using ucpf::yesod::is_sequence;
	using ucpf::mina::detail::is_composite;
	printf("aaa %d\n", is_sequence<aaa>::value);
	printf("bbb %d\n", is_sequence<bbb>::value);
	printf("vvv %d\n", is_sequence<std::vector<int>>::value);
	printf("ccc %d\n", is_composite<ccc, int>::value);
	printf("bbb1 %d\n", is_composite<bbb, int>::value);
	printf("ddd1 %d\n", is_composite<ddd, double>::value);
	printf("ddd2 %d\n", is_composite<ddd, int>::value);
	return 0;
}

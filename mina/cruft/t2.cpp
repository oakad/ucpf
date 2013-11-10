
#include <iterator>
#include <vector>
#include <yesod/is_sequence.hpp>


struct aaa {
};

struct bbb {
	void begin(int xx)
	{
	}
};

int main(int argc, char **argv)
{
	using ucpf::yesod::is_sequence;
	printf("aaa %d\n", is_sequence<aaa>::value);
	printf("bbb %d\n", is_sequence<bbb>::value);
	printf("vvv %d\n", is_sequence<std::vector<int>>::value);
	return 0;
}


#include <mina/detail/pack.hpp>
#include <iterator>
#include <vector>

int main(int argc, char **argv)
{
	using ucpf::mina::detail::pack;

	std::vector<unsigned char> out;
	uint64_t a(0x12ULL);
	pack(std::back_inserter(out), a);
	printf("xx ");
	for (auto v : out)
		printf("%02x_", v);
	printf("\n");
	return 0;
}

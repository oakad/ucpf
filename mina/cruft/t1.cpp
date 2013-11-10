
#include <mina/detail/pack.hpp>
#include <iterator>
#include <vector>

int main(int argc, char **argv)
{
	using ucpf::mina::detail::pack;

	std::vector<unsigned char> out;
	std::vector<unsigned int> b = {0x12, 0x1234, 0x12345678};
	pack(std::back_inserter(out), b);
	uint64_t a(0x12ULL);
	pack(std::back_inserter(out), a);
	printf("xx ");
	for (auto v : out)
		printf("%02x_", v);
	printf("\n");
	return 0;
}

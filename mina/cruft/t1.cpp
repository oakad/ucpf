
#include <mina/pack.hpp>
#include <iterator>
#include <vector>

int main(int argc, char **argv)
{
	using ucpf::mina::pack;

	std::vector<unsigned char> out;
	uint64_t a(0x12ULL);
//	pack(std::back_inserter(out), a);
	std::vector<unsigned int> b = {0x12, 0x1234, 0x12345678};
//	pack(std::back_inserter(out), b);
	int32_t c(-7);
//	pack(std::back_inserter(out), c);

	pack(std::back_inserter(out), a, b, c);
	printf("xx ");
	for (auto v : out)
		printf("%02x_", v);
	printf("\n");
	return 0;
}

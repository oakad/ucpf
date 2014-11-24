#include <vector>
#include <string>
#include <array>
#include <mina/to_ascii_decimal.hpp>

using ucpf::mina::to_ascii_decimal;

void t1()
{
	auto shs = 1.234324Q;
	std::allocator<void> a;

	std::string ref;
	auto sink(std::back_inserter(ref));
	std::array<int32_t, 9> n1{
		-2147483648, -191575439, -5744323, -5542, 0, 9445, 614332,
		111105854, 2147483647
	};

	auto iter1(n1.begin());

	while (true) {
		to_ascii_decimal(sink, *iter1);
		++iter1;
		if (iter1 == n1.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	}
	/*
	to_ascii_decimal(sink, *iter1);
	++iter1;
	*sink++ = ',';
	*sink++ = ' ';
	to_ascii_decimal(sink, *iter1);
	++iter1;
	*sink++ = ',';
	*sink++ = ' ';
	to_ascii_decimal(sink, *iter1);
	*/
	printf("out %s\n", ref.c_str());
}

int main(int argc, char **argv)
{
	t1();
	return 0;
}

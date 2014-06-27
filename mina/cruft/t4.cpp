#include <mina/detail/to_ascii_decimal_f.hpp>

using ucpf::mina::detail::to_ascii_decimal_f;

int main(int argc, char **argv)
{
	double x1(1.23);
	char buf[120] = {0};
	char *ptr(buf);

	to_ascii_decimal_f<double>(ptr, x1);
	printf("aa %s\n", buf);
	return 0;
}

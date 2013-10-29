#include "rasham.hpp"

using ucpf::rasham::rshm;

void aaa()
{
	printf("--- x2.1\n");
	rshm(1);
	printf("--- x2.2\n");
	rshm(1, 2, 3).fmt<0>("aaa").fmt<1>(1, 2, 3).dst();
	printf("--- x2.3\n");
	rshm("aa", 1, "bb").dst("xxx");
}

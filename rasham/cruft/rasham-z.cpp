#include "rasham.hpp"

using ucpf::rasham::rshm;

void aaa();

int main(int argc, char **argv)
{
	printf("--- x1.1\n");
	rshm(1);
	printf("--- x1.2\n");
	rshm(1, 2, 3).fmt<0>("aaa").fmt<1>(1, 2, 3).dst();
	printf("--- x1.3\n");
	rshm("aa", 1, "bb");
	fflush(stdout);
	auto l(dlopen("./librasham-y.so", RTLD_LAZY));
	printf("-- l %p\n", l);
	auto xa(dlsym(l, "_Z3aaav"));
	printf("-- xa %p\n", xa);
	((void (*)())xa)();

}

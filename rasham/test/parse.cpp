#include <rasham/rasham.hpp>
#include <rasham/sink.hpp>
#include <iostream>

int main(int argc, char **argv)
{
//	rasham_print("/a1", "/a1");
//	rasham_print("/a2", "/a2");
	rasham_print("/a1/b1/c1", "/a1/b1/c1");
	rasham_print("/a1/b2/c1", "/a1/b2/c1");
	rasham_print("/a2/b1/c1", "/a2/b1/c1");
	rasham_print("/a2/b1/c2", "/a2/b1/c2");
	rasham_print("/a1/b1/c2", "/a1/b1/c2");
	rasham::dump_hierarchy(std::cerr);
	return 0;
}

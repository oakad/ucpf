#include <yesod/allocator/debug.hpp>
#include <yesod/allocator/any.hpp>

namespace ya = ucpf::yesod::allocator;

int main(int argc, char **argv)
{
	ya::debug<void> dd;
	ya::any xx(dd);
	auto p1(xx.alloc(100));
	auto p2(xx.alloc(200));
	xx.free(p1, 100);
	xx.free(p2, 200);
	return 0;
}

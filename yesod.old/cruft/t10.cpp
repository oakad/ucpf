#include <yesod/allocator/klipah.hpp>
#include <yesod/allocator/any.hpp>

namespace ya = ucpf::yesod::allocator;

int main(int argc, char **argv)
{
	ya::klipah<void>::print();
	ya::klipah<int>::print();
	ya::klipah<void, ya::k1_policy>::print();
	ya::klipah<int, ya::k1_policy>::print();
}

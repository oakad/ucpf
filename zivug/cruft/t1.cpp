#include <zivug/socket.hpp>

namespace ucpf { namespace zivug {

void t1()
{
	socket::event_dispatcher<16> ev_d;
}

}}

int main(int argc, char **argv)
{
	ucpf::zivug::t1();

	return 0;
}

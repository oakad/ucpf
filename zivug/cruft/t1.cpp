#include <zivug/socket.hpp>

namespace ucpf { namespace zivug {

struct sig_n : socket::notification {
	virtual ~sig_n()
	{}

	virtual void read_ready(bool out_of_band, bool priority)
	{
		printf("read %d, %d\n", out_of_band, priority);
	}

	virtual void write_ready(bool out_of_band, bool priority)
	{
		printf("write %d, %d\n", out_of_band, priority);
	}

	virtual void error(bool priority)
	{
		printf("error %d\n", priority);
	}

	virtual void hang_up(bool read_only)
	{
		printf("hang_up %d\n", read_only);
	}
};

void t1()
{
	socket::event_dispatcher<16> ev_d;
	printf("xx %d\n", ev_d.process_next());
}

}}

int main(int argc, char **argv)
{
	ucpf::zivug::t1();

	return 0;
}

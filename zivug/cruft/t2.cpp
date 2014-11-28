#include <zivug/io_event_dispatcher.hpp>

extern "C" {

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

}

namespace ucpf { namespace zivug {

template <typename Dispatcher>
struct socket_n : io::notification {
	socket_n(Dispatcher &src_disp_)
	: d([]() -> int {
		return ::socket(
			AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP
		);
	}), src_disp(src_disp_)
	{
		if (d.native() < 0)
			throw std::system_error(errno, std::system_category());

		printf("--1-\n");
		int v(1);
		auto rv(::setsockopt(
			d.native(), SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int)
		));

		if (rv < 0)
			throw std::system_error(errno, std::system_category());

		printf("--2-\n");
		::sockaddr_in addr{
			.sin_family = AF_INET,
			.sin_port = htons(3456),
			.sin_addr = { INADDR_ANY },
			.sin_zero = {0}
		};

		rv = ::bind(
			d.native(),
			reinterpret_cast<struct sockaddr *>(&addr),
			sizeof(addr)
		);

		if (rv < 0)
			throw std::system_error(errno, std::system_category());

		printf("--3-\n");
		src_disp.reset(d, *this);

		rv = ::listen(d.native(), 16);
		if (rv < 0)
			throw std::system_error(errno, std::system_category());

		printf("--4-\n");
	}

	virtual ~socket_n()
	{
		src_disp.remove(d);
	}

	virtual void read_ready(bool out_of_band, bool priority)
	{
		printf("read %d, %d\n", out_of_band, priority);
/*
		while (true) {
			auto rv(read(d.native(), &s_val, sizeof(s_val)));
			if (rv < 0) {
				if (errno == EAGAIN)
					src_disp.reset(d, *this);

				break;
			}

			printf(
				"msg %d - %d\n",
				s_val.ssi_signo, s_val.ssi_pid
			);
		}
*/
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

	io::descriptor d;
	Dispatcher &src_disp;
};

void t1()
{
	using namespace std::literals::chrono_literals;
	struct {
		struct {
			int event_count = 16;
			int timeout_ms = 5000;
		} epoll;
	} config;

	io::event_dispatcher ev_d(config);

	socket_n<decltype(ev_d)> ss(ev_d);

	printf("xx 1 %d\n", ev_d.process_next());
}

}}

int main(int argc, char **argv)
{
	ucpf::zivug::t1();

	return 0;
}

#include <zivug/io_event_dispatcher.hpp>

extern "C" {

#include <signal.h>
#include <sys/signalfd.h>

}

namespace ucpf { namespace zivug {

template <typename Dispatcher>
struct sig_n : io::notification {
	static sigset_t sig_mask;

	sig_n(Dispatcher &src_disp_)
	: d([]() -> int {
		return signalfd(-1, &sig_mask, SFD_NONBLOCK);
	}), src_disp(src_disp_)
	{
		src_disp.reset(d, *this);
	}

	virtual ~sig_n()
	{
		src_disp.remove(d);
	}

	virtual void read_ready(bool out_of_band, bool priority)
	{
		printf("read %d, %d\n", out_of_band, priority);
		::signalfd_siginfo s_val;

		while (true) {
			auto rv(read(d.native(), &s_val, sizeof(s_val)));
			if (rv < 0) {
				if (errno == EAGAIN)
					src_disp.reset(d, *this);

				break;
			}

			printf(
				"signal %d - %d\n",
				s_val.ssi_signo, s_val.ssi_pid
			);
		}

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

template <typename Dispatcher>
sigset_t sig_n<Dispatcher>::sig_mask;

void t1()
{
	using namespace std::literals::chrono_literals;
	io::event_dispatcher<16> ev_d(5s);

	::sigfillset(&sig_n<decltype(ev_d)>::sig_mask);
	::sigprocmask(SIG_BLOCK, &sig_n<decltype(ev_d)>::sig_mask, nullptr);

	sig_n<decltype(ev_d)> ss(ev_d);

	printf("xx 1 %d\n", ev_d.process_next());
	printf("xx 1 %d\n", ev_d.process_next());
	printf("xx 1 %d\n", ev_d.process_next());
}

}}

int main(int argc, char **argv)
{
	ucpf::zivug::t1();

	return 0;
}

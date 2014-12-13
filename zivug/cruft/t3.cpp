#include <zivug/arch/io_socket_configurator.hpp>

#include <string>
#include <vector>

struct {
	std::string protocol = "inet6.dgram.udp";
	std::vector<std::string> settings = {
	};
	int listen_backlog = 5;
} c0;

int main(int argc, char **argv)
{
	using ucpf::zivug::io::socket_configurator;

	auto d(socket_configurator::make_server(c0));
	return 0;
}

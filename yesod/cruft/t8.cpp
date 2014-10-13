#include <yesod/detail/dense_encoding_map.hpp>
#include <cstdio>

using ucpf::yesod::detail::dense_encoding_map;

int main()
{
	dense_encoding_map<char> m0;
	m0.reset();

	m0.set('a', 0);
	m0.set('c', 1);
	m0.set('z', 2);
	m0.set('\0', 3);
	m0.set('d', 4);

	for (std::size_t c(0); c < 128; ++c) {
		printf("val %zd: %d (%c)\n", c, m0.value(c), m0.value(c));
	}
	printf("ix 'a' - %d\n", m0.index('a'));
	printf("ix 'c' - %d\n", m0.index('c'));
	printf("ix 'z' - %d\n", m0.index('z'));
	printf("ix '0' - %d\n", m0.index('\0'));
	printf("ix 'd' - %d\n", m0.index('d'));
	return 0;
}

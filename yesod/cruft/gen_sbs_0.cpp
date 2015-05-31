#include <map>
#include <set>

template <typename Seq, typename Printer>
void emit_sep(
	Seq const &s, std::size_t step, Printer &&p
)
{
	auto iter(s.begin());
	std::size_t c(0);
	auto lc(std::min(s.size(), c + step));

	if (lc) {
		auto xc(c);
		printf("\t\t");
		p(*iter++);
		for (++xc; xc < lc; ++xc) {
			printf(", ");
			p(*iter++);
		}
	}
	c += lc;

	for (; c < s.size(); c += step) {
		printf(",\n");
		lc = std::min(s.size(), c + step);

		if (!lc)
			break;

		auto xc(c);
		printf("\t\t");
		p(*iter++);
		for (++xc; xc < lc; ++xc) {
			printf(", ");
			p(*iter++);
		}
	}
}

struct ord {
	ord()
	{}

	ord(int s_, int t_)
	: s(s_), t(t_)
	{}

	auto value() const
	{
		return t << s;
	}

	friend bool operator<(ord const &l, ord const &r)
	{
		return l.value() < r.value();
	}

	int s;
	int t;
};

int main(int argc, char **argv)
{
	int max_order = 18;
	int max_class = 127;
	std::map<int, std::set<ord>> m_odd;
	std::map<int, ord> m_all;

	for (int s(3); s <= max_order; s++) {
		for (int t(1); t <= max_class; t += 2) {
			if ((t << s) > (1 << max_order))
				break;

			m_odd[t].emplace(s, t);
			m_all.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(t << s),
				std::forward_as_tuple(s, t)
			);
		}
	}

	auto iter(m_all.begin());
	auto l_val(iter->first);
	auto cnt(0);
	printf("- %d\n ---", l_val);

	for (iter++; iter != m_all.end(); iter++) {
		printf(" %d", iter->first - l_val);
		l_val = iter->first;
		++cnt;
		if (__builtin_popcount(iter->first) == 1) {
			printf("\n--- %d\n", cnt);
			cnt = 0;
			printf("- %d\n ---", l_val);
		}
	}

	/*
	printf("\tconstexpr static std::uint16_t order_selector[] = {\n");
	auto ind(0);
	printf("\t\t%d", ind);
	auto iter(m_all.begin());
	for (iter++; iter != m_all.end(); iter++) {
		ind++;
		if (__builtin_popcount(iter->first) == 1)
			printf(", %d", ind);
	}

	printf("\n\t};\n\n");
	printf("\tconstexpr static std::uint32_t size_selector[] = {\n");
	emit_sep(m_all, 8, [](auto &v) {
		printf("%d", v.first);
	});
	printf("\n\t};\n");
	printf("\tconstexpr static block_class_ref");
	*/
	return 0;
}

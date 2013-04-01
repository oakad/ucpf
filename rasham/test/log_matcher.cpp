#include <rasham/internal/log_item.hpp>
#include <iostream>
#include <typeinfo>

int main(int argc, char **argv)
{
	using boost::spirit::qi::parse;
	using rasham::log_item::scanner;
	using rasham::log_item::range;

	std::string t1(
		"jhfkjdhf%sdf lkjf%%sakjf lkjf%kf5kf<_kjfl"
		"%fgfg[gff]{hfhf}kfdjg%lfdkj{dlsjf}[df]"
		"jtrltj%dfjglfdk{}djfldsk"
		"ummak %nugh{aa{{cc}dd}ee{}}ghghg"
	);
	scanner<std::string::const_iterator> s('%');
	range<std::string::const_iterator> t_range;
	auto start_iter(t1.cbegin());

	while (start_iter != t1.cend()) {
		bool rv(parse(start_iter, t1.cend(), s, t_range));
		start_iter = t_range.extent.end();
		std::cout << "parse: " << rv << std::endl;
		if (!rv)
			break;

		std::cout << "pattern range: "
			  << std::string(
				t_range.item_match.begin(),
				t_range.item_match.end()
			) << '\n';
		std::cout << "raw: "
			  << std::string(
				t_range.extent.begin(),
				t_range.extent.end()
			) << '\n';
		std::cout << "pattern (" << int(t_range.type) << "): "
			  << t_range.stmt.name << " c_expr: "
			  << t_range.stmt.c_expr << " p_expr: "
			  << t_range.stmt.p_expr << '\n';
	}
	std::cout << "left: " << std::string(start_iter, t1.cend()) << std::endl;
	return 0;
}

/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of the GNU  Affero General Public License version 3 as
 * published by the Free Software Foundation.
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <mina/np_packager.hpp>
#include <mina/dumping_store.hpp>

struct config {
	template <typename Packager>
	void mina_pack(Packager &p, bool unpack)
	{
		p(MINA_NPP(aab, bab, nuurgh, malla, putak));
	}

	int aab = 15;
	double bab = 32.0;
	std::string nuurgh = "iffy";

	struct sub_config0 {
		template <typename Packager>
		void mina_pack(Packager &p, bool unpack)
		{
			p(MINA_NPP(baragh, quz));
		}

		std::string baragh = "sussuz";
		int quz = 32;
	} malla;

	struct sub_config1 {
		template <typename Packager>
		void mina_pack(Packager &p, bool unpack)
		{
			//p(MINA_NPP(turuk, sull));
			p(MINA_NPP(sull));
		}

		//std::vector<char> turuk = {'a', 'b', 'c'};
		double sull = 1.54;
	} putak;
};

int main(int argc, char **argv)
{
	int ads(3), bds(4), cdsa(5);
	ucpf::mina::np_packager<ucpf::mina::dumping_store> d;
	config dgf;

	d.save(MINA_NPP(ads, bds, cdsa));
	d.save(MINA_NPP(dgf));

	return 0;
}

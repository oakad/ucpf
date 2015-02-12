/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/unit_test.hpp>

#include <mina/np_packager.hpp>
#include <mina/text_store_adaptor.hpp>

#include <mina/store/gdbm.hpp>

namespace ucpf { namespace mina {
namespace test {

struct c_type_0 {
	template <typename Packager>
	void mina_pack(Packager &p, bool unpack)
	{
		p(MINA_NPP(
			fielde, tentheraling, confer, bleakfulner,
			reamatman, emenes, subdive
		));
	}

	uint16_t fielde;
	float tentheraling;
	double confer;
	std::vector<int32_t> bleakfulner;

	struct {
		template <typename Packager>
		void mina_pack(Packager &p, bool unpack)
		{
			p(MINA_NPP(
				brishessess, untereeffedows, beamonally,
				perculatz
			));
		}

		uint64_t brishessess;
		double untereeffedows;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(sootess, indernies, winians));
			}

			uint32_t sootess;
			std::string indernies;
			std::string winians;
		} beamonally;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(
					commartional, comends, abhorthcrayte
				));
			}

			int8_t commartional;
			int comends;
			double abhorthcrayte;
		} perculatz;
	} reamatman;

	struct {
		template <typename Packager>
		void mina_pack(Packager &p, bool unpack)
		{
			p(MINA_NPP(lowest, cordions));
		}

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(conuzzles, torsentallowed));
			}

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(inatoring, perces));
				}

				int inatoring;
				int perces;
			} conuzzles;

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(
						merating, ridiming, unevacce
					));
				}

				double merating;
				double ridiming;
				std::string unevacce;
			} torsentallowed;
		} lowest;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(
					fleeple, flinexcloute, bookerialine,
					sanduction
				));
			}

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(
						discatted, chuces, cofamillywed
					));
				}

				std::string discatted;
				short chuces;
				long cofamillywed;
			} fleeple;

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(conted, orbergenerces));
				}

				int conted;
				int orbergenerces;
			} flinexcloute;

			double bookerialine;
			std::string sanduction;
		} cordions;
	} emenes;

	struct {
		template <typename Packager>
		void mina_pack(Packager &p, bool unpack)
		{
			p(MINA_NPP(
				equits, compattly, indialaskable, unusky,
				spargumen
			));
		}

		int equits;
		int compattly;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(martured, neiguadarnis));
			}

			int martured;

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(
						troscoppletion, nerainglester
					));
				}

				std::list<double> troscoppletion;

				struct {
					template <typename Packager>
					void mina_pack(
						Packager &p, bool unpack
					)
					{
						p(MINA_NPP(croveletit));
					}

					float croveletit;
				} nerainglester[3];
			} neiguadarnis;
		} indialaskable;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(tander));
			}

			struct {
				template <typename Packager>
				void mina_pack(Packager &p, bool unpack)
				{
					p(MINA_NPP(bonento));
				}

				std::string bonento;
			} tander;
		} unusky;

		struct {
			template <typename Packager>
			void mina_pack(Packager &p, bool unpack)
			{
				p(MINA_NPP(
					fijiffus, drawakespoly, hetenessing
				));
			}

			std::string fijiffus;
			int drawakespoly;
			float hetenessing;
		} spargumen;
	} subdive;
};

}

BOOST_AUTO_TEST_CASE(np_packager_0)
{
	mina::store::gdbm gs("ref/np_packager/gdbm_pack.00.in");

	np_packager<
		text_store_adaptor<decltype(gs), std::allocator<void>>
	> pack(gs, std::allocator<void>());

	test::c_type_0 test_0;
	pack.restore(MINA_NPP(test_0));

	std::vector<int32_t> ref0{515, 645, 463, 696};

	BOOST_CHECK_EQUAL(test_0.fielde, 34490);
	BOOST_CHECK_EQUAL_COLLECTIONS(
		test_0.bleakfulner.begin(), test_0.bleakfulner.end(),
		ref0.begin(), ref0.end()
	);
	BOOST_CHECK_EQUAL(
		test_0.subdive.spargumen.fijiffus,
		std::string("spargumen mumak")
	);

	auto &v1(test_0.subdive.indialaskable.neiguadarnis.troscoppletion);
	std::list<double> ref1{2.3, -56.4, 123.789};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		v1.begin(), v1.end(), ref1.begin(), ref1.end()
	);

	BOOST_CHECK_EQUAL(
		test_0.subdive.indialaskable.neiguadarnis.nerainglester[0]
		.croveletit, 1.01f
	);
	BOOST_CHECK_EQUAL(
		test_0.subdive.indialaskable.neiguadarnis.nerainglester[1]
		.croveletit, 1.11f
	);
	BOOST_CHECK_EQUAL(
		test_0.subdive.indialaskable.neiguadarnis.nerainglester[2]
		.croveletit, 1.21f
	);
}

}}

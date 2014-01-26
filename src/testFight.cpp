#include "stdafx.h"
#pragma warning(push)
#pragma warning(disable: 4512 4100 4099 4244 4127 4267 4706 4616 4396)
#include "gen-cpp/thrift_types.h"
#pragma warning(pop)
#include "testFight.h"
#include "fighting.h"
#include "Model.h"
#include "Tools.h"

void simulOneFight()
{
	size_t const fleetCount = 2; //rand() % 40;
	//size_t const playerCount = (rand() % 4) + 1;
	std::vector<Fleet> fleetVect;
	for(size_t fleetIdx = 0; fleetIdx < fleetCount; ++fleetIdx)
	{
		fleetVect.push_back(Fleet(fleetIdx, Player::ID(fleetIdx) + 1, Coord(0, 0, 0), 0));
		size_t const typeCount = rand() % 3 + 1;
		for(size_t typeIdx = 0; typeIdx < typeCount; ++typeIdx)
		{
			size_t const type = rand() % Ship::Cargo;
			uint32_t const count = rand() % (10000 / Ship::List[type].life);
			fleetVect.back().shipList[type] = count;
		}
	}
	/*fleetVect.push_back(Fleet(1, 1, Coord(0, 0, 0), 0));
	Fleet::ShipTab shipTab1 = { 0, 0, 0, 0, 0, 0, 7792, 0, 0};
	fleetVect.back().shipList = shipTab1;
	fleetVect.push_back(Fleet(2, 2, Coord(0, 0, 0), 0));
	Fleet::ShipTab shipTab2 = { 1757, 0, 0, 0, 0, 0, 0, 0, 0};
	fleetVect.back().shipList = shipTab2;*/

	std::vector<Fleet*> fleetPtrVect;
	for(size_t i = 0; i < fleetCount; ++i)
		fleetPtrVect.push_back(&fleetVect[i]);
	Planet planet(Coord(), 0);
	Planet* planetPtr =
	  (rand() % 1) ?
	  &planet :
	  nullptr;
	PlayersFightingMap playersFightingMap;
	PlayerCodeMap playerCodeMap;
	FightReport fightReport;
	fight(fleetPtrVect, playersFightingMap, planetPtr, playerCodeMap, fightReport);
	if((fightReport.fleetList[0].fightInfo.before.shipList ==
	    fightReport.fleetList[0].fightInfo.after.shipList) ||
	   (fightReport.fleetList[1].fightInfo.before.shipList ==
	    fightReport.fleetList[1].fightInfo.after.shipList))
	{
		//Avent:
		for(int type = 0; type < Ship::Count; ++type)
		{
			std::string name = mapFind(ndw::_Ship_Enum_VALUES_TO_NAMES, type)->second;
			std::cout << name.substr(0, 4) << '\t';
		}
		std::cout << std::endl;
		for(Report<Fleet> fleetReport : fightReport.fleetList)
		{
			for(uint32_t count : fleetReport.fightInfo.before.shipList)
				std::cout << count << "\t";
			std::cout << std::endl;
		}
		//Aprés:
		for(Report<Fleet> fleetReport : fightReport.fleetList)
		{
			for(uint32_t count : fleetReport.fightInfo.after.shipList)
				std::cout << count << "\t";
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}

void simulFight()
{
	for(size_t test = 0; test < 100; ++test)
	{
		//std::cout << std::endl;
		simulOneFight();
	}
}
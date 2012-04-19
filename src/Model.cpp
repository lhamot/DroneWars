#include "Model.h"

#include <algorithm>
#include <iostream>
#pragma warning(push)
#pragma warning(disable:4244 4310)
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#pragma warning(pop)

Player::ID createPlayer(Universe& univ, std::string const& login)
{
	Player::ID newPlayerID = univ.nextPlayerID;
	univ.nextPlayerID += 1;

	univ.playerMap.insert(std::make_pair(newPlayerID, Player(newPlayerID, login)));

	bool done = false;
	{
		size_t const planetNumber = rand() % univ.planetMap.size();
		auto planetIter = univ.planetMap.begin();
		std::advance(planetIter, planetNumber);
		Planet& planet = planetIter->second;
		if(planet.playerId == Player::NoId)
		{
			planet.playerId = newPlayerID;

			planet.buildingMap[Building::CommandCenter] = 0;
			done = true;
			//planet.buildingSet.push_back(Building(Building::MetalMine));
			//planet.buildingSet.push_back(Building(Building::CarbonicCentral));
		}
	}while(done == false);

	return newPlayerID;
}

void construct(Universe& univ)
{
	//univ.zoneMap.resize(
	//	boost::extents[Universe::MapSizeX][Universe::MapSizeY][Universe::MapSizeZ]);
	univ.time = time(0) + 2000000;

	std::set<Coord, CompCoord> coordSet;
	for(int i = 0; i < 100000; ++i)
	{
		Coord coord(rand() % Universe::MapSizeX,
		            rand() % Universe::MapSizeY,
		            rand() % Universe::MapSizeZ);

		if(false == coordSet.count(coord))
		{
			univ.planetMap.insert(std::make_pair(coord, Planet(coord)));
			coordSet.insert(coord);
		}
	}

	createPlayer(univ, "admin");
};

std::string getBuildingName(Building::Type type)
{
#define BTA_STR(X) case Building::X: return #X;
	switch(type)
	{
		BTA_STR(CommandCenter)
		BTA_STR(MetalMine)
		BTA_STR(CarbonMine)
		BTA_STR(CristalMine)
		BTA_STR(Factory)
		BTA_STR(Laboratory)
		BTA_STR(CarbonicCentral)
		BTA_STR(SolarCentral)
		BTA_STR(GeothermicCentral)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown buiding type"));
}


void saveToStream(Universe const& univ, std::ostream &out)
{
	boost::archive::binary_oarchive oa(out);
	oa& univ;
}


void loadFromStream(std::istream &in, Universe& univ)
{
	std::cout << "Loading... ";
	boost::archive::binary_iarchive ia(in);
	ia& univ;
	std::cout << "OK" << std::endl;
}


std::string getTaskName(Task::Type type)
{
#define BTA_STR(X) case Task::X: return #X;
	switch(type)
	{
		BTA_STR(UpgradeBuilding)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
}
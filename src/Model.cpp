#include "Model.h"

#include <algorithm>
#include <iostream>
#include <functional>
#pragma warning(push)
#pragma warning(disable:4244 4310 4100)
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/geometry/geometries/adapted/boost_array.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp> 
#include <boost/iterator/zip_iterator.hpp> 
#include <boost/range/algorithm.hpp>
#pragma warning(pop)



BOOST_GEOMETRY_REGISTER_BOOST_ARRAY_CS(cs::cartesian)

using namespace std::placeholders; 
using namespace std;
using namespace boost;


Building const Building::List[] = {
	{RessourceSet(100, 0, 0), 1.6}, //CommandCenter
	{RessourceSet(100, 0, 0), 1.6}, //MetalMine
	{RessourceSet(100, 0, 0), 1.6}, //CarbonMine
	{RessourceSet(100, 0, 0), 1.6}, //CristalMine
	{RessourceSet(100, 0, 0), 1.6}, //Factory
	{RessourceSet(100, 0, 0), 1.6}, //Laboratory
	{RessourceSet(100, 0, 0), 1.6}, //CarbonicCentral
	{RessourceSet(100, 0, 0), 1.6}, //SolarCentral
	{RessourceSet(100, 0, 0), 1.6}  //GeothermicCentral
};
static_assert(sizeof(Building::List) == (sizeof(Building) * Building::Count), "Building info missing");


Ship const Ship::List[] = {
	{RessourceSet(   100, 0, 0)}, //Mosquito
	{RessourceSet(   400, 0, 0)}, //Hornet
	{RessourceSet(  2000, 0, 0)}, //Vulture
	{RessourceSet( 10000, 0, 0)}, //Dragon
	{RessourceSet( 40000, 0, 0)}, //Behemoth
	{RessourceSet(200000, 0, 0)}  //Apocalyps
};
static_assert(sizeof(Ship::List) == (sizeof(Ship) * Ship::Count), "Ship info missing");


RessourceSet getBuilingPrice(Building::Enum id, size_t level)
{
	Building const& buildind = Building::List[id];
	double const coef = std::pow(buildind.coef, level - 1.);
	RessourceSet result = buildind.price;
	boost::geometry::multiply_value(result.tab, size_t(coef * 1000.));
	boost::geometry::divide_value(result.tab, 1000);
	return result;
}

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
			planet.ressourceSet = RessourceSet(500, 200, 0);
			done = true;
			//planet.buildingSet.push_back(Building(Building::MetalMine));
			//planet.buildingSet.push_back(Building(Building::CarbonicCentral));
		}
	}
	while(done == false);

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

std::string getBuildingName(Building::Enum type)
{
#define BTA_STR(X) case Building::X: return #X;
	switch(type)
	{
		BTA_STR(CommandCenter)
		BTA_STR(MetalMine)
		BTA_STR(CarbonMine)
		BTA_STR(LoiciumFilter)
		BTA_STR(Factory)
		BTA_STR(Laboratory)
		BTA_STR(CarbonicCentral)
		BTA_STR(SolarCentral)
		BTA_STR(GeothermicCentral)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown buiding type"));
	static_assert(Building::GeothermicCentral == Building::Count - 1, "Building cases missing");
}


void saveToStream(Universe const& univ, std::ostream& out)
{
	boost::archive::binary_oarchive oa(out);
	oa& univ;
}


void loadFromStream(std::istream& in, Universe& univ)
{
	std::cout << "Loading... ";
	boost::archive::binary_iarchive ia(in);
	ia& univ;
	std::cout << "OK" << std::endl;
}


bool canPay(Planet const& planet, RessourceSet const& price)
{
	for(int i = 0; i < Ressource::Count; ++i)
	{
		if(planet.ressourceSet.tab[i] < price.tab[i])
			return false;
	}
	return true;
}

bool canPay(Fleet const& fleet, RessourceSet const& price)
{
	for(int i = 0; i < Ressource::Count; ++i)
	{
		if(fleet.ressourceSet.tab[i] < price.tab[i])
			return false;
	}
	return true;
}

void pay(Planet& planet, RessourceSet const& price)
{
	for(int i = 0; i < Ressource::Count; ++i)
	{
		assert(planet.ressourceSet.tab[i] >= price.tab[i]);
		planet.ressourceSet.tab[i] -= price.tab[i];
	}
}

void pay(Fleet& fleet, RessourceSet const& price)
{
	for(int i = 0; i < Ressource::Count; ++i)
	{
		assert(fleet.ressourceSet.tab[i] >= price.tab[i]);
		fleet.ressourceSet.tab[i] -= price.tab[i];
	}
}
bool canBuild(Planet const& planet, Ship::Enum type, size_t number)
{
	RessourceSet price = Ship::List[type].price;
	boost::geometry::multiply_value(price.tab, number);
	for(int i = 0; i < RessourceSet::Tab::static_size; ++i)
	{
		if(price.tab[i] > planet.ressourceSet.tab[i])
			return false;
	}
	return true;
}

bool canBuild(Planet const& planet, Building::Enum type)
{
	RessourceSet price = Building::List[type].price;
	for(int i = 0; i < RessourceSet::Tab::static_size; ++i)
	{
		if(price.tab[i] > planet.ressourceSet.tab[i])
			return false;
	}

	auto iter = find_if(planet.taskQueue.begin(), planet.taskQueue.end(),
	[&](Task const & task) {return task.value == static_cast<size_t>(type);});
	if(iter == planet.taskQueue.end())
		return true;
	else
		return false;
}

void addTask(Planet& planet, time_t time, Building::Enum building)
{
	auto const buIter = planet.buildingMap.find(building);
	size_t const buLevel = (buIter == planet.buildingMap.end()) ? 0 : buIter->second;
	size_t const duration = static_cast<size_t>(pow(buLevel + 1., 1.5) * 10);
	Task task(Task::UpgradeBuilding, time, duration);
	task.value = building;
	RessourceSet const price = getBuilingPrice(building, buLevel + 1);
	task.startCost = price;
	
	planet.taskQueue.push_back(task);
	pay(planet, price);
}

void addTask(Planet& planet, time_t time, Ship::Enum ship, size_t number)
{
	size_t const duration = static_cast<size_t>(pow(ship + 1., 2.) * 10);
	Task task(Task::MakeShip, time, duration);
	task.value = ship;
	task.value2 = number;
	RessourceSet const& price = Ship::List[ship].price;
	task.startCost = price;
	planet.taskQueue.push_back(task);
	pay(planet, price);
}

bool canStop(
  Planet const&,// planet,
  Building::Enum// type
)
{
	//TODO
	return true;
}

void stopTask(Planet& planet, Task::Enum tasktype, Building::Enum building)
{
	auto iter = find_if(planet.taskQueue, [&]
	(Task const & task)
	{
		return task.type == tasktype && task.value == static_cast<size_t>(building);
	});

	if(iter != planet.taskQueue.end())
		planet.taskQueue.erase(iter);
}


void execTask(Universe& univ, Planet& planet, Task& task)
{
	if((task.lauchTime + task.duration) <= univ.time)
	{
		switch(task.type)
		{
		case Task::UpgradeBuilding:
			planet.buildingMap[static_cast<Building::Enum>(task.value)] += 1;
			break;
		case Task::MakeShip:
		{
			Fleet newFleet(univ.nextFleetID++, planet.playerId, planet.coord);
			newFleet.shipList[task.value] += task.value2;
			univ.fleetList.push_back(newFleet);
		}
		break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown Task"));
		}
		static_assert(Task::MakeShip == (Task::Count - 1), "Missing cases in switch");
		task.expired = true;
	}
}

void execBuilding(Planet& planet, Building::Enum type, size_t level)
{
	switch(type)
	{
	case Building::CommandCenter: 
		planet.ressourceSet.tab[Ressource::Metal] += 1;
		break;
	case Building::MetalMine: 
		planet.ressourceSet.tab[Ressource::Metal] += level * size_t(std::pow(1.1, double(level)));
		break;
	case Building::CarbonMine: break;
	case Building::LoiciumFilter: break;
	case Building::Factory: break;
	case Building::Laboratory: break;
	case Building::CarbonicCentral: break;
	case Building::SolarCentral: break;
	case Building::GeothermicCentral: break;
	};
}

void planetRound(Universe& univ, Planet& planet)
{
	BOOST_FOREACH(Task& task, planet.taskQueue)
		execTask(univ, planet, task);

	planet.taskQueue.erase(
		remove_if(planet.taskQueue, bind(&Task::expired, _1)),
		planet.taskQueue.end());

	BOOST_FOREACH(auto& buildingNVP, planet.buildingMap)
		execBuilding(planet, buildingNVP.first, buildingNVP.second);
}
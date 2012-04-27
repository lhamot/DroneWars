#include "Model.h"

#include <algorithm>
#include <iostream>
#include <functional>
#pragma warning(push)
#pragma warning(disable:4244 4310 4100 4512)
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/geometry/geometries/adapted/boost_array.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

#include "Tools.h"


BOOST_GEOMETRY_REGISTER_BOOST_ARRAY_CS(cs::cartesian)

using namespace std::placeholders;
using namespace std;
using namespace boost;


Building const Building::List[] =
{
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


Ship const Ship::List[] =
{
	{RessourceSet(100, 0, 0)},    //Mosquito
	{RessourceSet(400, 0, 0)},    //Hornet
	{RessourceSet(2000, 0, 0)},   //Vulture
	{RessourceSet(10000, 0, 0)},  //Dragon
	{RessourceSet(40000, 0, 0)},  //Behemoth
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

	Player player(newPlayerID, login);
	player.fleetsCode = "";
	player.planetsCode =
		"function AI(planet, actions)\n"
		"  if (not planet.buildingMap:count(Building.MetalMine)) or (planet.buildingMap:find(Building.MetalMine) < 4) then\n"
		"    actions:append(PlanetAction(PlanetAction.Building, Building.MetalMine))\n"
		"  elseif not planet.buildingMap:count(Building.Factory) then\n"
		"    actions:append(PlanetAction(PlanetAction.Building, Building.Factory))\n"
		"  else\n"
		"    actions:append(PlanetAction(PlanetAction.Ship, Ship.Mosquito, 1))\n"
		"  end\n"
		"end";
	player.fleetsCode =
		"class 'AI'\n"
		"function AI:do_gather(myFleet, otherFleet)\n"
	  "  return true\n"
		"end\n\n"
		"function AI:do_fight(myFleet, otherFleet)\n"
	  "  return true\n"
		"end\n\n"
		"function AI:action(myFleet, planet)\n"
	  "  if planet then\n"
		"    if planet:is_free() then\n"
		"      if not (planet.ressourceSet == RessourceSet()) then\n"
	  "        return FleetAction(FleetAction.Harvest)\n"
		"      end\n"
		"    elseif planet.playerId == myFleet.playerId and myFleet.shipList:at(Ship.Mosquito) < 10 then\n"
	  "      return FleetAction(FleetAction.Nothing)\n"
		"    end\n"
		"  end\n"
	  "  return FleetAction(FleetAction.Move)\n"
	  "end\n\n";
	/*player.planetsCode =
	  "from DroneWars import *\n\n"
	  "def AI(planet, actions):\n"
	  "  if Building.MetalMine not in planet.buildingMap or planet.buildingMap[Building.MetalMine] < 4:\n"
	  "    actions.append(PlanetAction(PlanetAction.Type.Building, Building.MetalMine))\n"
	  "  elif Building.Factory not in planet.buildingMap:\n"
	  "    actions.append(PlanetAction(PlanetAction.Type.Building, Building.Factory))\n"
	  "  else:\n"
	  "    actions.append(PlanetAction(PlanetAction.Type.Ship, Ship.Mosquito, 1))\n";*/
	univ.playerMap.insert(std::make_pair(newPlayerID, player));

	bool done = false;

	do
	{
		size_t const planetNumber = rand() % univ.planetMap.size();
		auto planetIter = univ.planetMap.begin();
		std::advance(planetIter, planetNumber);
		Planet& planet = planetIter->second;
		if(planet.playerId == Player::NoId)
		{
			planet.playerId = newPlayerID;

			planet.buildingMap[Building::CommandCenter] = 0;
			planet.ressourceSet = RessourceSet(2000, 200, 0);
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
		RessourceSet ress(rand() % 1000,
		                  rand() % 500,
		                  rand() % 200);

		if(false == coordSet.count(coord))
		{
			Planet planet(coord);
			planet.ressourceSet = ress;
			univ.planetMap.insert(std::make_pair(coord, planet));
			coordSet.insert(coord);
		}
	}

	for(int i = 0; i < 1000; ++i)
		createPlayer(univ, "admin" + boost::lexical_cast<std::string>(i));
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
	if(false == canPay(planet, price))
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));

	for(int i = 0; i < Ressource::Count; ++i)
	{
		assert(planet.ressourceSet.tab[i] >= price.tab[i]);
		planet.ressourceSet.tab[i] -= price.tab[i];
	}
	if(planet.ressourceSet.tab[0] > 2000000000)
		BOOST_THROW_EXCEPTION(std::logic_error("Strange ressources value"));
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
	auto const buIter = planet.buildingMap.find(type);
	size_t const buLevel = (buIter == planet.buildingMap.end()) ? 0 : buIter->second;
	RessourceSet const price = getBuilingPrice(type, buLevel + 1);
	if(false == canPay(planet, price))
		return false;

	auto iter = find_if(planet.taskQueue.begin(), planet.taskQueue.end(),
	[&](PlanetTask const & task) {return task.value == static_cast<size_t>(type);});
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
	PlanetTask task(PlanetTask::UpgradeBuilding, time, duration);
	task.value = building;
	RessourceSet const price = getBuilingPrice(building, buLevel + 1);
	task.startCost = price;

	if(false == canPay(planet, price))
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));

	planet.taskQueue.push_back(task);
	pay(planet, price);
}

void addTask(Planet& planet, time_t time, Ship::Enum ship, size_t number)
{
	size_t const duration = static_cast<size_t>(pow(ship + 1., 2.) * 10);
	PlanetTask task(PlanetTask::MakeShip, time, duration);
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

void stopTask(Planet& planet, PlanetTask::Enum tasktype, Building::Enum building)
{
	auto iter = find_if(planet.taskQueue, [&]
	                    (PlanetTask const & task)
	{
		return task.type == tasktype && task.value == static_cast<size_t>(building);
	});

	if(iter != planet.taskQueue.end())
		planet.taskQueue.erase(iter);
}


void execTask(Universe& univ, Planet& planet, PlanetTask& task, time_t time)
{
	if((task.lauchTime + task.duration) <= univ.time)
	{
		switch(task.type)
		{
		case PlanetTask::UpgradeBuilding:
			planet.buildingMap[static_cast<Building::Enum>(task.value)] += 1;
			planet.eventList.push_back(Event(time, Event::Upgraded, "Building upgraded"));
			break;
		case PlanetTask::MakeShip:
		{
			Fleet newFleet(univ.nextFleetID++, planet.playerId, planet.coord);
			newFleet.shipList[task.value] += task.value2;
			univ.fleetMap.insert(make_pair(newFleet.id, newFleet));
		}
		break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetTask"));
		}
		static_assert(PlanetTask::MakeShip == (PlanetTask::Count - 1), "Missing cases in switch");
		task.expired = true;
	}
}

void execTask(Universe& univ, Fleet& fleet, FleetTask& task, time_t time)
{
	if((task.lauchTime + task.duration) <= univ.time)
	{
		switch(task.type)
		{
		case FleetTask::Move:
			fleet.coord = task.position;
			break;
		case FleetTask::Harvest:
		{
			Planet& planet = mapFind(univ.planetMap, task.position)->second;
			if(planet.playerId == Player::NoId)
			{
				boost::geometry::add_point(fleet.ressourceSet.tab, planet.ressourceSet.tab);
				boost::geometry::assign_value(planet.ressourceSet.tab, 0);
				fleet.eventList.push_back(Event(time, Event::PlanetHarvested, "Planet harvested"));
			}
		}
		break;
		case FleetTask::Colonize:
			break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown FleetTask"));
		}
		static_assert(FleetTask::Colonize == (FleetTask::Count - 1), "Missing cases in switch");
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
	case Building::CarbonMine:
		break;
	case Building::LoiciumFilter:
		break;
	case Building::Factory:
		break;
	case Building::Laboratory:
		break;
	case Building::CarbonicCentral:
		break;
	case Building::SolarCentral:
		break;
	case Building::GeothermicCentral:
		break;
	};
}

void planetRound(Universe& univ, Planet& planet, time_t time)
{
	BOOST_FOREACH(PlanetTask & task, planet.taskQueue)
		execTask(univ, planet, task, time);

	remove_erase_if(planet.taskQueue, bind(&PlanetTask::expired, _1));

	BOOST_FOREACH(auto & buildingNVP, planet.buildingMap)
		execBuilding(planet, buildingNVP.first, buildingNVP.second);
}


void fleetRound(Universe& univ, Fleet& fleet, time_t time)
{
	BOOST_FOREACH(FleetTask & task, fleet.taskQueue)
		execTask(univ, fleet, task, time);

	remove_erase_if(fleet.taskQueue, bind(&FleetTask::expired, _1));
}


void gather(Fleet& fleet, Fleet const& otherFleet)
{
	boost::geometry::add_point(fleet.ressourceSet.tab, otherFleet.ressourceSet.tab);
	boost::transform(fleet.shipList, otherFleet.shipList, fleet.shipList.begin(), std::plus<size_t>());
}


boost::logic::tribool fight(Fleet& fleet1, Fleet& fleet2)
{
	size_t value1 = 0;
	size_t value2 = 0;
	for(int i = 0; i < Ship::Count; ++i)
	{
		value1 += fleet1.shipList[i] * Ship::List[i].price.tab[0];
		value2 += fleet2.shipList[i] * Ship::List[i].price.tab[0];
	}
	if(value1 > value2)
		return true;
	else if(value1 < value2)
		return false;
	else
		return boost::logic::indeterminate;
}


bool canMove(Fleet const& fleet,
             Coord const& coord //Destination en valeur absolue
            )
{
	if(false == fleet.taskQueue.empty())
		return false;
	if(abs(fleet.coord.X - coord.X) > 1 ||
	   abs(fleet.coord.Y - coord.Y) > 1 ||
	   abs(fleet.coord.Z - coord.Z) > 1)
		return false;
	if(coord.X < 0 || coord.X >= Universe::MapSizeX ||
	   coord.Y < 0 || coord.Y >= Universe::MapSizeY ||
	   coord.Z < 0 || coord.Z >= Universe::MapSizeZ)
		return false;
	//TODO: Gestion du caburant
	return true;
}

void addTask(Fleet& fleet, time_t time, Coord const& coord)
{
	FleetTask task(FleetTask::Move, time, 10);
	task.position = coord;
	fleet.taskQueue.push_back(task);
}

bool canHarvest(Fleet const& fleet, Planet const& planet)
{
	if(false == fleet.taskQueue.empty())
		return false;
	return planet.playerId == Player::NoId;
}

void addTaskHarvest(Fleet& fleet, time_t time, Planet const& planet)
{
	FleetTask task(FleetTask::Harvest, time, 10);
	task.position = planet.coord;
	fleet.taskQueue.push_back(task);
}

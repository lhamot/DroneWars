#include "stdafx.h"
#include "Model.h"

#include "Tools.h"
#include "NameGen.h"
#include <boost/range/numeric.hpp>
#include <boost/format.hpp>


BOOST_GEOMETRY_REGISTER_BOOST_ARRAY_CS(cs::cartesian)

using namespace std::placeholders;
using namespace std;
//using namespace boost;


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
	{RessourceSet(100, 0, 0),    1,       1}, //Mosquito
	{RessourceSet(400, 0, 0),    2,       2}, //Hornet
	{RessourceSet(2000, 0, 0),   4,       4}, //Vulture
	{RessourceSet(10000, 0, 0),  8,       8}, //Dragon
	{RessourceSet(40000, 0, 0),  16,     16}, //Behemoth
	{RessourceSet(200000, 0, 0), 32,     32}, //Azathoth
	{RessourceSet(2000, 0, 0),   4,       2},	//Queen
	{RessourceSet(400, 0, 0),    2,       0}, //Cargo
	{RessourceSet(2000, 0, 0),   8,       0}  //LargeCargo
};
static_assert(sizeof(Ship::List) == (sizeof(Ship) * Ship::Count), "Ship info missing");

Cannon const Cannon::List[] =
{
	{RessourceSet(100, 0, 0),    1,       1},
	{RessourceSet(400, 0, 0),    2,       2},
	{RessourceSet(2000, 0, 0),   4,       4},
	{RessourceSet(10000, 0, 0),  8,       8},
	{RessourceSet(40000, 0, 0),  16,     16},
	{RessourceSet(200000, 0, 0), 32,     32},
};
static_assert(sizeof(Cannon::List) == (sizeof(Cannon) * Cannon::Count), "Cannon info missing");


RessourceSet getBuilingPrice(Building::Enum id, size_t level)
{
	Building const& buildind = Building::List[id];
	double const coef = std::pow(buildind.coef, level - 1.);
	RessourceSet result = buildind.price;
	boost::geometry::multiply_value(result.tab, size_t(coef * 1000.));
	boost::geometry::divide_value(result.tab, 1000);
	return result;
}

Player::ID createPlayer(Universe& univ, std::string const& login, std::string const& password)
{
	Player::ID newPlayerID = univ.nextPlayerID;
	univ.nextPlayerID += 1;

	Player player(newPlayerID, login, password);
	{
		std::stringstream blocklyFleetDefaultCode;
		{
			std::ifstream fleetFile("blocklyFleetDefaultCode.xml", std::ios::binary | std::ios::in);
			if(fleetFile.is_open() == false)
				BOOST_THROW_EXCEPTION(std::ios::failure("Can't open blocklyFleetDefaultCode.xml"));
			boost::iostreams::copy(fleetFile, blocklyFleetDefaultCode);
		}
		player.fleetsCode.setBlocklyCode(
		  (boost::format(blocklyFleetDefaultCode.str()) %
		   gettext("myFleet") %
		   gettext("otherFleet") %
		   gettext("planet") %
		   gettext("order") %
		   gettext("DO_GATHER_CODE_COMMENT") %
		   gettext("DO_FIGHT_CODE_COMMENT") %
		   gettext("FLEET_ACTION_CODE_COMMENT")).str());
		player.fleetsCode.setCode(
		  "function AI:do_gather(myFleet, otherFleet)\n"
		  "  return true\n"
		  "end\n"
		  "function AI:do_fight(myFleet, otherFleet)\n"
		  "  return true\n"
		  "end\n"
		  "function AI:action(myFleet, planet)\n"
		  "  order = FleetAction(FleetAction.Nothing,Coord())\n"
		  "  return order\n"
		  "end"
		);
	}

	{
		std::stringstream blocklyPlanetDefaultCode;
		{
			std::ifstream planetFile("blocklyPlanetDefaultCode.xml", std::ios::binary | std::ios::in);
			if(planetFile.is_open() == false)
				BOOST_THROW_EXCEPTION(std::ios::failure("Can't open blocklyPlanetDefaultCode.xml"));
			boost::iostreams::copy(planetFile, blocklyPlanetDefaultCode);
		}
		player.planetsCode.setBlocklyCode(
		  (boost::format(blocklyPlanetDefaultCode.str()) %
		   gettext("planet") %
		   gettext("fleets") %
		   gettext("order") %
		   gettext("PLANET_ACTION_CODE_COMMENT")).str());
		player.planetsCode.setCode(
		  "function AI(planet, fleets)\n"
		  "  return noPlanetAction()\n"
		  "end");
	}
	Universe::PlayerMap::iterator playerIter;
	bool added = false;
	tie(playerIter, added) = univ.playerMap.insert(make_pair(newPlayerID, player));

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
			if(planet.playerId > 100000)
				BOOST_THROW_EXCEPTION(std::logic_error("planet.playerId > 100000"));

			playerIter->second.mainPlanet = planet.coord;

			planet.buildingList[Building::CommandCenter] = 1;
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
	univ.time = 0;

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
			planet.name = nameGen();
			univ.planetMap.insert(std::make_pair(coord, planet));
			coordSet.insert(coord);
		}
	}

	std::string const& password = "test";
	for(int i = 0; i < 100; ++i)
	{
		Player::ID pid = createPlayer(univ, (boost::format("admin%1%") % i).str(), password);
		Player& player = mapFind(univ.playerMap, pid)->second;
		player.planetsCode.setCode(
		  "function AI(planet, fleets)\n"
		  "order = makeShip(Ship.Mosquito)\n"
		  "if planet.buildingList[Building.MetalMine] < (4) then\n"
		  "  order = makeBuilding(Building.MetalMine)\n"
		  "elseif planet.buildingList[Building.Factory] == (0) then\n"
		  "  order = makeBuilding(Building.Factory)\n"
		  "elseif planet.cannonTab[Cannon.Cannon1] < (10) then\n"
		  "  order = makeCannon(Cannon.Cannon1)\n"
		  "else\n"
		  "  for _,fleet in ipairs(fleets) do\n"
		  "    if fleet.shipList[Ship.Queen] == (0) then\n"
		  "      order = makeShip(Ship.Queen)\n"
		  "    elseif fleet.shipList[Ship.Mosquito] < (10) then\n"
		  "      order = makeShip(Ship.Mosquito)\n"
		  "    end\n"
		  "    ::continue::\n"
		  "  end\n"
		  "end\n"
		  "return order\n"
		  "end");
		player.fleetsCode.setCode(
		  "function AI:do_fight(myFleet, otherFleet)\n"
		  "  return true\n"
		  "end\n"
		  "function AI:do_gather(myFleet, otherFleet)\n"
		  "  return true\n"
		  "end\n"
		  "function AI:action(myFleet, planet)\n"
		  "  if myFleet.ressourceSet:at(Ressource.Metal) > (2000) then\n"
		  "    order = FleetAction(FleetAction.Move,directionFromTo(myFleet.coord,myFleet.origin))\n"
		  "  else\n"
		  "    order = FleetAction(FleetAction.Move,directionRandom())\n"
		  "  end\n"
		  "  if planet then\n"
		  "    if planet:isFree() then\n"
		  "      if myFleet.shipList[Ship.Queen] > (0) then\n"
		  "        order = FleetAction(FleetAction.Colonize,Coord())\n"
		  "      elseif planet.ressourceSet ~= RessourceSet(0,0,0) then\n"
		  "        order = FleetAction(FleetAction.Harvest,Coord())\n"
		  "      end\n"
		  "    elseif planet.playerId == myFleet.playerId then\n"
		  "      if myFleet.ressourceSet ~= RessourceSet(0,0,0) then\n"
		  "        order = FleetAction(FleetAction.Drop,Coord())\n"
		  "      elseif myFleet.shipList[Ship.Mosquito] < (10) then\n"
		  "        order = FleetAction(FleetAction.Nothing,Coord())\n"
		  "      end\n"
		  "    end\n"
		  "  end\n"
		  "  return order\n"
		  "end\n");
	}
};


void saveToStream(Universe const& univ, std::ostream& out)
{
	using namespace boost::iostreams;
	filtering_streambuf<output> outFilter;
	outFilter.push(gzip_compressor());
	outFilter.push(out);
	boost::archive::binary_oarchive oa(outFilter);
	oa& univ;
}


void loadFromStream(std::istream& in, Universe& univ)
{
	using namespace boost::iostreams;
	std::cout << "Loading... ";
	filtering_streambuf<input> inFilter;
	inFilter.push(gzip_decompressor());
	inFilter.push(in);

	boost::archive::binary_iarchive ia(inFilter);
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
	if(type >= Ship::Count)
		return false;

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
	if(type >= Building::Count)
		return false;

	size_t const buLevel = planet.buildingList[type];
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
	size_t const buLevel =  planet.buildingList[building];
	size_t const mult = 1;
	size_t const duration = static_cast<size_t>((pow(buLevel + 1., 1.5) * mult) + 0.5);
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
	size_t const div = 100;
	size_t duration = Ship::List[ship].price.tab[0] / div;
	if(duration == 0)
		duration = 1;
	PlanetTask task(PlanetTask::MakeShip, time, duration);
	task.value = ship;
	task.value2 = number;
	RessourceSet const& price = Ship::List[ship].price;
	task.startCost = price;
	planet.taskQueue.push_back(task);
	pay(planet, price);
}


void addTask(Planet& planet, time_t time, Cannon::Enum cannon, size_t number)
{
	size_t const div = 100;
	size_t duration = Cannon::List[cannon].price.tab[0] / div;
	if(duration == 0)
		duration = 1;
	PlanetTask task(PlanetTask::MakeCannon, time, duration);
	if(cannon >= Cannon::Count)
		BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent cannon type"));
	task.value = cannon;
	task.value2 = number;
	RessourceSet const& price = Cannon::List[cannon].price;
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
	auto iter = boost::find_if(planet.taskQueue, [&]
	                           (PlanetTask const & task)
	{
		return task.type == tasktype && task.value == static_cast<size_t>(building);
	});

	if(iter != planet.taskQueue.end())
		planet.taskQueue.erase(iter);
}


void execTask(Universe& univ,
              Planet& planet,
              PlanetTask& task,
              time_t time,
              std::vector<Signal>& signals)
{
	if(time_t(task.lauchTime + task.duration) <= univ.time)
	{
		switch(task.type)
		{
		case PlanetTask::UpgradeBuilding:
			if(task.value >= planet.buildingList.size())
				BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent building type"));
			else
			{
				planet.buildingList[task.value] += 1;
				Event event(univ.nextEventID++, time, Event::Upgraded);
				planet.eventList.push_back(event);
				signals.push_back(Signal(planet.playerId, event));
			}
			break;
		case PlanetTask::MakeShip:
		{
			Fleet newFleet(univ.nextFleetID++, planet.playerId, planet.coord);
			newFleet.shipList[task.value] += task.value2;
			univ.fleetMap.insert(make_pair(newFleet.id, newFleet));
			Event event(univ.nextEventID++, time, Event::ShipMade);
			signals.push_back(Signal(planet.playerId, event));
		}
		break;
		case PlanetTask::MakeCannon:
			if(task.value < Cannon::Count)
			{
				//BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent cannon type"));
				planet.cannonTab[task.value] += 1;
				Event event(univ.nextEventID++, time, Event::CannonMade);
				planet.eventList.push_back(event);
				signals.push_back(Signal(planet.playerId, event));
			}
			break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetTask"));
		}
		static_assert(PlanetTask::MakeCannon == (PlanetTask::Count - 1), "Missing cases in switch");
		task.expired = true;
	}
}

void execTask(Universe& univ,
              Fleet& fleet,
              FleetTask& task,
              time_t time,
              std::vector<Signal>& signals)
{
	if(time_t(task.lauchTime + task.duration) <= univ.time)
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
				Event event(univ.nextEventID++, time, Event::PlanetHarvested);
				fleet.eventList.push_back(event);
				signals.push_back(Signal(planet.playerId, event));
			}
		}
		break;
		case FleetTask::Colonize:
		{
			Planet& planet = mapFind(univ.planetMap, task.position)->second;
			if(planet.playerId == Player::NoId && fleet.shipList[Ship::Queen])
			{
				Event event(univ.nextEventID++, time, Event::PlanetColonized);
				fleet.eventList.push_back(event);
				signals.push_back(Signal(planet.playerId, event));
				fleet.shipList[Ship::Queen] -= 1;

				planet.buildingList[Building::CommandCenter] = 1;
				boost::geometry::add_point(planet.ressourceSet.tab, RessourceSet(2000, 500, 0).tab);
				planet.playerId = fleet.playerId;
				if(planet.playerId > 100000)
					BOOST_THROW_EXCEPTION(std::logic_error("planet.playerId > 100000"));
			}
		}
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
	if(level == 0)
		return;
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

void planetRound(Universe& univ, Planet& planet, time_t time, std::vector<Signal>& signals)
{
	for(PlanetTask & task: planet.taskQueue)
		execTask(univ, planet, task, time, signals);

	boost::remove_erase_if(planet.taskQueue, bind(&PlanetTask::expired, placeholders::_1));

	for(size_t type = 0; type < planet.buildingList.size(); ++type)
		execBuilding(planet, Building::Enum(type), planet.buildingList[type]);
}


void fleetRound(Universe& univ, Fleet& fleet, time_t time, std::vector<Signal>& signals)
{
	for(FleetTask & task: fleet.taskQueue)
		execTask(univ, fleet, task, time, signals);

	boost::remove_erase_if(fleet.taskQueue, bind(&FleetTask::expired, placeholders::_1));
}


void gather(Fleet& fleet, Fleet const& otherFleet)
{
	boost::geometry::add_point(fleet.ressourceSet.tab, otherFleet.ressourceSet.tab);
	boost::transform(fleet.shipList, otherFleet.shipList, fleet.shipList.begin(), std::plus<size_t>());
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

bool canColonize(Fleet const& fleet, Planet const& planet)
{
	if(false == fleet.taskQueue.empty())
		return false;
	return planet.playerId == Player::NoId && fleet.shipList[Ship::Queen];
}

void addTaskColonize(Fleet& fleet, time_t time, Planet const& planet)
{
	FleetTask task(FleetTask::Colonize, time, 10);
	task.position = planet.coord;
	fleet.taskQueue.push_back(task);
}

bool canDrop(Fleet const& fleet, Planet const& planet)
{
	size_t const ressCount = boost::accumulate(fleet.ressourceSet.tab, 0);
	return planet.playerId == fleet.playerId && ressCount > 0;
}

void drop(Fleet& fleet, Planet& planet)
{
	boost::geometry::add_point(planet.ressourceSet.tab, fleet.ressourceSet.tab);
	boost::geometry::assign_value(fleet.ressourceSet.tab, 0);
}

bool canBuild(Planet const& planet, Cannon::Enum type, size_t number)
{
	if(type >= Cannon::Count)
		return false;
	RessourceSet price = Cannon::List[type].price;
	boost::geometry::multiply_value(price.tab, number);
	for(int i = 0; i < RessourceSet::Tab::static_size; ++i)
	{
		if(price.tab[i] > planet.ressourceSet.tab[i])
			return false;
	}
	return true;
}


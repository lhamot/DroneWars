//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include <boost/range/numeric.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#pragma warning(push)
#pragma warning(disable: 4310 4100 4244 4458 4456 4706 4702)
#include <boost/archive/binary_iarchive.hpp>
#include "portable_binary_oarchive.hpp"
#include "portable_binary_iarchive.hpp"
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/locale.hpp>
#include <boost/serialization/boost_array.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/combine.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#pragma warning(pop)
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "UnivManip.h"

#include "Tools.h"
#include "Rules.h"
#include "NameGen.h"

#include "DataBase.h"
#include "Skills.h"


BOOST_GEOMETRY_REGISTER_BOOST_ARRAY_CS(cs::cartesian)

using namespace std;
using namespace boost;
namespace BL = boost::locale;


//! Retourne le prix d'un Building donné, à un niveau donné
//! @pre id est dans [0: Building::Count[
//! @pre level > 0
boost::optional<RessourceSet> getBuilingPrice(Building::Enum id, size_t level)
{
	if(id < 0 || id >= Building::Count)
		BOOST_THROW_EXCEPTION(
		  std::out_of_range("Expect a building id in the range [0: Building::Count["));
	if(level < 1)
		BOOST_THROW_EXCEPTION(
		  std::out_of_range("Expect a level greater than 1"));

	Building const& building = Building::List[id];
	double const coef = std::pow(building.coef, level - 1.) * 1000.;
	RessourceSet result = building.price;
	static double const maxVal = std::numeric_limits<Ressource::Value>::max();
	for(auto& val : result.tab)
	{
		double val2 = (double(val) * coef);
		if(val2 > maxVal)
			return boost::none;
		val = NUMERIC_CAST(Ressource::Value, val2);
	}
	for(auto& val : result.tab)
		val /= 1000;
	return result;
}


//! Recupère les script pour un nouveau joueur
//! @param codes [out] planète_lua, planète_blockly, flotte_lua, flotte_blockly
void getNewPlayerCode(std::vector<std::string>& codes)
{
	std::vector<std::string> result;

	//Ajout du code du joueur (Planet)
	{
		std::stringstream blocklyPlanetDefaultCode;
		{
			std::ifstream planetFile("blocklyPlanetDefaultCode.xml",
			                         std::ios::binary | std::ios::in);
			if(planetFile.is_open() == false)
				BOOST_THROW_EXCEPTION(
				  std::ios::failure("Cant open blocklyPlanetDefaultCode.xml"));
			boost::iostreams::copy(planetFile, blocklyPlanetDefaultCode);
		}
		std::string const blocklyCode =
		  (boost::format(blocklyPlanetDefaultCode.str()) %
		   BL::gettext("my_planet") %
		   BL::gettext("fleets") %
		   BL::gettext("order") %
		   BL::gettext("PLANET_ACTION_CODE_COMMENT")).str();
		std::string const code =
		  "function AI(planet, fleets)\n"
		  "  return noPlanetAction()\n"
		  "end";
		result.push_back(code);
		result.push_back(blocklyCode);
	}

	//Ajout du code du joueur (Fleet)
	{
		std::stringstream blocklyFleetDefaultCode;
		{
			std::ifstream fleetFile("blocklyFleetDefaultCode.xml",
			                        std::ios::binary | std::ios::in);
			if(fleetFile.is_open() == false)
				BOOST_THROW_EXCEPTION(
				  std::ios::failure("Can't open blocklyFleetDefaultCode.xml"));
			boost::iostreams::copy(fleetFile, blocklyFleetDefaultCode);
		}
		std::string const blocklyCode =
		  (boost::format(blocklyFleetDefaultCode.str()) %
		   BL::gettext("my_fleet") %
		   BL::gettext("otherFleet") %
		   BL::gettext("local_planet") %
		   BL::gettext("order") %
		   BL::gettext("DO_GATHER_CODE_COMMENT") %
		   BL::gettext("DO_FIGHT_CODE_COMMENT") %
		   BL::gettext("FLEET_ACTION_CODE_COMMENT") %
		   BL::gettext("myself_player") %
		   BL::gettext("other_player")).str();
		std::string const code =
		  "function AI_do_gather(myFleet, otherFleet)\n"
		  "  return true\n"
		  "end\n"
		  "function AI_do_fight(myFleet, fighter)\n"
		  "  return true\n"
		  "end\n"
		  "function AI_action(myFleet, planet)\n"
		  "  order = FleetAction(FleetAction.Nothing, Direction())\n"
		  "  return order\n"
		  "end\n"
		  "function AI_do_escape(ma_flotte, planete_locale, autres_flottes)\n"
		  "  return simulates(ma_flotte, planete_locale, autres_flottes) < 50\n"
		  "end";
		result.push_back(code);
		result.push_back(blocklyCode);
	}

	result.swap(codes);
}


//! @brief Donne une planète a un nouveau joueur
//! @return la coordonée de la nouvelle planète
Coord createMainPlanet(Universe& univ, Player::ID pid)
{
	bool done = false;
	Coord mainPlanetCoord;

	do
	{
		size_t const planetNumber = rand() % univ.planetMap.size();
		auto planetIter = univ.planetMap.begin();
		std::advance(planetIter, planetNumber);
		Planet& planet = planetIter->second;
		if(planet.playerId == Player::NoId)
		{
			planet.playerId = pid;
			if(planet.playerId > 100000)
				BOOST_THROW_EXCEPTION(
				  std::logic_error("planet.playerId > 100000"));

			mainPlanetCoord = planet.coord;

			planet.buildingList[Building::CommandCenter] = 1;
			planet.ressourceSet = RessourceSet(4000, 400, 0);
			done = true;
		}
	}
	while(done == false);

	return mainPlanetCoord;
}


//! Construit l'Universe et la base de donnée SQL, au premier lancement
//!
//! Si des joueurs sont présent dans la base ils seront conservés mais reinisialisés
void construct(Universe& univ, DataBase& database)
{
	univ.roundCount = 0;

	// Creation des planetes
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
			Planet planet(coord, 0);
			planet.ressourceSet = ress;
			planet.name = nameGen();
			planet.parentCoord = coord;
			univ.planetMap.insert(std::make_pair(coord, planet));
			coordSet.insert(coord);
		}
	}

	// Création des joueurs AI
	std::string const& password = "test";
	std::vector<std::string> codes;
	getNewPlayerCode(codes);
	for(size_t playerCount = 0; playerCount < 100;)
	{
		Player::ID pid = database.addPlayer(nameGen(), password, codes, true);
		if(pid != Player::NoId)
		{
			playerCount += 1;
			Player::SkillTab skillTab;
			skillTab.fill(0);
			skillTab[Skill::Cohesion] = rand() % 10;
			skillTab[Skill::Strategy] = rand() % 10;
			skillTab[Skill::Conquest] = rand() % 10;
			skillTab[Skill::Escape] = rand() % 3;
			database.setPlayerSkills(pid, skillTab);
		}
	}
	auto players = database.getPlayers();
	// Assignement d'une planete a chaque joueurs (AI et non AI)
	for(Player const& player : players)
		database.setPlayerMainPlanet(player.id,
		                             createMainPlanet(univ, player.id));
	// Chargement des codes des IA
	std::string planetScript;
	std::string fleetScript;
	{
		ifstream planetFile("planetScript.lua");
		std::getline(planetFile, planetScript, char(0));
		ifstream fleetFile("fleetScript.lua");
		std::getline(fleetFile, fleetScript, char(0));
	}
	// Definition du code source de chaque joueur (AI et non AI)
	for(Player const& player : players)
	{
		if(player.isAI)
		{
			database.addScript(player.id, CodeData::Planet, planetScript);
			database.addScript(player.id, CodeData::Fleet, fleetScript);
		}
		else
		{
			database.addScript(player.id, CodeData::Planet, codes[0]);
			database.addScript(player.id, CodeData::Fleet, codes[2]);
			database.addBlocklyCode(player.id, CodeData::Planet, codes[1]);
			database.addBlocklyCode(player.id, CodeData::Fleet, codes[3]);
		}
	}
};


//! Serialize l'Universe dans un flux
void saveToStream(Universe const& univ, std::ostream& out)
{
	using namespace boost::iostreams;
	filtering_ostream outFilter;
	outFilter.push(gzip_compressor());
	outFilter.push(out);
	portable_binary_oarchive oa(outFilter);
	clock_t start = clock();
	oa& univ;
	clock_t end = clock();
	std::cout << "Saving time : " << double(end - start) / CLOCKS_PER_SEC <<
	          " sec" << std::endl;
}


//! @brief Déserialize l'Universe depuit un flux
void loadFromStream_v2(std::istream& in, Universe& univ)
{
	using namespace boost::iostreams;
	std::cout << "Loading... ";
	filtering_istream inFilter;
	inFilter.push(gzip_decompressor());
	inFilter.push(in);

	portable_binary_iarchive ia(inFilter);
	ia& univ;
	std::cout << "OK" << std::endl;
}


//! @brief true si un RessourceSet en contient un autre
//!
//! C'est a dire que chaque quantité de chaque type de ressource dans stock
//! est plus grand que dans price
bool canPay(RessourceSet const& stock, RessourceSet const& price)
{
	for(int i = 0; i < Ressource::Count; ++i)
	{
		if(stock.tab[i] < price.tab[i])
			return false;
	}
	return true;
}


//! @brief true si la planète a asser de ressource pour payer price
bool canPay(Planet const& planet, RessourceSet const& price)
{
	return canPay(planet.ressourceSet, price);
}


//! @brief true si la flotte a asser de ressource pour payer price
bool canPay(Fleet const& fleet, RessourceSet const& price)
{
	return canPay(fleet.ressourceSet, price);
}


//! @brief Retire à planète la quantité de ressource demandée
//! @pre La planète peut payer
void pay(Planet& planet, RessourceSet const& price)
{
	if(canPay(planet, price) == false)
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));

	boost::transform(planet.ressourceSet.tab,
	                 price.tab,
	                 planet.ressourceSet.tab.begin(),
	                 std::minus<Ressource::Value>());

	if(planet.ressourceSet.tab[0] > 2000000000)
		BOOST_THROW_EXCEPTION(std::logic_error("Strange ressources value"));
}


//! @brief Retire à flotte la quantité de ressource demandée
//! @pre La flotte peut payer
void pay(Fleet& fleet, RessourceSet const& price)
{
	if(canPay(fleet, price) == false)
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));

	boost::transform(fleet.ressourceSet.tab,
	                 price.tab,
	                 fleet.ressourceSet.tab.begin(),
	                 std::minus<Ressource::Value>());

	if(fleet.ressourceSet.tab[0] > 2000000000)
		BOOST_THROW_EXCEPTION(std::logic_error("Strange ressources value"));
}


//! Test si une planète peut fabriquer un vaisseau dans la quantité demandée
BuildTestState canBuild(Planet const& planet, Ship::Enum type)
{
	if(type < 0 || type >= Ship::Count)
		return BuildTestState::BadValue;
	if(planet.buildingList[Building::Factory] == 0)
		return BuildTestState::FactoryMissing;
	if(planet.task)
		return BuildTestState::OtherTaskRunning;

	RessourceSet price = Ship::List[type].price;
	if(canPay(planet, price) == false)
		return BuildTestState::NotEnoughRessources;

	size_t const hangarSize = boost::accumulate(planet.hangar, 0);
	if(hangarSize)
		return BuildTestState::HangarFull;
	else
		return BuildTestState::Ok;
}


//! Test si une planète peut upgrader un batiment
BuildTestState canBuild(Planet const& planet, Building::Enum type)
{
	if(type < 0 || type >= Building::Count)
		return BuildTestState::BadValue;
	if(planet.buildingList[Building::CommandCenter] == 0)
		return BuildTestState::CommendCenterMissing;
	if(planet.task)
		return BuildTestState::OtherTaskRunning;

	size_t const buNextLevel = planet.buildingList[type] + 1;
	boost::optional<RessourceSet> const price = getBuilingPrice(type, buNextLevel);
	if(price.is_initialized())
	{
		if(false == canPay(planet, price.get()))
			return BuildTestState::NotEnoughRessources;
		else
			return BuildTestState::Ok;
	}
	else
		return BuildTestState::NotEnoughRessources;
}


//! Test si une planète peut fabriquer un canon dans la quantité demandée
BuildTestState canBuild(Planet const& planet, Cannon::Enum type)
{
	if(type < 0 || type >= Cannon::Count)
		return BuildTestState::BadValue;
	if(planet.buildingList[Building::Factory] == 0)
		return BuildTestState::FactoryMissing;
	if(planet.task)
		return BuildTestState::OtherTaskRunning;

	RessourceSet price = Ship::List[type].price;
	if(false == canPay(planet, price))
		return BuildTestState::NotEnoughRessources;
	else
		return BuildTestState::Ok;
}


//! Ajoute une tache d'upgrade de building a la planète
//! @pre building est compris dans [0: Building::Count[
//! @pre La planète a un CommandCenter
//! @pre La planète peut payer
void addTask(Planet& planet, uint32_t roundCount, Building::Enum building)
{
	size_t const buNextLevel =  planet.buildingList[building] + 1;
	size_t const centerLevel = planet.buildingList[Building::CommandCenter];
	if(centerLevel == 0)
		BOOST_THROW_EXCEPTION(
		  std::logic_error("Can't create Building without CommandCenter"));

	double const floatDuration =
	  double(Building::List[building].price.tab[0]) /
	  (centerLevel * pow(1.15, centerLevel) * 4); //-V112
	uint32_t const duration = statRound<uint32_t>(floatDuration);
	PlanetTask task(PlanetTask::UpgradeBuilding, roundCount, duration);
	task.value = building;
	boost::optional<RessourceSet> const price = getBuilingPrice(building, buNextLevel);
	if(price.is_initialized() == false)
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));
	task.startCost = price.get();

	if(canPay(planet, price.get()) == false)
		BOOST_THROW_EXCEPTION(std::logic_error("Can't pay"));

	if(planet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("planet.task is full"));
	planet.task = task;
	pay(planet, price.get());
}


//! Ajoute une tache de fabrication de vaisseau à la planète
//! @pre ship est compris dans [0: Ship::Count[
//! @pre La planète a un Building::Factory
//! @pre La planète peut payer
void addTask(Planet& planet, uint32_t roundCount, Ship::Enum ship, uint32_t number)
{
	size_t const factoryLvl = planet.buildingList[Building::Factory];
	if(factoryLvl == 0)
		BOOST_THROW_EXCEPTION(std::logic_error("Need Factory"));
	//! @todo: Ajouter regle sur durée de fabrication dans Rules
	double const floatDuration =
	  double(Ship::List[ship].price.tab[0]) /
	  (factoryLvl * pow(1.15, factoryLvl) * 4); //-V112
	uint32_t const duration = statRound<uint32_t>(floatDuration);
	PlanetTask task(PlanetTask::MakeShip, roundCount, duration);
	task.value = ship;
	task.value2 = number;
	RessourceSet const& price = Ship::List[ship].price;
	task.startCost = price;
	if(planet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("planet.task is full"));
	planet.task = task;
	pay(planet, price);
}


//! Ajoute une tache de fabrication de canon à la planète
//! @pre cannon est compris dans [0: Cannon::Count[
//! @pre La planète a un Building::Factory
//! @pre La planète peut payer
void addTask(Planet& planet,
             uint32_t roundCount,
             Cannon::Enum cannon,
             uint32_t number)
{
	size_t const factoryLvl = planet.buildingList[Building::Factory];
	if(factoryLvl == 0)
		BOOST_THROW_EXCEPTION(std::logic_error("Need Factory"));
	double const floatDuration =
	  double(Cannon::List[cannon].price.tab[0]) /
	  (factoryLvl * pow(1.15, factoryLvl) * 4); //-V112
	uint32_t const duration = statRound<uint32_t>(floatDuration);
	PlanetTask task(PlanetTask::MakeCannon, roundCount, duration);
	if(cannon >= Cannon::Count)
		BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent cannon type"));
	task.value = cannon;
	task.value2 = number;
	RessourceSet const& price = Cannon::List[cannon].price;
	task.startCost = price;
	if(planet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("planet.task is full"));
	planet.task = task;
	pay(planet, price);
}


//! @brief tTest si il y une tache d'upgrade d'un building pour arreter
//! @todo: canStop
bool canStop(
  Planet const&,// planet,
  Building::Enum// type
)
{
	return true;
}


//! Stop une tache de cette, si la tache existe
void stopTask(Planet& planet,
              PlanetTask::Enum tasktype,
              Building::Enum building)
{
	if(!planet.task)
		return;
	PlanetTask const& task = *planet.task;
	if(task.type == tasktype &&
	   task.value == static_cast<uint32_t>(building))
		planet.task.reset();
}


//! Excecute une tache sur une planète
void execTask(Universe& univ,
              Planet& planet,
              PlanetTask& task,
              std::vector<Event>& events)
{
	if((task.lauchTime + task.duration) <= univ.roundCount)
	{
		switch(task.type)
		{
		case PlanetTask::UpgradeBuilding:
			if(static_cast<size_t>(task.value) >= planet.buildingList.size())
				BOOST_THROW_EXCEPTION(std::logic_error("Bad building type"));
			else
			{
				planet.buildingList[static_cast<size_t>(task.value)] += 1;
				Event event(planet.playerId, time(0), Event::Upgraded);
				event.setValue(task.value).setPlanetCoord(planet.coord);
				events.push_back(event);
			}
			break;
		case PlanetTask::MakeShip:
		{
			planet.hangar[static_cast<size_t>(task.value)] += task.value2;
			Event event(planet.playerId, time(0), Event::ShipMade);
			event.setValue(intptr_t(task.value)).setPlanetCoord(planet.coord);
			events.push_back(event);
		}
		break;
		case PlanetTask::MakeCannon:
			if(task.value < Cannon::Count)
			{
				planet.cannonTab[static_cast<size_t>(task.value)] += 1;
				Event event(planet.playerId, time(0), Event::CannonMade);
				event.setValue(task.value).setPlanetCoord(planet.coord);
				events.push_back(event);
			}
			break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetTask"));
		}
		static_assert(PlanetTask::MakeCannon == (PlanetTask::Count - 1),
		              "Missing cases in switch");
		task.expired = true;
	}
}


//! Excecute une tache sur une flotte
void execTask(Universe& univ,
              Player const& player,
              Fleet& fleet,
              FleetTask& task,
              std::vector<Event>& events)
{
	if((task.lauchTime + task.duration) <= univ.roundCount)
	{
		switch(task.type)
		{
		case FleetTask::Move:
			fleet.coord = task.position;
			break;
		case FleetTask::Harvest:
		{
			Planet& planet = MAP_FIND(univ.planetMap, task.position)->second;
			if(planet.playerId == Player::NoId)
			{
				addArray(fleet.ressourceSet.tab, planet.ressourceSet.tab);
				planet.ressourceSet.tab.fill(0);
				Event event(fleet.playerId, time(0), Event::PlanetHarvested);
				event.setFleetID(fleet.id);
				events.push_back(event);
			}
		}
		break;
		case FleetTask::Colonize:
		{
			using namespace boost;
			Planet& planet = MAP_FIND(univ.planetMap, task.position)->second;
			if(planet.playerId == Player::NoId && fleet.shipList[Ship::Queen])
			{
				if(player.planetCount < getMaxPlanetCount(player))
				{
					Event ev = Event(
					             fleet.playerId,
					             time(0),
					             Event::PlanetColonized)
					           .setFleetID(fleet.id)
					           .setPlanetCoord(planet.coord);
					events.push_back(ev);
					fleet.shipList[Ship::Queen] -= 1;

					planet.buildingList[Building::CommandCenter] = 1;
					addArray(planet.ressourceSet.tab, RessourceSet(2000, 500, 0).tab);
					planet.playerId = fleet.playerId;
					planet.player = fleet.player;
					planet.parentCoord = fleet.origin;
					planet.firstRound = univ.roundCount;
					if(planet.playerId > 100000)
						BOOST_THROW_EXCEPTION(
						  std::logic_error("planet.playerId > 100000"));
				}
			}
		}
		break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown FleetTask"));
		}
		static_assert(FleetTask::Colonize == (FleetTask::Count - 1),
		              "Missing cases in switch");
		task.expired = true;
	}
}


//! @brief Fait le travail d'un building, en connaissant son type et son niveau
//! @todo: Remplacer par une liste de building polymorphic?
void execBuilding(Planet& planet, Building::Enum type, size_t level)
{
	static size_t const speedMult = 4; //-V112
	if(level == 0)
		return;
	switch(type)
	{
	case Building::CommandCenter:
		cappedAdd(planet.ressourceSet.tab[Ressource::Metal], Ressource::Value(1));
		break;
	case Building::MetalMine:
	{
		size_t toAdd = level * size_t(std::pow(1.1, double(level))) * speedMult;
		toAdd = min<size_t>(std::numeric_limits<Ressource::Value>::max(), toAdd);
		if(toAdd > (std::numeric_limits<Ressource::Value>::max() - planet.ressourceSet.tab[Ressource::Metal]))
			planet.ressourceSet.tab[Ressource::Metal] = std::numeric_limits<Ressource::Value>::max();
		else
			planet.ressourceSet.tab[Ressource::Metal] += static_cast<Ressource::Value>(toAdd);
	}
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
	default:
		assert(false && "Unconsistent Building::Enum type");
	};
}


//! Simule la vie de la planète durant un round
void planetRound(Player const& player,
                 Universe& univ,
                 Planet& planet,
                 std::vector<Event>& events)
{
	if(planet.task)
		execTask(univ, planet, *planet.task, events);

	size_t const maxFleetCount = getMaxFleetCount(player);
	if(player.fleetCount < maxFleetCount && boost::accumulate(planet.hangar, 0))
	{
		Fleet newFleet(univ.nextFleetID++,
		               planet.playerId,
		               planet.coord,
		               univ.roundCount);
		newFleet.shipList.swap(planet.hangar);
		newFleet.player = planet.player;
		univ.fleetMap.insert(make_pair(newFleet.id, newFleet));
	}


	if(planet.task && planet.task->expired)
		planet.task.reset();

	for(size_t type = 0; type < planet.buildingList.size(); ++type)
		execBuilding(planet, Building::Enum(type), planet.buildingList[type]);

	//Cristalisations des ressources
	if((rand() % 10) == 0) //! @todo: rand rapide
	{
		cappedAdd(planet.ressourceSet.tab[Ressource::Metal], Ressource::Value(rand() % 7));
		cappedAdd(planet.ressourceSet.tab[Ressource::Carbon], Ressource::Value(rand() % 5));
		cappedAdd(planet.ressourceSet.tab[Ressource::Loicium], Ressource::Value(rand() % 3));
	}

	//Limitation des ressources à un milliard
	for(auto& val : planet.ressourceSet.tab)
		val = std::min(val, Ressource::Value(1000000000));
}


//! Simule la vie de la flotte durant un round
void fleetRound(Universe& univ,
                Player const& player,
                Fleet& fleet,
                std::vector<Event>& events)
{
	if(fleet.task)
		execTask(univ, player, fleet, *fleet.task, events);

	if(fleet.task && fleet.task->expired)
		fleet.task.reset();

	//Limitation des ressources à un milliard
	for(auto& val : fleet.ressourceSet.tab)
		val = std::min(val, Ressource::Value(1000000000));
}

//! Ajoute otherFleet dans fleet
void gather(Fleet& fleet, Fleet const& otherFleet)
{
	addArray(fleet.ressourceSet.tab, otherFleet.ressourceSet.tab);
	addArray(fleet.shipList, otherFleet.shipList);
}

//! Test si une flotte peut se rendre a la destination coord
FleetActionTest canMove(Fleet const& fleet,
                        Coord const& coord //Destination en valeur absolue
                       )
{
	if(fleet.task)
		return FleetActionTest::OtherTaskRunning;
	if(abs(fleet.coord.X - coord.X) > 1 ||
	   abs(fleet.coord.Y - coord.Y) > 1 ||
	   abs(fleet.coord.Z - coord.Z) > 1)
		return FleetActionTest::TooFarAway;
	if(coord.X < 0 || coord.X >= Universe::MapSizeX ||
	   coord.Y < 0 || coord.Y >= Universe::MapSizeY ||
	   coord.Z < 0 || coord.Z >= Universe::MapSizeZ)
		return FleetActionTest::OutOfGalaxy;
	//! @todo: Gestion du caburant
	return FleetActionTest::Ok;
}


//! Ajoute une tache de déplacement dans la flotte
//! @pre la flotte peut se rendre a cet coordonée (canMove)
void addTaskMove(Fleet& fleet, uint32_t roundCount, Coord const& coord)
{
	if(fleet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("fleet.task is not NULL!"));
	FleetTask task(FleetTask::Move, roundCount, 1);
	task.position = coord;
	fleet.task = task;
}


//! Test si la flotte peut récolter la planète
FleetActionTest canHarvest(Fleet const& fleet, Planet const* planet)
{
	if(fleet.task)
		return FleetActionTest::OtherTaskRunning;
	else if(planet == nullptr)
		return FleetActionTest::NoPlanet;
	else if(planet->playerId)
		return FleetActionTest::PlanetHasOwner;
	else
		return FleetActionTest::Ok;
}


//! Ajoute une tache de récolte dans la flotte
//! @pre la flotte peut récolter la planète (canHarvest)
void addTaskHarvest(Fleet& fleet, uint32_t roundCount, Planet const& planet)
{
	if(fleet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("fleet.task is not NULL!"));
	FleetTask task(FleetTask::Harvest, roundCount, 1);
	task.position = planet.coord;
	fleet.task = task;
}


//! Test si la flotte peut colonizer la planète
FleetActionTest canColonize(Player const& player,
                            Fleet const& fleet,
                            Planet const* planet,
                            size_t planetCount)
{
	if(fleet.task)
		return FleetActionTest::OtherTaskRunning;
	else if(planet == nullptr)
		return FleetActionTest::NoPlanet;
	else if(planet->playerId != Player::NoId)
		return FleetActionTest::PlanetHasOwner;
	else if(fleet.shipList[Ship::Queen] == 0)
		return FleetActionTest::QueenMissing;
	return InternalRules::canColonize(player, fleet, *planet, planetCount);
}


//! Ajoute une tache de récolte dans la flotte
//! @pre la flotte peut colonizer la planète (canColonize)
void addTaskColonize(Fleet& fleet, uint32_t roundCount, Planet const& planet)
{
	if(fleet.task)
		BOOST_THROW_EXCEPTION(std::logic_error("fleet.task is not NULL!"));
	FleetTask task(FleetTask::Colonize, roundCount, 1);
	task.position = planet.coord;
	fleet.task = task;
}


//! Test si la flotte peut balancer ses ressources sur la planète
FleetActionTest canDrop(Fleet const& fleet, Planet const* planet)
{
	if(planet == nullptr)
		return FleetActionTest::NoPlanet;
	Ressource::Value const ressCount = boost::accumulate(fleet.ressourceSet.tab, 0);
	if(planet->playerId != fleet.playerId)
		return FleetActionTest::NotYourOwnPlanet;
	else if(ressCount == 0)
		return FleetActionTest::NotEnoughRessources;
	else
		return FleetActionTest::Ok;
}


//! La flotte balance ses ressources sur la planète
//! @pre la flotte peut balancer ses ressources sur la planète (canDrop)
void drop(Fleet& fleet, Planet& planet)
{
	addArray(planet.ressourceSet.tab, fleet.ressourceSet.tab);
	fleet.ressourceSet.tab.fill(0);
}


FleetActionTest canGather(
  Player const& player,
  Fleet const& fleet1,
  Fleet const& fleet2)
{
	return InternalRules::canGather(player, fleet1, fleet2);
}

//! test si la flotte est vide
bool emptyFleet(Fleet const& fleet)
{
	return boost::accumulate(fleet.shipList, 0) == 0;
}

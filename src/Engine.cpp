#include "Engine.h"

#include <fstream>
#include <vector>
#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/optional.hpp>

//#include "PythonUniverse.h"
#include "LuaUniverse.h"
#include "LuaTools.h"
#include "Tools.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


using namespace std;
using namespace boost;

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;


Engine::Engine():
	simulation_(univ_)
{
	construct(univ_);
	start();
}


Engine::~Engine()
{
	stop();
}


void Engine::start()
{
	simulating_ = boost::thread(&Simulation::loop, boost::ref(simulation_));
}


void Engine::stop()
{
	simulating_.interrupt();
	simulating_.join();
}


/*void Engine::construct()
{
	UniqueLock lock(mutex_);
	::construct(univ_);
}*/


void Engine::load(std::string const& univName)
{
	UniqueLock lock(univ_.mutex);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(std::ios::failure("Can't load from " + univName));
	loadFromStream(loadFile, univ_);
}


void Engine::save(std::string const& saveName) const
{
	SharedLock lock(univ_.mutex);

	using namespace std;
	std::string const newSaveName = saveName + ".new";
	{
		ofstream saveFile(newSaveName.c_str(), ios::out | ios::binary);
		if(saveFile.is_open() == false)
			BOOST_THROW_EXCEPTION(std::ios::failure("Can't save in " + newSaveName));
		saveToStream(univ_, saveFile);
	}
	std::string const ansSaveName = saveName + ".ans";
	remove(ansSaveName.c_str());
	struct stat buf;
	if(stat(saveName.c_str(), &buf) == 0)
		rename(saveName.c_str(), ansSaveName.c_str());
	rename(newSaveName.c_str(), saveName.c_str());
	//std::cout << "OK" << std::endl;
}


void Engine::addPlayer(Player const&) //player
{
	UniqueLock lock(univ_.mutex);
	//TODO
}


std::vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	std::vector<Fleet> fleetList;
	BOOST_FOREACH(Fleet const & fleet, univ_.fleetMap | boost::adaptors::map_values)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(fleet);
	}
	return fleetList;
}


std::vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	std::vector<Planet> planetList;
	BOOST_FOREACH(Universe::PlanetMap::value_type const & planetNVP, univ_.planetMap)
	{
		if(planetNVP.second.playerId == pid)
			planetList.push_back(planetNVP.second);
	}
	return planetList;
}


void Engine::setPlayerFleetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.mutex);
	mapFind(univ_.playerMap, pid)->second.fleetsCode = code;
	simulation_.reloadPlayer(pid);
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.mutex);
	mapFind(univ_.playerMap, pid)->second.planetsCode = code;
	simulation_.reloadPlayer(pid);
}


std::string Engine::getPlayerFleetCode(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return 	mapFind(univ_.playerMap, pid)->second.fleetsCode;
}


std::string Engine::getPlayerPlanetCode(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return 	mapFind(univ_.playerMap, pid)->second.planetsCode;
}


std::vector<Player> Engine::getPlayers() const
{
	SharedLock lock(univ_.mutex);
	std::vector<Player> playerList;
	BOOST_FOREACH(Universe::PlayerMap::value_type const & nvp, univ_.playerMap)
	{
		playerList.push_back(nvp.second);
	}
	return playerList;
}

Player Engine::getPlayer(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.playerMap, pid)->second;
}

Planet Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.planetMap, coord)->second;
}

Fleet Engine::getFleet(Fleet::ID fid)
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.fleetMap, fid)->second;
}


boost::optional<Player> Engine::getPlayer(
	std::string const& login, std::string const& password) const
{
	auto iter = boost::find_if(univ_.playerMap, [&]
	(Universe::PlayerMap::value_type const& player)
	{
		return player.second.login == login && player.second.password == password;
	});
	
	if(iter == univ_.playerMap.end())
		return false;
	else
		return iter->second;
}


//   -------   PRIVEE   -------------------------------------------------------

luabind::object Engine::Simulation::registerCode(
  LuaTools::LuaEngine& luaEngine,
  Player::ID const pid, std::string const& code, time_t time)
try
{
	using namespace luabind;
	using namespace std;

	if(luaL_dostring(luaEngine.state(), code.c_str()) != 0)
	{
		std::string message = lua_tostring(luaEngine.state(), -1);
		mapFind(univ_.playerMap, pid)->second.eventList.push_back(
		  Event(time, Event::FleetCodeError, message));
		return luabind::object();
	}
	else
	{
		luabind::object g = luabind::globals(luaEngine.state());
		return g["AI"];
	}
}
catch(std::exception const& ex)
{
	std::string message = lua_tostring(luaEngine.state(), -1);
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(
	  Event(time, Event::FleetCodeError, ex.what() + string(" ") + message));
	return luabind::object();
}


void Engine::Simulation::execPlanet(luabind::object code, Planet& planet, time_t time)
try
{
	if(false == code.is_valid())
		return;
	PlanetActionList list;
	code(boost::cref(planet), boost::ref(list));

	BOOST_FOREACH(PlanetAction const & action, list)
	{
		switch(action.action)
		{
		case PlanetAction::Building:
		{
			if(canBuild(planet, action.building))
				addTask(planet, univ_.time, action.building);
		}
		break;
		case PlanetAction::StopBuilding:
		{
			if(canStop(planet, action.building))
				stopTask(planet, PlanetTask::UpgradeBuilding, action.building);
		}
		case PlanetAction::Ship:
		{
			if(canBuild(planet, action.ship, action.number))
				addTask(planet, univ_.time, action.ship, action.number);
		}
		break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetAction::Type"));
		};
	}
}
catch(luabind::error& e)
{
  luabind::object error_msg(luabind::from_stack(e.state(), -1));
	std::stringstream ss;
	ss << error_msg;
  mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(
	  Event(time, Event::FleetCodeError, ss.str()));
}
catch(std::exception const& ex)
{
	mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(
	  Event(time, Event::FleetCodeError, ex.what()));
}

bool Engine::Simulation::execFleet(luabind::object code, Fleet& fleet, FleetCoordMap& fleetMap, time_t time)
try
{
	if(false == code.is_valid())
		return true;
	//! Si flotte allié
	//! - Si rassemblement = oui ET oui
	//!   rassemblement
	//!
	//! Si flotte enemie:
	//! - Si combat? : oui OU oui
	//!   - excecution du script de préparation
	//!     Combat
	//!
	//! Si planete enemie
	//! - Si combat : oui
	//!   - excecution du script de préparation
	//!     Combat
	//!
	//! Sinon Si planete libre ET (flotte veux récolter ou coloniser)
	//!   - Si Récolte:
	//!     recolte
	//!   - Si Construire:
	//!     construction : contre de commandement(la planete appartien au joueur)
	//! Sinon Si la flotte veux se déplacer:
	//! - Déplacement

	//! Gestion flottes alliées
	auto localFleetsKV = fleetMap.equal_range(fleet.coord);
	auto fleetIter = localFleetsKV.first;
	while(fleetIter != localFleetsKV.second)
	{
		Fleet& otherFleet = fleetIter->second;
		if((otherFleet.id > fleet.id) && (otherFleet.playerId == fleet.playerId))
		{
			bool const wantGather1 = luabind::call_member<bool>(code, "do_gather", boost::cref(fleet), boost::cref(otherFleet));
			bool const wantGather2 = luabind::call_member<bool>(code, "do_gather", boost::cref(otherFleet), boost::cref(fleet));
			if(wantGather1 && wantGather2)
			{
				gather(fleet, otherFleet);
				auto condemned = fleetIter;
				++fleetIter;
				fleetMap.erase(condemned);
				fleet.eventList.push_back(
				  Event(time, Event::FleetsGather, "Rassemblement des flottes"));
				//mapFind(univ_.planetMap, fleet)
				continue;
			}
		}
		++fleetIter;
	}

	//! Gestion flottes enemies
	localFleetsKV = fleetMap.equal_range(fleet.coord);
	fleetIter = localFleetsKV.first;
	while(fleetIter != localFleetsKV.second)
	{
		Fleet& otherFleet = fleetIter->second;
		if((otherFleet.id != fleet.id) && (otherFleet.playerId != fleet.playerId))
		{
			if(luabind::call_member<bool>(code, "do_fight", boost::cref(fleet), boost::cref(otherFleet)))
			{
				boost::tribool result = fight(fleet, otherFleet);
				if(result == true)
				{
					auto condamned = fleetIter;
					++fleetIter;
					fleetMap.erase(condamned);
					fleet.eventList.push_back(
					  Event(time, Event::FleetWin, "Victoire"));
					continue;
				}
				else if(result == false)
				{
					otherFleet.eventList.push_back(
					  Event(time, Event::FleetWin, "Victoire"));
					return false;
				}
			}
		}
		++fleetIter;
	}

	auto planetIter = univ_.planetMap.find(fleet.coord);
	boost::optional<Planet> planet;
	if(planetIter != univ_.planetMap.end())
	{
		planet = planetIter->second;
		/*if(planetIter->second.playerId == Player::NoId)
		{
			FleetAction action =
				boost::python::extract<FleetAction>(code.attr("action")(fleet, planet));
		}*/
	}
	using namespace luabind;
	FleetAction action(FleetAction::Nothing);
	if(planet)
		action = luabind::call_member<FleetAction>(code, "action", boost::cref(fleet), boost::cref(*planet));
	else
		action = luabind::call_member<FleetAction>(code, "action", boost::cref(fleet), false);
	switch(action.action)
	{
	case FleetAction::Nothing:
		break;
	case FleetAction::Move:
	{
		Coord target = fleet.coord;
		target.X += (rand() % 3) - 1;
		target.Y += (rand() % 3) - 1;
		target.Z += (rand() % 3) - 1;
		if(canMove(fleet, target))
			addTask(fleet, univ_.time, target);
	}
	break;
	case FleetAction::Harvest:
	{
		if(planet && canHarvest(fleet, *planet))
			addTaskHarvest(fleet, univ_.time, *planet);
	}
	break;
	case FleetAction::Colonize:
		//TODO
		break;
	}


	/*FleetActionList list;
	code(fleet, boost::ref(list));
	BOOST_FOREACH(FleetAction const & action, list)
	{
		switch(action.action)
		{
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetAction::Type"));
		};
	}*/

	return true;
}
catch(luabind::error& e)
{
  luabind::object error_msg(luabind::from_stack(e.state(), -1));
	std::stringstream ss;
	ss << error_msg;
  mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(
	  Event(time, Event::FleetCodeError, ss.str()));
	return true;
}
catch(std::exception const& ex)
{
	mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(
	  Event(time, Event::FleetCodeError, ex.what()));
	return true;
}

static size_t const RoundSecond = 1;


void Engine::Simulation::round(LuaTools::LuaEngine& luaEngine, PlayerCodeMap& codesMap)
try
{
	std::cout << time(0) << " ";

	//std::cout << "Mise a jour";
	UniqueLock lock(univ_.mutex);

	univ_.time += RoundSecond;

	//Rechargement des codes flote/planet des joueurs dont le code a été changé
	BOOST_FOREACH(Player::ID pid, playerToReload_)
	{
		Player const& player = mapFind(univ_.playerMap, pid)->second;
		PlayerCodes newCodes =
		{
			registerCode(luaEngine, player.id, player.fleetsCode, univ_.time),
			registerCode(luaEngine, player.id, player.planetsCode, univ_.time)
		};
		codesMap[player.id] = newCodes;
	}
	playerToReload_.clear();

	//Les planètes
	BOOST_FOREACH(Universe::PlanetMap::value_type & planetNVP, univ_.planetMap)
	{
		Planet& planet = planetNVP.second;
		planetRound(univ_, planet, univ_.time);
		if(planet.playerId != Player::NoId)
			execPlanet(codesMap[planet.playerId].planetsCode, planet, univ_.time);
	}

	{
		FleetCoordMap fleetMap;
		BOOST_FOREACH(Fleet & fleet, univ_.fleetMap | boost::adaptors::map_values)
			fleetMap.insert(make_pair(fleet.coord, fleet));

		auto iter = fleetMap.begin();
		while(iter != fleetMap.end())
		{
			fleetRound(univ_, iter->second, univ_.time);

			bool keepFleet = execFleet(codesMap[iter->second.playerId].fleetsCode, iter->second, fleetMap, univ_.time);
			if(keepFleet == false)
			{
				auto condemned = iter;
				++iter;
				fleetMap.erase(condemned);
			}
			else
				++iter;
		}

		std::map<Fleet::ID, Fleet> newFleetMap;
		BOOST_FOREACH(Fleet & fleet, fleetMap | boost::adaptors::map_values)
			newFleetMap.insert(make_pair(fleet.id, fleet));
		newFleetMap.swap(univ_.fleetMap);
	}

	std::cout << time(0) << std::endl;
}
catch(std::exception const& ex)
{
	std::cout << boost::diagnostic_information(ex) << std::endl;
}


void Engine::Simulation::loop()
{
	LuaTools::LuaEngine luaEngine;
	PlayerCodeMap codesMap;

	luaL_openlibs(luaEngine.state());

	initDroneWars(luaEngine.state());

	{
		UniqueLock lock(univ_.mutex);

		//Chargement de tout les code flote/planet de tout les joueur(chargement dans python)
		BOOST_FOREACH(Universe::PlayerMap::value_type & playerNVP, univ_.playerMap)
		{
			Player const& player = playerNVP.second;
			PlayerCodes newCodes =
			{
				registerCode(luaEngine, player.id, player.fleetsCode, univ_.time),
				registerCode(luaEngine, player.id, player.planetsCode, univ_.time)
			};
			codesMap[player.id] = newCodes;
		}
	}


	time_t newUpdate;
	time(&newUpdate);
	newUpdate += RoundSecond;

	while(false == boost::this_thread::interruption_requested())
	{
		time_t now;
		time(&now);

		if(newUpdate <= now)
		{
			//std::cout << newUpdate << " " << now << std::endl;
			round(luaEngine, codesMap);
			newUpdate += RoundSecond;
		}
		else
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}



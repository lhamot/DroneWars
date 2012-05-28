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
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/zip_iterator.hpp>

//#include "PythonUniverse.h"
#include "LuaUniverse.h"
#include "LuaTools.h"
#include "Tools.h"
#include "fighting.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


using namespace std;
using namespace boost;
using namespace LuaTools;

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;


Engine::Engine():
	simulation_(univ_)
{
	boost::filesystem::directory_iterator dir("."), end;

	time_t maxtime = 0;
	BOOST_FOREACH(const boost::filesystem::path & p, std::make_pair(dir, end))
	{
		std::string const fileStr = p.filename().string();
		if(fileStr.find("_save.bta") == 10 && fileStr.size() == 19)
		{
			std::string const strTime = fileStr.substr(0, 10);
			time_t filetime = strtoul(strTime.c_str(), 0, 10);
			maxtime = max(maxtime, filetime);
		}
	}
	if(maxtime)
	{
		std::stringstream ss;
		ss << maxtime << "_save.bta";
		load(ss.str());
	}
	else
		construct(univ_);
	start();
}


void Engine::load(std::string const& univName)
{
	UniqueLock lock(univ_.mutex);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(std::ios::failure("Can't load from " + univName));
	loadFromStream(loadFile, univ_);
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



bool Engine::addPlayer(std::string const& login, std::string const& password)
{
	UniqueLock lock(univ_.mutex);

	if(login.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("login"));
	if(password.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("password"));

	auto iter = boost::find_if(univ_.playerMap, [&]
	                           (Universe::PlayerMap::value_type const & keyValue)
	{
		return keyValue.second.login == login;
	});
	if(iter != univ_.playerMap.end())
		return false;

	Player::ID const pid = createPlayer(univ_, login, password);
	simulation_.reloadPlayer(pid);
	return true;
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
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setCode(code);
	simulation_.reloadPlayer(pid);
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.mutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.planetsCode.setCode(code);
	simulation_.reloadPlayer(pid);
}


CodeData Engine::getPlayerFleetCode(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return 	mapFind(univ_.playerMap, pid)->second.fleetsCode;
}


CodeData Engine::getPlayerPlanetCode(Player::ID pid) const
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
	                           (Universe::PlayerMap::value_type const & player)
	{
		return player.second.login == login && player.second.password == password;
	});

	if(iter == univ_.playerMap.end())
		return false;
	else
		return iter->second;
}

FightReport Engine::getFightReport(size_t id)
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.reportMap, id)->second;
}


//   -------   PRIVEE   -------------------------------------------------------
static size_t const LuaMaxInstruction = 20000;
static size_t const MaxCodeExecTry = 10;

void luaCountHook(lua_State* L, lua_Debug* ar)
{
	lua_sethook(L, luaCountHook, LUA_MASKCOUNT, 1);
	luaL_error(L, "timeout was reached");
}

Engine::Simulation::Simulation(Universe& univ):
	univ_(univ)
{
}


luabind::object Engine::Simulation::registerCode(
  LuaTools::LuaEngine& luaEngine,
  Player::ID const pid, CodeData& code, time_t time, bool isFleet)
try
{
	using namespace luabind;
	using namespace std;

	luaL_dostring(luaEngine.state(), "AI = nil");

	if(luaL_dostring(luaEngine.state(), code.getCode().c_str()) != 0)
	{
		char const* message = lua_tostring(luaEngine.state(), -1);
		code.newError(message);
		mapFind(univ_.playerMap, pid)->second.eventList.push_back(
		  Event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, message ? message : ""));
		return luabind::object();
	}
	else
	{
		luabind::object g = luabind::globals(luaEngine.state());
		return g["AI"];
	}
}
catch(luabind::error& ex)
{
	std::string message = GetLuabindErrorString(ex);
	code.newError(message);
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(
	  Event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, message));
	return luabind::object();
}
catch(std::exception const& ex)
{
	char const* message = lua_tostring(luaEngine.state(), -1);
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(
	  Event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, ex.what() + string(" ") + (message ? message : "")));
	return luabind::object();
}


void Engine::Simulation::execPlanet(
  LuaEngine& luaEngine, luabind::object code, Planet& planet, time_t time, std::vector<Fleet const*> const& fleetList)
try
{
	if(false == code.is_valid() || luabind::type(code) != LUA_TFUNCTION)
		return;
	PlanetActionList list;
	lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
	luabind::call_function<void>(code, boost::cref(planet), boost::cref(fleetList), boost::ref(list));

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
catch(luabind::error& ex)
{
	std::string const message = GetLuabindErrorString(ex);
	Player& player = mapFind(univ_.playerMap, planet.playerId)->second;
	player.planetsCode.newError(message);//ex.what() + string(" ") + ss.str());
	player.eventList.push_back(Event(univ_.nextEventID++, time, Event::PlanetCodeError, message));//ex.what() + string(" ") + ss.str()));
}
catch(std::exception const& ex)
{
	mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(
	  Event(univ_.nextEventID++, time, Event::FleetCodeError, ex.what()));
}

bool Engine::Simulation::execFleet(LuaEngine& luaEngine, luabind::object code, Fleet& fleet, FleetCoordMap& fleetMap, time_t time)
try
{
	if(false == code.is_valid())
		return true;

	auto localFleetsKV = fleetMap.equal_range(fleet.coord);
	auto fleetIter = localFleetsKV.first;
	while(fleetIter != localFleetsKV.second)
	{
		Fleet& otherFleet = fleetIter->second;
		if((otherFleet.id > fleet.id) && (otherFleet.playerId == fleet.playerId))
		{
			lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
			bool const wantGather1 = luabind::call_member<bool>(code, "do_gather", boost::cref(fleet), boost::cref(otherFleet));
			lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
			bool const wantGather2 = luabind::call_member<bool>(code, "do_gather", boost::cref(otherFleet), boost::cref(fleet));
			if(wantGather1 && wantGather2)
			{
				gather(fleet, otherFleet);
				auto condemned = fleetIter;
				++fleetIter;
				fleetMap.erase(condemned);
				fleet.eventList.push_back(
				  Event(univ_.nextEventID++, time, Event::FleetsGather));
				//mapFind(univ_.planetMap, fleet)
				continue;
			}
		}
		++fleetIter;
	}

	auto planetIter = univ_.planetMap.find(fleet.coord);
	Planet* planet = nullptr;
	if(planetIter != univ_.planetMap.end())
		planet = &(planetIter->second);
	using namespace luabind;
	FleetAction action(FleetAction::Nothing);
	lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
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
		target.X += action.target.X;
		target.Y += action.target.Y;
		target.Z += action.target.Z;
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
		if(planet && canColonize(fleet, *planet))
			addTaskColonize(fleet, univ_.time, *planet);
		break;
	case FleetAction::Drop:
	{
		if(planet && canDrop(fleet, *planet))
			drop(fleet, *planet);
		fleet.eventList.push_back(
		  Event(univ_.nextEventID++, time, Event::FleetDrop));
		planet->eventList.push_back(
		  Event(univ_.nextEventID++, time, Event::FleetDrop));
	}
	break;
	}

	return true;
}
catch(luabind::error& ex)
{
	std::string const message = GetLuabindErrorString(ex);
	Player& player = mapFind(univ_.playerMap, fleet.playerId)->second;
	player.fleetsCode.newError(message);
	player.eventList.push_back(Event(univ_.nextEventID++, time, Event::FleetCodeError, message));
	return true;
}
catch(std::exception const& ex)
{
	mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(
	  Event(univ_.nextEventID++, time, Event::FleetCodeError, ex.what()));
	return true;
}

static size_t const RoundSecond = 1;
static size_t const SaveSecond = 60;


void Engine::Simulation::round(LuaTools::LuaEngine& luaEngine, PlayerCodeMap& codesMap)
try
{
	std::cout << time(0) << " ";

	//std::cout << "Mise a jour";
	UniqueLock lock(univ_.mutex);

	univ_.time += RoundSecond;

	//Désactivation de tout les codes qui echoue
	BOOST_FOREACH(Player const & player, univ_.playerMap | boost::adaptors::map_values)
	{
		if(player.planetsCode.getFailCount() >= MaxCodeExecTry)
			codesMap[player.id].planetsCode = luabind::object();
		if(player.fleetsCode.getFailCount() >= MaxCodeExecTry)
			codesMap[player.id].fleetsCode = luabind::object();
	}

	//Rechargement des codes flote/planet des joueurs dont le code a été changé
	{
		UniqueLock lockReload(mutex_);
		BOOST_FOREACH(Player::ID pid, playerToReload_)
		{
			Player& player = mapFind(univ_.playerMap, pid)->second;
			PlayerCodes newCodes =
			{
				registerCode(luaEngine, player.id, player.fleetsCode, univ_.time, true),
				registerCode(luaEngine, player.id, player.planetsCode, univ_.time, false)
			};
			codesMap[player.id] = newCodes;
		}
		playerToReload_.clear();
	}

	{
		FleetCoordMap fleetMap;
		BOOST_FOREACH(Fleet & fleet, univ_.fleetMap | boost::adaptors::map_values)
			fleetMap.insert(make_pair(fleet.coord, fleet));

		//Les planètes
		std::vector<Fleet const*> fleetList;
		BOOST_FOREACH(Universe::PlanetMap::value_type & planetNVP, univ_.planetMap)
		{
			Planet& planet = planetNVP.second;
			if(planet.eventList.size() > 10)
				planet.eventList.erase(planet.eventList.begin(), planet.eventList.end() - 10);

			fleetList.clear();
			auto localFleets = fleetMap.equal_range(planet.coord);
			auto getFleetPointer = [](FleetCoordMap::value_type const & coordFleet) {return &coordFleet.second;};
			boost::transform(localFleets, back_inserter(fleetList), getFleetPointer);
			//boost::copy(localFleets | boost::adaptors::map_values, back_inserter(fleetList));
			planetRound(univ_, planet, univ_.time);
			if(planet.playerId != Player::NoId)
				execPlanet(luaEngine, codesMap[planet.playerId].planetsCode, planet, univ_.time, fleetList);
		}
	}

	//Les combats
	if(false == univ_.fleetMap.empty()) //si il y as des flottes
	{
		vector<Fleet::ID> deadFleets;
		deadFleets.reserve(univ_.fleetMap.size());
		typedef std::multimap<Coord, Fleet*, CompCoord> FleetCoordMultimap;
		FleetCoordMultimap fleetMultimap;
		BOOST_FOREACH(Fleet & fleet, univ_.fleetMap | boost::adaptors::map_values)
			fleetMultimap.insert(make_pair(fleet.coord, &fleet));

		std::vector<Fleet*> fleetVect;
		// Pour chaque coordonées, on accede au range des flotes
		for(FleetCoordMultimap::iterator iter1 = fleetMultimap.begin(), iter2 = nextNot(fleetMultimap, iter1);
		    iter1 != fleetMultimap.end();
		    iter1 = iter2, iter2 = nextNot(fleetMultimap, iter1))
		{
			auto fleetRange = make_pair(iter1, iter2);

			//Si 0 ou 1 vaisseau, on passe
			auto testShipNumber = iter1;
			if(testShipNumber == iter2)
				continue;
			++testShipNumber;
			if(testShipNumber == iter2)
				continue;

			//On lance le combat
			fleetVect.clear();
			boost::copy(fleetRange | boost::adaptors::map_values, back_inserter(fleetVect));
			FightReport fightReportVect;
			fight(fleetVect, fightReportVect);
			bool hasFight = false;
			auto range = make_zip_range(fleetVect, fightReportVect);
			BOOST_FOREACH(auto fleetReportPair, range)
			{
				FleetReport const& report = fleetReportPair.get<1>();
				hasFight = hasFight | report.hasFight;
			}
			//Si personne ne c'est batue, on passe
			if(hasFight == false)
				continue;

			//On ajoute le rapport dans la base de donné
			size_t const reportID = univ_.nextFightID;
			univ_.nextFightID += 1;
			univ_.reportMap.insert(make_pair(reportID, fightReportVect));
			//On ajoute les evenement/message dans les flottes/joueur
			BOOST_FOREACH(auto fleetReportPair, range)
			{
				Fleet& fleet = *fleetReportPair.get<0>();
				FleetReport const& report = fleetReportPair.get<1>();
				if(report.isDead)
				{
					deadFleets.push_back(fleet.id);
					mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(
					  Event(univ_.nextEventID++, univ_.time, Event::FleetLose, reportID));
				}
				else if(report.hasFight)
				{
					fleet.eventList.push_back(
					  Event(univ_.nextEventID++, univ_.time, Event::FleetWin, reportID));
				}
			}
		}

		//Suppression de toute les flottes mortes
		boost::for_each(deadFleets, [&](Fleet::ID fleetID)
		{
			//cout << "Supresion flotte : " << fleetID << endl;
			univ_.fleetMap.erase(fleetID);
		});
	}

	//Les flottes
	{
		FleetCoordMap fleetMap;
		BOOST_FOREACH(Fleet & fleet, univ_.fleetMap | boost::adaptors::map_values)
			fleetMap.insert(make_pair(fleet.coord, fleet));

		auto iter = fleetMap.begin();
		while(iter != fleetMap.end())
		{
			fleetRound(univ_, iter->second, univ_.time);

			bool keepFleet = execFleet(luaEngine, codesMap[iter->second.playerId].fleetsCode, iter->second, fleetMap, univ_.time);
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

		BOOST_FOREACH(Fleet & fleet, univ_.fleetMap | boost::adaptors::map_values)
		{
			if(fleet.eventList.size() > 10)
				fleet.eventList.erase(fleet.eventList.begin(), fleet.eventList.end() - 10);
		}
	}

	set<size_t> usedReport;
	BOOST_FOREACH(Player & player, univ_.playerMap | boost::adaptors::map_values)
	{
		if(player.eventList.size() > 10)
			player.eventList.erase(player.eventList.begin(), player.eventList.end() - 10);
		BOOST_FOREACH(Event const & ev, player.eventList)
		{
			if(ev.type == Event::FleetLose)
				usedReport.insert(ev.value);
		}
	}

	map_remove_erase_if(univ_.reportMap,
	                    [&](Universe::ReportMap::value_type const & reportKV)
	{
		return usedReport.count(reportKV.first) == false;
	});


	//std::cout << lexical_cast<std::string>(time(0)) + "_save.bta ";
	//save(lexical_cast<std::string>(time(0)) + "_save.bta");

	/*
	size_t playerSize = 0;
	BOOST_FOREACH(auto const& playerKV, univ_.playerMap)
	{
		playerSize += sizeof(playerKV);
		playerSize += playerKV.second.fleetsCode.getCode().size();
		playerSize += playerKV.second.planetsCode.getCode().size();
		playerSize += playerKV.second.login.size();
		playerSize += playerKV.second.password.size();
		BOOST_FOREACH(auto const& ev, playerKV.second.eventList)
		{
			playerSize += sizeof(ev);
			playerSize += ev.comment.size();
		}
	}

	size_t planetSize = 0;
	BOOST_FOREACH(auto const& planetKV, univ_.planetMap)
	{
		planetSize += sizeof(planetKV);
		planetSize += planetKV.second.buildingMap.size() * sizeof(Planet::BuildingMap::value_type);
		planetSize += planetKV.second.taskQueue.size() * sizeof(PlanetTask);
		BOOST_FOREACH(auto const& ev, planetKV.second.eventList)
		{
			planetSize += sizeof(ev);
			planetSize += ev.comment.size();
		}
	}

	size_t fleetSize = 0;
	BOOST_FOREACH(auto const& fleetKV, univ_.fleetMap)
	{
		fleetSize += sizeof(fleetKV);
		fleetSize += fleetKV.second.shipList.size() * sizeof(Fleet::ShipTab::value_type);
		fleetSize += fleetKV.second.taskQueue.size() * sizeof(PlanetTask);
		BOOST_FOREACH(auto const& ev, fleetKV.second.eventList)
		{
			fleetSize += sizeof(ev);
			fleetSize += ev.comment.size();
		}
	}

	size_t reportSize = 0;
	BOOST_FOREACH(auto const& reportKV, univ_.reportMap)
	{
		reportSize += sizeof(reportKV);
		BOOST_FOREACH(auto const& fleetReport, reportKV.second)
		{
			reportSize += sizeof(fleetReport);
			reportSize += fleetReport.enemySet.size() * sizeof(size_t);
			BOOST_FOREACH(auto const& ev, fleetReport.fleetsAfter.eventList)
			{
				reportSize += sizeof(ev);
				reportSize += ev.comment.size();
			}
			BOOST_FOREACH(auto const& ev, fleetReport.fleetsBefore.eventList)
			{
				reportSize += sizeof(ev);
				reportSize += ev.comment.size();
			}
		}
	}

	size_t const univSize = playerSize + planetSize + fleetSize + reportSize;
	cout << "Univ global size :" << univSize << endl;
	cout << "  playerMap:" << playerSize << endl;
	cout << "  planetMap:" << planetSize << endl;
	cout << "  fleetMap:" << fleetSize << endl;
	cout << "  reportMap:" << reportSize << endl;
	*/

	std::cout << time(0) << std::endl;
}
catch(std::exception const& ex)
{
	std::cout << boost::diagnostic_information(ex) << std::endl;
}

void Engine::Simulation::loop()
{
	LuaTools::LuaEngine luaEngine;
	//lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, 20000);
	PlayerCodeMap codesMap;

	//luaL_openlibs(luaEngine.state());
	luaopen_base(luaEngine.state());
	//luaopen_package(luaEngine.state());
	//luaopen_string(luaEngine.state());
	//luaopen_table(luaEngine.state());
	//luaopen_math(luaEngine.state());
	//luaopen_io(luaEngine.state());
	//luaopen_os(luaEngine.state());
	//luaopen_debug(luaEngine.state());

	initDroneWars(luaEngine.state());

	{
		UniqueLock lock(univ_.mutex);

		//Chargement de tout les code flote/planet de tout les joueur(chargement dans python)
		BOOST_FOREACH(Universe::PlayerMap::value_type & playerNVP, univ_.playerMap)
		{
			Player& player = playerNVP.second;
			PlayerCodes newCodes =
			{
				registerCode(luaEngine, player.id, player.fleetsCode, univ_.time, true),
				registerCode(luaEngine, player.id, player.planetsCode, univ_.time, false)
			};
			codesMap.insert(make_pair(player.id, newCodes));
		}
	}


	time_t newUpdate = time(0);
	time_t newSave = newUpdate;
	newUpdate += RoundSecond;
	newSave += SaveSecond;

	size_t gcCounter = 0;

	while(false == boost::this_thread::interruption_requested())
	{
		time_t const now = time(0);

		if(newSave <= now)
		{
			std::cout << lexical_cast<std::string>(time(0)) + "_save.bta ";
			save(lexical_cast<std::string>(time(0)) + "_save.bta");
			std::cout << "OK" << std::endl;
			newSave += SaveSecond;
		}
		if(newUpdate <= now)
		{
			//std::cout << newUpdate << " " << now << std::endl;
			round(luaEngine, codesMap);
			newUpdate += RoundSecond;
			gcCounter += 1;
			if((gcCounter % 1) == 0)
			{
				std::cout << "GC : " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0);
				lua_gc(luaEngine.state(), LUA_GCCOLLECT, 0);
				std::cout << " -> " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0) << std::endl;
			}
		}
		else
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}



void Engine::Simulation::save(std::string const& saveName) const
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
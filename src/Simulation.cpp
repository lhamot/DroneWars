#include "stdafx.h"
#include "Simulation.h"

#include "LuaUniverse.h"
#include "Rules.h"
#include "Tools.h"
#include "LuaTools.h"
#include "fighting.h"
#pragma warning(push)
#pragma warning(disable: 4189 4100)
#include <luabind/stl_container_converter.hpp>
#pragma warning(pop)
#include <boost/range/numeric.hpp>
#include <boost/format.hpp>


extern "C"
{
#include "lua.h"
#include "lualib.h"
}


typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;
typedef boost::upgrade_lock<Universe::Mutex> UpgradeLock;
typedef boost::upgrade_to_unique_lock<Universe::Mutex> UpToUniqueLock;

using namespace LuaTools;
using namespace std;



static size_t const LuaMaxInstruction = 20000;
static size_t const MaxCodeExecTry = 10;

void luaCountHook(lua_State* L,
                  lua_Debug* //ar
                 )
{
	lua_sethook(L, luaCountHook, LUA_MASKCOUNT, 1);
	luaL_error(L, "timeout was reached");
}

static size_t const RoundSecond = 1;
static size_t const SaveSecond = 60;



Simulation::Simulation(Universe& univ):
	univ_(univ)
{
}

void Simulation::reloadPlayer(Player::ID pid)
{
	boost::unique_lock<Universe::Mutex> lock(mutex_);
	playerToReload_.insert(pid);
}


luabind::object registerCode(
  Universe& univ_,
  LuaTools::LuaEngine& luaEngine,
  Player::ID const pid,
  CodeData& code,
  time_t time,
  bool isFleet,
  std::vector<Signal>& signals)
try
{
	using namespace luabind;
	using namespace std;

	luaL_dostring(luaEngine.state(), "AI = nil");

	std::string const codeString = (isFleet ? "class 'AI' " : "") + code.getCode();
	if(luaL_dostring(luaEngine.state(), codeString.c_str()) != 0)
	{
		char const* message = lua_tostring(luaEngine.state(), -1);
		code.newError(message);
		Event event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, message ? message : "");
		mapFind(univ_.playerMap, pid)->second.eventList.push_back(event);
		signals.push_back(Signal(pid, event));
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
	Event event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, message);
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(event);
	signals.push_back(Signal(pid, event));
	return luabind::object();
}
catch(std::exception const& ex)
{
	char const* message = lua_tostring(luaEngine.state(), -1);
	Event event(univ_.nextEventID++, time, isFleet ? Event::FleetCodeError : Event::PlanetCodeError, ex.what() + string(" ") + (message ? message : ""));
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(event);
	signals.push_back(Signal(pid, event));
	return luabind::object();
}


void execPlanet(
  Universe& univ_,
  LuaEngine& luaEngine,
  luabind::object code,
  Planet& planet,
  time_t time,
  std::vector<Fleet const*> const& fleetList,
  std::vector<Signal>& signals)
try
{
	if(false == code.is_valid() || luabind::type(code) != LUA_TFUNCTION)
		return;
	lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);

	if(planet.buildingList.size() != Building::Count)
		BOOST_THROW_EXCEPTION(std::logic_error("planet.buildingList.size() != Building::Count"));

	PlanetAction action =	luabind::call_function<PlanetAction>(
	                        code, boost::cref(planet), boost::cref(fleetList));

	//for(PlanetAction const & action: list)
	{
		switch(action.action)
		{
		case PlanetAction::Undefined:
			break;
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
		case PlanetAction::Cannon:
		{
			if(canBuild(planet, action.cannon, action.number))
				addTask(planet, univ_.time, action.cannon, action.number);
		}
		break;
		default:
			BOOST_THROW_EXCEPTION(std::logic_error("Unknown PlanetAction::Type"));
		};
	}
	static_assert(PlanetAction::Count == 4, "Missing PlanetAction case");
}
catch(luabind::error& ex)
{
	std::string const message = GetLuabindErrorString(ex);
	Player& player = mapFind(univ_.playerMap, planet.playerId)->second;
	player.planetsCode.newError(message);//ex.what() + string(" ") + ss.str());
	Event event(univ_.nextEventID++, time, Event::PlanetCodeError, message);
	player.eventList.push_back(event);//ex.what() + string(" ") + ss.str()));
	signals.push_back(Signal(player.id, event));
}
catch(std::exception const& ex)
{
	Event event(univ_.nextEventID++, time, Event::FleetCodeError, ex.what());
	mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(event);
	signals.push_back(Signal(planet.playerId, event));
}

bool execFleet(Universe& univ_,
               LuaEngine& luaEngine,
               luabind::object code,
               Fleet& fleet,
               FleetCoordMap& fleetMap,
               time_t time,
               std::vector<Signal>& signals)
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
			//TODO: Décoreler script et traitement
			if(wantGather1 && wantGather2)
			{
				gather(fleet, otherFleet);
				auto condemned = fleetIter;
				++fleetIter;
				fleetMap.erase(condemned);
				Event event(univ_.nextEventID++, time, Event::FleetsGather);
				fleet.eventList.push_back(event);
				signals.push_back(Signal(fleet.playerId, event));

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
		action = luabind::call_member<FleetAction>(code, "action", boost::cref(fleet), (Planet*)nullptr);
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
		if(planet && canHarvest(fleet, *planet))
			addTaskHarvest(fleet, univ_.time, *planet);
		break;
	case FleetAction::Colonize:
		if(planet && canColonize(fleet, *planet))
			addTaskColonize(fleet, univ_.time, *planet);
		break;
	case FleetAction::Drop:
		if(planet && canDrop(fleet, *planet))
		{
			drop(fleet, *planet);
			Event event(univ_.nextEventID++, time, Event::FleetDrop);
			fleet.eventList.push_back(event);
			if(planet->playerId == Player::NoId)
				BOOST_THROW_EXCEPTION(std::logic_error("planet->playerId == Player::NoId"));
			planet->eventList.push_back(event);
			signals.push_back(Signal(planet->playerId, event));
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
	Event event(univ_.nextEventID++, time, Event::FleetCodeError, message);
	player.eventList.push_back(event);
	signals.push_back(Signal(player.id, event));
	return true;
}
catch(std::exception const& ex)
{
	std::string const message = typeid(ex).name() + string(" ") + ex.what();
	Player& player = mapFind(univ_.playerMap, fleet.playerId)->second;
	player.fleetsCode.newError(message);
	Event event(univ_.nextEventID++, time, Event::FleetCodeError, message);
	player.eventList.push_back(event);
	signals.push_back(Signal(player.id, event));
	return true;
}


//! Désactivation de tout les codes qui echoue
void disableFailingCode(Universe const& univ_, PlayerCodeMap& codesMap)
{
	for(Player const & player: univ_.playerMap | boost::adaptors::map_values)
	{
		if(player.planetsCode.getFailCount() >= MaxCodeExecTry)
			codesMap[player.id].planetsCode = luabind::object();
		if(player.fleetsCode.getFailCount() >= MaxCodeExecTry)
			codesMap[player.id].fleetsCode = luabind::object();
	}
}


//! Rechargement des codes flote/planet des joueurs dont le code a été changé
void Simulation::updatePlayersCode(LuaTools::LuaEngine& luaEngine,
                                   PlayerCodeMap& codesMap,
                                   std::vector<Signal>& signals)
{
	UniqueLock lockReload(mutex_);
	for(Player::ID pid: playerToReload_)
	{
		Player& player = mapFind(univ_.playerMap, pid)->second;
		PlayerCodes newCodes =
		{
			registerCode(univ_, luaEngine, player.id, player.fleetsCode, univ_.time, true, signals),
			registerCode(univ_, luaEngine, player.id, player.planetsCode, univ_.time, false, signals)
		};
		codesMap[player.id] = newCodes;
	}
	playerToReload_.clear();
}


void execPlanets(Universe& univ_,
                 UpgradeLock& lock,
                 LuaTools::LuaEngine& luaEngine,
                 PlayerCodeMap& codesMap,
                 std::vector<Signal>& signals)
{
	FleetCoordMap fleetMap;
	for(Fleet & fleet: univ_.fleetMap | boost::adaptors::map_values)
		fleetMap.insert(make_pair(fleet.coord, fleet));

	//Les planètes
	std::vector<Fleet const*> fleetList;
	for(Universe::PlanetMap::value_type & planetNVP: univ_.planetMap)
	{
		UpToUniqueLock writeLock(lock);

		Planet& planet = planetNVP.second;
		if(planet.eventList.size() > 10)
			planet.eventList.erase(planet.eventList.begin(), planet.eventList.end() - 10);

		fleetList.clear();
		auto localFleets = fleetMap.equal_range(planet.coord);
		auto getFleetPointer = [](FleetCoordMap::value_type const & coordFleet) {return &coordFleet.second;};
		boost::transform(localFleets, back_inserter(fleetList), getFleetPointer);
		//boost::copy(localFleets | boost::adaptors::map_values, back_inserter(fleetList));
		planetRound(univ_, planet, univ_.time, signals);
		if(planet.playerId != Player::NoId)
			execPlanet(univ_, luaEngine, codesMap[planet.playerId].planetsCode, planet, univ_.time, fleetList, signals);
	}
}

//! Les combats
void execFights(Universe& univ_, UpgradeLock& lock, std::vector<Signal>& signals)
{
	if(univ_.fleetMap.empty()) //si il y as des flottes
		return;

	vector<Fleet::ID> deadFleets;
	deadFleets.reserve(univ_.fleetMap.size());
	vector<Coord> lostPlanets;
	lostPlanets.reserve(univ_.planetMap.size());
	typedef std::multimap<Coord, FighterPtr, CompCoord> FleetCoordMultimap;
	FleetCoordMultimap fleetMultimap;

	//! Remplissage de la multimap de flote Coord=>flote
	for(Fleet & fleet: univ_.fleetMap | boost::adaptors::map_values)
		fleetMultimap.insert(make_pair(fleet.coord, FighterPtr(&fleet)));
	for(Planet & planet: univ_.planetMap | boost::adaptors::map_values)
		fleetMultimap.insert(make_pair(planet.coord, FighterPtr(&planet)));

	std::vector<Fleet*> fleetVect;
	//! Pour chaque coordonées, on accede au range des flotes
	for(FleetCoordMultimap::iterator iter1 = fleetMultimap.begin(), iter2 = nextNot(fleetMultimap, iter1);
	    iter1 != fleetMultimap.end();
	    iter1 = iter2, iter2 = nextNot(fleetMultimap, iter1))
	{
		auto fleetRange = make_pair(iter1, iter2);

		//! - Si 0 vaisseau, on passe
		auto testShipNumber = iter1;
		if(testShipNumber == iter2)
			continue;
		++testShipNumber;
		if(testShipNumber == iter2)
			continue;

		//! - On remplis le tableau de combatant a cette position
		fleetVect.clear();
		Planet* planetPtr = nullptr;
		for(FighterPtr const & fighterPtr: fleetRange | boost::adaptors::map_values)
		{
			if(fighterPtr.isPlanet())
				planetPtr = fighterPtr.getPlanet();
			else
				fleetVect.push_back(fighterPtr.getFleet());
		}

		UpToUniqueLock writeLock(lock);
		//! - On excecute le combats
		FightReport fightReport;
		fight(fleetVect, planetPtr, fightReport);
		bool hasFight = false;
		auto range = make_zip_range(fleetVect, fightReport.fleetList);
		for(auto fleetReportPair: range)
		{
			Report<Fleet> const& report = fleetReportPair.get<1>();
			if(report.hasFight)
			{
				hasFight = true;
				break;
			}
		}
		hasFight = hasFight | fightReport.planet.hasFight;

		//! - Si personne ne c'est batue, on passe
		if(hasFight == false)
			continue;

		//! - On ajoute le rapport dans la base de donné
		size_t const reportID = univ_.nextFightID;
		univ_.nextFightID += 1;
		univ_.reportMap.insert(make_pair(reportID, fightReport));

		//! - On ajoute les evenement/message dans les flottes/joueur
		for(auto fleetReportPair: range)
		{
			//! --Pour les flotes
			Fleet* fleetPtr = fleetReportPair.get<0>();
			Report<Fleet> const& report = fleetReportPair.get<1>();
			Fleet& fleet = *fleetPtr;
			if(report.isDead)
			{
				deadFleets.push_back(fleet.id);
				Event event(univ_.nextEventID++, univ_.time, Event::FleetLose, reportID);
				mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(event);
				signals.push_back(Signal(fleet.playerId, event));
			}
			else if(report.hasFight)
			{
				Event event(univ_.nextEventID++, univ_.time, Event::FleetWin, reportID);
				fleet.eventList.push_back(event);
				signals.push_back(Signal(fleet.playerId, event));
			}
		}
		if(planetPtr)
		{
			//! --Et pour la planete
			Planet& planet = *planetPtr;
			if(fightReport.planet.isDead)
			{
				lostPlanets.push_back(planet.coord);
				if(planet.playerId != Player::NoId)
				{
					Event event(univ_.nextEventID++, univ_.time, Event::PlanetLose, reportID);
					mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(event);
					signals.push_back(Signal(planet.playerId, event));
				}
			}
			else if(fightReport.planet.hasFight)
			{
				if(planet.playerId == Player::NoId)
					BOOST_THROW_EXCEPTION(std::logic_error("planet.playerId == Player::NoId"));
				Event event(univ_.nextEventID++, univ_.time, Event::PlanetWin, reportID);
				planet.eventList.push_back(event);
				signals.push_back(Signal(planet.playerId, event));
			}
		}
	}

	UpToUniqueLock writeLock(lock);
	//! On gère chaque planete perdues(onPlanetLose)
	for(Coord planetCoord: lostPlanets)
		onPlanetLose(planetCoord, univ_);

	//! Suppression de toute les flottes mortes
	for(Fleet::ID fleetID: deadFleets)
		univ_.fleetMap.erase(fleetID);
}

//! Excecutes les code des flottes
void execFleets(
  Universe& univ_,
  UpgradeLock& lock,
  LuaTools::LuaEngine& luaEngine,
  PlayerCodeMap& codesMap,
  std::vector<Signal>& signals)
{
	FleetCoordMap fleetMap;
	for(Fleet & fleet: univ_.fleetMap | boost::adaptors::map_values)
		fleetMap.insert(make_pair(fleet.coord, fleet));

	auto iter = fleetMap.begin();
	while(iter != fleetMap.end())
	{
		UpToUniqueLock writeLock(lock);

		fleetRound(univ_, iter->second, univ_.time, signals);

		bool keepFleet = execFleet(univ_,
		                           luaEngine,
		                           codesMap[iter->second.playerId].fleetsCode,
		                           iter->second,
		                           fleetMap,
		                           univ_.time,
		                           signals);
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
	for(Fleet & fleet: fleetMap | boost::adaptors::map_values)
	{
		if(boost::accumulate(fleet.shipList, 0) > 0) //Si flotte vide, on garde pas
			newFleetMap.insert(make_pair(fleet.id, fleet));
	}
	UpToUniqueLock writeLock(lock);
	newFleetMap.swap(univ_.fleetMap);

	for(Fleet & fleet: univ_.fleetMap | boost::adaptors::map_values)
	{
		if(fleet.eventList.size() > 10)
			fleet.eventList.erase(fleet.eventList.begin(), fleet.eventList.end() - 10);
	}
}


//! Supprime les evenement trop vieux, et les rapport plus réfférencés
void removeOldEvents(Universe& univ_)
{
	set<size_t> usedReport;
	for(Player & player: univ_.playerMap | boost::adaptors::map_values)
	{
		if(player.eventList.size() > 10)
			player.eventList.erase(player.eventList.begin(), player.eventList.end() - 10);
		for(Event const & ev: player.eventList)
		{
			if(ev.type == Event::FleetLose || ev.type == Event::PlanetLose)
				usedReport.insert(ev.value);
		}
	}

	map_remove_erase_if(univ_.reportMap,
	                    [&](Universe::ReportMap::value_type const & reportKV)
	{
		return usedReport.count(reportKV.first) == false;
	});
}

void Simulation::round(LuaTools::LuaEngine& luaEngine,
                       PlayerCodeMap& codesMap,
                       std::vector<Signal>& signals)
try
{
	std::cout << time(0) << " ";

	univ_.time += RoundSecond;

	UpgradeLock lock(univ_.mutex);

	//! Désactivation de tout les codes qui echoue
	disableFailingCode(univ_, codesMap);

	//! Rechargement des codes flote/planet des joueurs dont le code a été changé
	{
		UpToUniqueLock writeLock(lock);
		updatePlayersCode(luaEngine, codesMap, signals);
	}

	//! Excecution du code des planetes(modifie l'univers)
	execPlanets(univ_, lock, luaEngine, codesMap, signals);

	//! Les combats
	execFights(univ_, lock, signals);

	//! Les flottes
	execFleets(univ_, lock, luaEngine, codesMap, signals);

	//! Supprime les flotte mortes
	{
		UpToUniqueLock writeLock(lock);
		removeOldEvents(univ_);
	}

	//! CheckTutos
	checkTutos(univ_, lock, signals);


	//std::cout << lexical_cast<std::string>(time(0)) + "_save.bta ";
	//save(lexical_cast<std::string>(time(0)) + "_save.bta");

	/*
	//Calcule de l'occupation memoire
	size_t playerSize = 0;
	for(auto const& playerKV: univ_.playerMap)
	{
		playerSize += sizeof(playerKV);
		playerSize += playerKV.second.fleetsCode.getCode().size();
		playerSize += playerKV.second.planetsCode.getCode().size();
		playerSize += playerKV.second.login.size();
		playerSize += playerKV.second.password.size();
		for(auto const& ev: playerKV.second.eventList)
		{
			playerSize += sizeof(ev);
			playerSize += ev.comment.size();
		}
	}

	size_t planetSize = 0;
	for(auto const& planetKV: univ_.planetMap)
	{
		planetSize += sizeof(planetKV);
		planetSize += planetKV.second.buildingMap.size() * sizeof(Planet::BuildingMap::value_type);
		planetSize += planetKV.second.taskQueue.size() * sizeof(PlanetTask);
		for(auto const& ev: planetKV.second.eventList)
		{
			planetSize += sizeof(ev);
			planetSize += ev.comment.size();
		}
	}

	size_t fleetSize = 0;
	for(auto const& fleetKV: univ_.fleetMap)
	{
		fleetSize += sizeof(fleetKV);
		fleetSize += fleetKV.second.shipList.size() * sizeof(Fleet::ShipTab::value_type);
		fleetSize += fleetKV.second.taskQueue.size() * sizeof(PlanetTask);
		for(auto const& ev: fleetKV.second.eventList)
		{
			fleetSize += sizeof(ev);
			fleetSize += ev.comment.size();
		}
	}

	size_t reportSize = 0;
	for(auto const& reportKV: univ_.reportMap)
	{
		reportSize += sizeof(reportKV);
		for(auto const& fleetReport: reportKV.second)
		{
			reportSize += sizeof(fleetReport);
			reportSize += fleetReport.enemySet.size() * sizeof(size_t);
			for(auto const& ev: fleetReport.fleetsAfter.eventList)
			{
				reportSize += sizeof(ev);
				reportSize += ev.comment.size();
			}
			for(auto const& ev: fleetReport.fleetsBefore.eventList)
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

static const luaL_Reg loadedlibs[] =
{
	{"_G", luaopen_base},
	//{LUA_LOADLIBNAME, luaopen_package},
	//{LUA_COLIBNAME, luaopen_coroutine},
	//{LUA_TABLIBNAME, luaopen_table},
	//{LUA_IOLIBNAME, luaopen_io},
	//{LUA_OSLIBNAME, luaopen_os},
	//{LUA_STRLIBNAME, luaopen_string},
	//{LUA_BITLIBNAME, luaopen_bit32},
	{LUA_MATHLIBNAME, luaopen_math},
	//{LUA_DBLIBNAME, luaopen_debug},
	{NULL, NULL}
};


static const luaL_Reg preloadedlibs[] =
{
	{NULL, NULL}
};


void openlibs(lua_State* L)
{
	const luaL_Reg* lib;
	//call open functions from 'loadedlibs' and set results to global table
	for(lib = loadedlibs; lib->func; lib++)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  //remove lib
	}
	//add open functions from 'preloadedlibs' into 'package.preload' table
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	for(lib = preloadedlibs; lib->func; lib++)
	{
		lua_pushcfunction(L, lib->func);
		lua_setfield(L, -2, lib->name);
	}
	lua_pop(L, 1);  //remove _PRELOAD table
}

void Simulation::loop()
try
{
	LuaTools::LuaEngine luaEngine;
	//lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, 20000);
	PlayerCodeMap codesMap;

	openlibs(luaEngine.state());
	//luaL_openlibs(luaEngine.state());
	//luaopen_base(luaEngine.state());
	//luaopen_package(luaEngine.state());
	//luaopen_string(luaEngine.state());
	//luaopen_table(luaEngine.state());
	//luaopen_math(luaEngine.state());
	//luaopen_io(luaEngine.state());
	//luaopen_os(luaEngine.state());
	//luaopen_debug(luaEngine.state());

	initDroneWars(luaEngine.state());

	std::vector<Signal> signals;

	{
		UniqueLock lock(univ_.mutex);

		//Chargement de tout les code flote/planet de tout les joueur(chargement dans python)
		for(Universe::PlayerMap::value_type & playerNVP: univ_.playerMap)
		{
			Player& player = playerNVP.second;
			PlayerCodes newCodes =
			{
				registerCode(univ_, luaEngine, player.id, player.fleetsCode, univ_.time, true, signals),
				registerCode(univ_, luaEngine, player.id, player.planetsCode, univ_.time, false, signals)
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
			std::string const filename = (boost::format("save/%1%_save.bta") % time(0)).str();
			save(filename);
			removeOldSaves();
			std::cout << "OK" << std::endl;
			newSave += SaveSecond;
		}
		if(newUpdate <= now)
			try
			{
				//std::cout << newUpdate << " " << now << std::endl;
				round(luaEngine, codesMap, signals);
				signals.clear();
				newUpdate += RoundSecond;
				gcCounter += 1;
				if((gcCounter % 1) == 0)
				{
					std::cout << "GC : " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0);
					lua_gc(luaEngine.state(), LUA_GCCOLLECT, 0);
					std::cout << " -> " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0) << std::endl;
				}
			}
			catch(std::exception const& ex)
			{
				std::cerr << boost::diagnostic_information(ex) << std::endl;
				throw;
			}
			catch(...)
			{
				//std::cerr << "Not standard exception" << std::endl;
				std::cerr << boost::current_exception_diagnostic_information() << std::endl;
				throw;
			}
		else
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}
catch(std::exception const& ex)
{
	std::cerr << boost::diagnostic_information(ex) << std::endl;
	throw;
}



void Simulation::save(std::string const& saveName) const
{
	auto savingFunc = [](Universe const & clone, std::string const & saveName)
	{
		try
		{
			using namespace std;
			std::string const newSaveName = saveName + ".new";
			{
				ofstream saveFile(newSaveName.c_str(), ios::out | ios::binary);
				if(saveFile.is_open() == false)
					BOOST_THROW_EXCEPTION(std::ios::failure("Can't save in " + newSaveName));
				saveToStream(clone, saveFile);
			}
			std::string const ansSaveName = saveName + ".ans";
			remove(ansSaveName.c_str());
			struct stat buf;
			if(stat(saveName.c_str(), &buf) == 0)
				rename(saveName.c_str(), ansSaveName.c_str());
			rename(newSaveName.c_str(), saveName.c_str());
		}
		catch(std::exception const& ex)
		{
			std::cerr << boost::diagnostic_information(ex) << std::endl;
			throw;
		}
	};

	SharedLock lock(univ_.mutex);
	boost::thread savingThread(savingFunc, univ_, saveName);
}


void Simulation::removeOldSaves() const
{
	using namespace boost::filesystem;
	typedef boost::filesystem::directory_iterator DirIter;

	std::set<time_t> timeSet;

	DirIter beginFileIter("save/"), endFileIter;
	for(auto path: boost::make_iterator_range(beginFileIter, endFileIter))
	{
		std::string const filename = path.path().filename().string();
		if(filename.find("_save.bta") != 10 || filename.size() != 19) //Ce n'est pas une archive normal
			continue;

		std::string const timeStr = path.path().stem().string().substr(0, 10);
		time_t const timeValue = boost::lexical_cast<time_t>(timeStr);
		timeSet.insert(timeValue);
	}

	if(timeSet.size() < 2)
		return;

	time_t endTime = *boost::prior(timeSet.end());
	time_t beginTime = endTime;
	time_t timeLaps = 30;

	//On va supprimer tout les fichier en trop dans laps de temps
	//La taille du laps augmente au fure et a mesure qu'on recule dans le temps
	std::set<time_t>::const_iterator begin;
	std::set<time_t>::const_iterator end;
	do
	{
		timeLaps = (time_t)(timeLaps * 1.1);
		//timeLaps /= 10;
		endTime = beginTime;

		beginTime = endTime - timeLaps;
		//cout << beginTime << " " << endTime << endl;
		begin = timeSet.lower_bound(beginTime);
		end = timeSet.lower_bound(endTime);

		size_t fileCount = 0;
		for(time_t fileTime: boost::make_iterator_range(begin, end))
		{
			++fileCount;
			if(fileCount == 1)
				continue;
			boost::filesystem::path const filename = (boost::format("save/%1%_save.bta") % fileTime).str();
			//cout << filename << endl;
			remove(filename);
		}
	}
	while(end != timeSet.begin());
}
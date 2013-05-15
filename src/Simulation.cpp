//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Simulation.h"

#include "LuaUniverse.h"
#include "Rules.h"
#include "Tools.h"
#include "LuaTools.h"
#include "fighting.h"
#include "Logger.h"
#include "DataBase.h"
#include <luabind/stl_container_converter.hpp>
#include <boost/range/numeric.hpp>
#include <boost/format.hpp>
#include <boost/chrono.hpp>


extern "C"
{
#include "lua.h"
#include "lualib.h"
}


//! Verrou en écriture
typedef boost::unique_lock<Universe::Mutex> UniqueLock;
//! Verrou en lecture
typedef boost::shared_lock<Universe::Mutex> SharedLock;
//! Verrou en lecture, mutable en écriture
typedef boost::upgrade_lock<Universe::Mutex> UpgradeLock;
//! Verrou en écriture construit a partir d'un UpgradeLock
typedef boost::upgrade_to_unique_lock<Universe::Mutex> UpToUniqueLock;

using namespace LuaTools;
using namespace std;
namespace BL = boost::locale;

using namespace log4cplus;
//! Logger du thread de simulation
static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("Simulation"));


//! Nombre d'instruction max authorisé dans l'éxcecution d'un script lua
static size_t const LuaMaxInstruction = 20000;


//! callback appelé par lua quand le nombre d'instruction max est dépassé
void luaCountHook(lua_State* L,
                  lua_Debug* //ar
                 )
{
	lua_sethook(L, luaCountHook, LUA_MASKCOUNT, 1);
	luaL_error(L, "timeout was reached");
}


static size_t const RoundSecond = 10; //!< Nombre de secondes min par round
static size_t const SaveSecond = 60;  //!< Nombre de secondes min entre 2 save


Simulation::Simulation(Universe& univ):
	univ_(univ)
{
}

void Simulation::reloadPlayer(Player::ID pid)
{
	boost::unique_lock<Universe::Mutex> lock(reloadPlayerMutex_);
	playerToReload_.insert(pid);
}


//! @brief Traite une erreur de scripte en ajoutant un Event
Event makeCodeErrorEvent(Player::ID pid,
                         CodeData::Target target,
                         size_t codeID,
                         std::string const& message)
{
	return Event(pid,
	             time(0),
	             target == CodeData::Fleet ?
	             Event::FleetCodeError :
	             Event::PlanetCodeError)
	       .setComment(message)
	       .setValue(codeID);
}


//! @brief Traite une erreur de scripte en ajoutant un Event
void addErrorMessage(CodeData const& codeData,
                     std::string const& message,
                     std::vector<Event>& events)
{
	events.push_back(
	  makeCodeErrorEvent(
	    codeData.playerId, codeData.target, codeData.id, message));
}


//! @brief Traite une erreur de scripte en ajoutant un Event
void addErrorMessage(PlayerCodes::ObjectMap& objMap,
                     std::string const& message,
                     std::vector<Event>& events)
{
	events.push_back(
	  makeCodeErrorEvent(
	    objMap.playerId, objMap.target, objMap.scriptID, message));
	objMap.functions.clear();
}


//! Fait interpreter un script à lua et le transforme en PlayerCodes::ObjectMap
PlayerCodes::ObjectMap registerCode(
  LuaTools::LuaEngine& luaEngine,
  CodeData const& code,
  std::vector<Event>& events)
try
{
	using namespace luabind;
	using namespace std;

	luaL_dostring(luaEngine.state(), "AI = nil");
	luaL_dostring(luaEngine.state(), "AI_action = nil");
	luaL_dostring(luaEngine.state(), "AI_do_gather = nil");
	luaL_dostring(luaEngine.state(), "AI_do_fight = nil");

	std::string codeString = code.code;

	auto replace = [&](char const * const str)
	{
		size_t pos = codeString.find(str);
		if(pos != std::string::npos)
			codeString[pos + 3] = '_';
	};
	replace(" AI:action(");
	replace(" AI:do_gather(");
	replace(" AI:do_fight(");

	if(luaL_dostring(luaEngine.state(), codeString.c_str()) != 0)
	{
		char const* message = lua_tostring(luaEngine.state(), -1);
		addErrorMessage(code, message, events);
		return PlayerCodes::ObjectMap(); //Vide car toute les fonction sont invalides
	}
	else
	{
		luabind::object g = luabind::globals(luaEngine.state());
		PlayerCodes::ObjectMap result;
		result.playerId = code.playerId;
		result.scriptID = code.id;
		result.target = code.target;
		result.functions["AI"] = g["AI"];
		result.functions["action"] = g["AI_action"];
		result.functions["do_gather"] = g["AI_do_gather"];
		result.functions["do_fight"] = g["AI_do_fight"];
		return result;
	}
}
catch(luabind::error& ex)
{
	std::string message = GetLuabindErrorString(ex);
	addErrorMessage(code, message, events);
	return PlayerCodes::ObjectMap(); //Vide car toute les fonction sont invalides
}
catch(luabind::cast_failed& ex)
{
	std::string message = ex.what() + std::string(" to ") + ex.info().name();
	addErrorMessage(code, message, events);
	return PlayerCodes::ObjectMap(); //Vide car toute les fonction sont invalides
}
//Les logic_error ne doivent pas etre catchées
/*catch(std::exception const& ex)
{
	char const* message = lua_tostring(luaEngine.state(), -1);
	Event event(univ_.nextEventID++, time(0), isFleet ? Event::FleetCodeError : Event::PlanetCodeError, ex.what() + string(" ") + (message ? message : ""));
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(event);
	signals.push_back(Signal(pid, event));
	return luabind::object();
}*/


//! Excecute le code de la planete et lui ajoute des tache
void execPlanet(
  Universe& univ_,
  LuaEngine& luaEngine,
  PlayerCodes::ObjectMap& codeMap,
  Planet& planet,
  std::vector<Fleet const*> const& fleetList,
  std::vector<Event>& events)
try
{
	auto codeIter = codeMap.functions.find("AI");
	if(codeIter == codeMap.functions.end())
		return; //Le code a été invalidé
	luabind::object code = codeIter->second;
	if(code.is_valid() == false || luabind::type(code) != LUA_TFUNCTION)
	{
		//call_function crash quand on lui passe autre chose qu'une fonction
		boost::format trans(BL::gettext("Procedure \"%1%\" not found."));
		std::string const message = str(trans % "AI");
		addErrorMessage(codeMap, message, events);
		return;
	}

	lua_sethook(
	  luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);

	if(planet.buildingList.size() != Building::Count)
		BOOST_THROW_EXCEPTION(
		  std::logic_error("planet.buildingList.size() != Building::Count"));

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
				addTask(planet, univ_.roundCount, action.building);
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
				addTask(planet, univ_.roundCount, action.ship, action.number);
		}
		break;
		case PlanetAction::Cannon:
		{
			if(canBuild(planet, action.cannon, action.number))
				addTask(planet, univ_.roundCount, action.cannon, action.number);
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
	addErrorMessage(codeMap, message, events);
}
catch(luabind::cast_failed& ex)
{
	std::string const message =
	  str(boost::format(BL::gettext("Unable to make cast to %1%")) % ex.info().name());
	addErrorMessage(codeMap, message, events);
}
//Les logic_error ne doivent pas etre catchées
/*catch(std::exception const& ex)
{
	UniqueLock lockPlayer(univ_.playersMutex);
	Event event(univ_.nextEventID++, time(0), Event::PlanetCodeError, ex.what());
	mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(event);
	signals.push_back(Signal(planet.playerId, event));
}*/


//! @brief Verifie si une methode lua existe dans la codeMap et est valide
//! @remark Si invalide, ajoute un Event avec addErrorMessage
bool checkLuaMethode(PlayerCodes::ObjectMap& codeMap,
                     std::string const& name,
                     std::vector<Event>& events)
{
	auto codeIter = codeMap.functions.find(name);
	if(codeIter == codeMap.functions.end())
		return false; //Le code à été invalidé
	else
	{
		luabind::object methode = codeIter->second;
		if(luabind::type(methode) == LUA_TFUNCTION)
		{
			if(codeMap.playerId == Player::NoId)
				BOOST_THROW_EXCEPTION(std::logic_error("codeMap.playerId == Player::NoId"));
			return true;
		}
		else
		{
			std::string const message =
			  str(boost::format(BL::gettext("Procedure \"%1%\" not found.")) % name);
			addErrorMessage(codeMap, message, events);
			return false;
		}
	}
}

bool execFleet(Universe& univ_,
               LuaEngine& luaEngine,
               PlayerCodes::ObjectMap& codeMap,
               Fleet& fleet,
               FleetCoordMap& fleetMap,
               std::vector<Event>& events)
try
{
	auto localFleetsKV = fleetMap.equal_range(fleet.coord);
	auto fleetIter = localFleetsKV.first;
	if(checkLuaMethode(codeMap, "do_gather", events))
	{
		luabind::object do_gather = mapFind(codeMap.functions, "do_gather")->second;
		while(fleetIter != localFleetsKV.second)
		{
			Fleet& otherFleet = fleetIter->second;
			if((otherFleet.id > fleet.id) && (otherFleet.playerId == fleet.playerId))
			{
				lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
				bool const wantGather1 = luabind::call_function<bool>(do_gather, boost::cref(fleet), boost::cref(otherFleet));
				lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
				bool const wantGather2 = luabind::call_function<bool>(do_gather, boost::cref(otherFleet), boost::cref(fleet));
				//! @todo: Décoreler script et traitement
				if(wantGather1 && wantGather2)
				{
					gather(fleet, otherFleet);
					auto condemned = fleetIter;
					++fleetIter;
					fleetMap.erase(condemned);
					Event event(fleet.playerId, time(0), Event::FleetsGather);
					event.setFleetID(fleet.id);
					events.push_back(event);

					continue;
				}
			}
			++fleetIter;
		}
	}

	if(checkLuaMethode(codeMap, "action", events) == false)
		return true;
	luabind::object actionFunc = mapFind(codeMap.functions, "action")->second;
	auto planetIter = univ_.planetMap.find(fleet.coord);
	Planet* planet = nullptr;
	if(planetIter != univ_.planetMap.end())
		planet = &(planetIter->second);
	using namespace luabind;
	FleetAction action(FleetAction::Nothing);
	lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, LuaMaxInstruction);
	if(planet && fleetCanSeePlanet(fleet, *planet, univ_) == false)
		planet = nullptr;
	if(planet)
	{
		auto result = luabind::call_function<FleetAction>(actionFunc, boost::cref(fleet), boost::cref(*planet));
		action = result;
	}
	else
		action = luabind::call_function<FleetAction>(actionFunc, boost::cref(fleet), (Planet*)nullptr);
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
			addTaskMove(fleet, univ_.roundCount, target);
	}
	break;
	case FleetAction::Harvest:
		if(planet && canHarvest(fleet, *planet))
			addTaskHarvest(fleet, univ_.roundCount, *planet);
		break;
	case FleetAction::Colonize:
		if(planet && canColonize(fleet, *planet))
			addTaskColonize(fleet, univ_.roundCount, *planet);
		break;
	case FleetAction::Drop:
		if(planet && canDrop(fleet, *planet))
		{
			drop(fleet, *planet);
			Event event(planet->playerId, time(0), Event::FleetDrop);
			event.setPlanetCoord(planet->coord).setFleetID(fleet.id);
			events.push_back(event);
			if(planet->playerId == Player::NoId)
				BOOST_THROW_EXCEPTION(std::logic_error("planet->playerId == Player::NoId"));
			events.push_back(event);
		}
		break;
	}

	return true;
}
catch(luabind::error& ex)
{
	std::string const message = GetLuabindErrorString(ex);
	addErrorMessage(codeMap, message, events);
	return true;
}
catch(luabind::cast_failed& ex)
{
	std::string const message =
	  str(boost::format(BL::gettext("Unable to make cast to %1%")) % ex.info().name());
	addErrorMessage(codeMap, message, events);
	return true;
}
//Les logic_error ne doivent pas etre catchées
/*catch(std::exception const& ex)
{
	std::string const message = typeid(ex).name() + string(" ") + ex.what();
	addErrorMessage(univ_, fleet.playerId, true, message, signals, codeMap);
	return true;
}*/


//! Rechargement des codes flote/planet des joueurs dont le code a été changé,
//! Modifie la codesMap et playerToReload_
void Simulation::updatePlayersCode(LuaTools::LuaEngine& luaEngine,
                                   PlayerCodeMap& codesMap,
                                   std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	UniqueLock lockReload(reloadPlayerMutex_);
	for(Player::ID pid : playerToReload_)
	{
		CodeData const fleetsCode = database_.getPlayerCode(pid, CodeData::Fleet);
		CodeData const planetsCode = database_.getPlayerCode(pid, CodeData::Planet);
		PlayerCodes newCodes =
		{
			registerCode(luaEngine, fleetsCode, events),
			registerCode(luaEngine, planetsCode, events)
		};
		codesMap[pid] = newCodes;
	}
	playerToReload_.clear();
	LOG4CPLUS_TRACE(logger, "exit");
}


//! Simule le round pour toute les planètes
void execPlanets(Universe& univ_,
                 LuaTools::LuaEngine& luaEngine,
                 PlayerCodeMap& codesMap,
                 std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	UpgradeLock lockPlanet(univ_.planetsFleetsReportsmutex);
	FleetCoordMap fleetMap;
	for(Fleet const & fleet : univ_.fleetMap | boost::adaptors::map_values)
		fleetMap.insert(make_pair(fleet.coord, fleet));

	//Les planètes
	std::vector<Fleet const*> fleetList;
	for(Universe::PlanetMap::value_type & planetNVP : univ_.planetMap)
	{
		lockPlanet.unlock();//On peut deverouiller temporairement car on sait que
		lockPlanet.lock();  //   l'autre thread ne fera que lire les planètes(pas d'ajout)

		UpToUniqueLock writeLock(lockPlanet);

		Planet& planet = planetNVP.second;
		//if(planet.eventList.size() > 10)
		//	planet.eventList.erase(planet.eventList.begin(), planet.eventList.end() - 10);
		fleetList.clear();
		auto localFleets = fleetMap.equal_range(planet.coord);
		auto getFleetPointer = [](FleetCoordMap::value_type const & coordFleet) {return &coordFleet.second;};
		boost::transform(localFleets, back_inserter(fleetList), getFleetPointer);
		planetRound(univ_, planet, events);
		if(planet.playerId != Player::NoId)
			execPlanet(univ_, luaEngine, codesMap[planet.playerId].planetsCode, planet, fleetList, events);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}

//! Les combats
void execFights(Universe& univ_,
                DataBase& database,
                PlayerCodeMap& codesMap,
                std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	UpgradeLock lockFleets(univ_.planetsFleetsReportsmutex);
	if(univ_.fleetMap.empty()) //si il y as des flottes
		return;

	std::map<Player::ID, uint32_t> experienceMap;

	std::vector<Player> players = database.getPlayers();
	std::map<Player::ID, Player> const playerMap = [&]()
	{
		std::map<Player::ID, Player> playerMap;
		for(Player const & player : players)
			playerMap.insert(make_pair(player.id, player));
		return playerMap;
	}();

	vector<Fleet::ID> deadFleets;
	deadFleets.reserve(univ_.fleetMap.size());
	vector<Coord> lostPlanets;
	lostPlanets.reserve(univ_.planetMap.size());
	typedef std::multimap<Coord, FighterPtr, CompCoord> FleetCoordMultimap;
	static FleetCoordMultimap fleetMultimap;

	if(fleetMultimap.empty())
		for(Planet & planet : univ_.planetMap | boost::adaptors::map_values)
			fleetMultimap.insert(make_pair(planet.coord, FighterPtr(&planet)));
	else
	{
		map_remove_erase_if(fleetMultimap, []
		                    (FleetCoordMultimap::value_type const & nvp)
		{
			return nvp.second.isPlanet() == false;
		});
	}

	//! Remplissage de la multimap de flote Coord=>flote
	for(Fleet & fleet : univ_.fleetMap | boost::adaptors::map_values)
		fleetMultimap.insert(make_pair(fleet.coord, FighterPtr(&fleet)));

	std::vector<Fleet*> fleetVect;
	//! Pour chaque coordonées, on accede au range des flotes
	auto fleetMultimapEnd = fleetMultimap.end();
	for(FleetCoordMultimap::iterator iter1 = fleetMultimap.begin(), iter2 = nextNot(fleetMultimap, iter1);
	    iter1 != fleetMultimapEnd;
	    iter1 = iter2, iter2 = nextNot(fleetMultimap, iter1))
	{
		lockFleets.unlock(); //On peut deverouiller temporairement car on sait que
		lockFleets.lock();   //   l'autre thread ne fera que lire les flottes(pas d'ajout)

		//! - Si 0 ou 1 combatant, on passe
		{
			auto testShipNumber = iter1;
			if(testShipNumber == iter2)
				continue;
			++testShipNumber;
			if(testShipNumber == iter2)
				continue;
		}

		//! - On remplis le tableau de combatant a cette position
		auto fleetRange = make_pair(iter1, iter2);
		fleetVect.clear();
		Planet* planetPtr = nullptr;
		for(FighterPtr const & fighterPtr : fleetRange | boost::adaptors::map_values)
		{
			if(fighterPtr.isPlanet())
				planetPtr = fighterPtr.getPlanet();
			else
				fleetVect.push_back(fighterPtr.getFleet());
		}
		//if(planetPtr && planetPtr->playerId != Player::NoId)
		//	planetPtr = nullptr;

		UpToUniqueLock writeLock(lockFleets);
		//! - On excecute le combats
		FightReport fightReport;
		fight(fleetVect, planetPtr, codesMap, fightReport);
		calcExperience(playerMap, fightReport);
		bool hasFight = false;
		auto range = make_zip_range(fleetVect, fightReport.fleetList);
		for(auto fleetReportPair : range)
		{
			Report<Fleet> const& report = fleetReportPair.get<1>();
			if(report.hasFight)
			{
				hasFight = true;
				break;
			}
		}
		if(fightReport.hasPlanet)
			hasFight = hasFight || fightReport.planet.get().hasFight;

		//! - Si personne ne c'est batue, on passe
		if(hasFight == false)
			continue;

		//! - On ajoute le rapport dans la base de donné
		size_t const reportID = database.addFightReport(fightReport);

		//! - On ajoute les evenement/message dans les flottes/joueur
		std::set<Player::ID> informedPlayer;
		for(auto fleetReportPair : range)
		{
			//! --Pour les flotes
			Fleet* fleetPtr = fleetReportPair.get<0>();
			Report<Fleet> const& report = fleetReportPair.get<1>();
			Fleet const& fleet = *fleetPtr;
			if(report.isDead)
			{
				deadFleets.push_back(fleet.id);
				Event event(fleet.playerId, time(0), Event::FleetLose);
				event.setValue(intptr_t(reportID));
				if(informedPlayer.count(fleet.playerId) == 0)
				{
					events.push_back(event);
					informedPlayer.insert(fleet.playerId);
				}
			}
			else if(report.hasFight)
			{
				Event event(fleet.playerId, time(0), Event::FleetWin);
				event.setValue(intptr_t(reportID)).setFleetID(fleet.id);
				events.push_back(event);
			}
		}
		if(planetPtr)
		{
			//! --Et pour la planete
			Planet& planet = *planetPtr;
			if(fightReport.planet.get().isDead)
			{
				lostPlanets.push_back(planet.coord);
				if(planet.playerId != Player::NoId)
				{
					Event event(planet.playerId, time(0), Event::PlanetLose);
					event.setValue(intptr_t(reportID));
					events.push_back(event);
					informedPlayer.insert(planet.playerId);
				}
			}
			else if(fightReport.planet.get().hasFight)
			{
				if(planet.playerId == Player::NoId)
					BOOST_THROW_EXCEPTION(
					  std::logic_error("planet.playerId == Player::NoId"));
				Event event(planet.playerId, time(0), Event::PlanetWin);
				event
				.setValue(intptr_t(reportID))
				.setPlanetCoord(planet.coord);
				events.push_back(event);
			}
		}

		//! - On cumul l'experience gagné par les joueurs
		for(Report<Fleet> const & fleetReport : fightReport.fleetList)
			experienceMap[fleetReport.fightInfo.before.playerId] +=
			  fleetReport.experience;
		if(fightReport.planet)
		{
			Report<Planet> const& report = *fightReport.planet;
			experienceMap[report.fightInfo.before.playerId] +=
			  report.experience;
		}
	}

	//! On envoie dans la base les gain d'eperience
	database.updateXP(experienceMap);

	UpToUniqueLock writeLock(lockFleets);
	//! On gère chaque planete perdues(onPlanetLose)
	std::unordered_map<Coord, Coord> newParentMap;
	for(Coord planetCoord : lostPlanets)
		onPlanetLose(planetCoord, univ_, playerMap, newParentMap);

	//! Les planètes et flottes orpheline sont réasigné a leurs grand-parents
	auto newParentMapEnd = newParentMap.end();
	for(Fleet & fleet : univ_.fleetMap | boost::adaptors::map_values)
	{
		while(true)
		{
			auto iter = newParentMap.find(fleet.origin);
			if(iter != newParentMapEnd)
				fleet.origin = iter->second;
			else
				break;
		}
	}
	for(Planet & planet : univ_.planetMap | boost::adaptors::map_values)
	{
		if(planet.playerId == Player::NoId)
			continue;
		while(true)
		{
			auto iter = newParentMap.find(planet.parentCoord);
			if(iter != newParentMapEnd)
				planet.parentCoord = iter->second;
			else
				break;
		}
	}

	//! Suppression de toute les flottes mortes
	for(Fleet::ID fleetID : deadFleets)
		univ_.fleetMap.erase(fleetID);

	LOG4CPLUS_TRACE(logger, "exit");
}

//! Excecutes les code des flottes
void execFleets(
  Universe& univ_,
  LuaTools::LuaEngine& luaEngine,
  PlayerCodeMap& codesMap,
  std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	UpgradeLock lockFleets(univ_.planetsFleetsReportsmutex);

	FleetCoordMap fleetMap;
	for(Fleet & fleet : univ_.fleetMap | boost::adaptors::map_values)
		fleetMap.insert(make_pair(fleet.coord, fleet));

	std::map<Player::ID, size_t> playersPlanetCount;
	for(Planet const & planet : univ_.planetMap | boost::adaptors::map_values)
		++playersPlanetCount[planet.playerId];

	auto iter = fleetMap.begin();
	while(iter != fleetMap.end())
	{
		lockFleets.unlock(); //On peut deverouiller temporairement car on sait que
		lockFleets.lock();   //   l'autre thread ne fera que lire les flottes(pas d'ajout)

		UpToUniqueLock writeLock(lockFleets);

		fleetRound(univ_, iter->second, events, playersPlanetCount);

		bool keepFleet = execFleet(univ_,
		                           luaEngine,
		                           codesMap[iter->second.playerId].fleetsCode,
		                           iter->second,
		                           fleetMap,
		                           events);
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
	for(Fleet & fleet : fleetMap | boost::adaptors::map_values)
	{
		if(boost::accumulate(fleet.shipList, 0) > 0) //Si flotte vide, on garde pas
			newFleetMap.insert(make_pair(fleet.id, fleet));
	}
	UpToUniqueLock writeLock(lockFleets);
	newFleetMap.swap(univ_.fleetMap);

	LOG4CPLUS_TRACE(logger, "exit");
}


void Simulation::round(LuaTools::LuaEngine& luaEngine,
                       PlayerCodeMap& codesMap,
                       std::vector<Event>& events)
try
{
	//! @todo: Ne plus passer events en argument
	LOG4CPLUS_TRACE(logger, "enter");

	std::cout << time(0) << " ";

	univ_.roundCount += 1; //1 round

	//! Désactivation de tout les codes qui echoue
	//disableFailingCode(univ_, codesMap);

	//! Rechargement des codes flote/planet des joueurs dont le code a été changé
	updatePlayersCode(luaEngine, codesMap, events);

	//! Excecution du code des planetes(modifie l'univers)
	//SharedLock writeLock(univ_.playersMutex);
	execPlanets(univ_, luaEngine, codesMap, events);

	//! Les combats
	execFights(univ_, database_, codesMap, events);

	//! Les flottes
	execFleets(univ_, luaEngine, codesMap, events);

	//! Supprime evenement trop vieux dans les Player et les Rapport plus utile
	database_.removeOldEvents();

	//! Ajoute les nouveau evenements dans la base
	database_.addEvents(events);

	//! Ajout des erreur de code dans la base
	std::vector<DataBase::CodeError> errorVect;
	errorVect.reserve(events.size());
	for(Event const & ev : events)
	{
		if(ev.type == Event::FleetCodeError || ev.type == Event::PlanetCodeError)
		{
			DataBase::CodeError err = {ev.value, ev.comment};
			errorVect.push_back(err);
		}
	}
	database_.addCodeErrors(errorVect);

	//! Met a jour les score des joueurs (modifie les joueurs)
	LOG4CPLUS_TRACE(logger, "updateScore start");
	updateScore(univ_, database_);

	//! CheckTutos
	LOG4CPLUS_TRACE(logger, "checkTutos start");
	checkTutos(univ_, database_, events);
	LOG4CPLUS_TRACE(logger, "checkTutos end");


	//std::cout << lexical_cast<std::string>(time(0)) + "_save.bta ";
	//save(lexical_cast<std::string>(time(0)) + "_save.bta");

	//Calcule de l'occupation memoire
	/*
	size_t planetSize = 0;
	size_t fleetSize = 0;
	size_t fleetHeapSize = 0;
	size_t fleetSizeMap = 0;
	size_t planetHeapSize = 0;
	size_t planetSizeMap = 0;
	for(auto const& planetKV: univ_.planetMap)
	{
		planetSize += sizeof(planetKV);
		planetHeapSize += planetKV.second.heap_size();
		planetSizeMap += sizeof(planetKV.first);// + 2 * sizeof(size_t);
	}
	for(auto const& fleetKV: univ_.fleetMap)
	{
		fleetSize += sizeof(fleetKV.second);
		fleetHeapSize += fleetKV.second.heap_size();
		fleetSizeMap += sizeof(fleetKV.first);// + 2 * sizeof(size_t);
	}

	size_t const univSize =  planetSize + planetHeapSize + planetSizeMap + fleetSize + fleetSizeMap + fleetHeapSize;
	cout << "Univ global size :" << univSize << endl;
	cout << "  planetMap:" << planetSize << endl;
	cout << "  planetHeapSize:" << planetHeapSize << endl;
	cout << "  planetSizeMap:" << planetSizeMap << endl;
	cout << "  fleets:" << fleetSize << endl;
	cout << "  fleetsheap:" << fleetHeapSize << endl;
	cout << "  fleetMap:" << fleetSizeMap << endl;
	cout << "  fleet count:" << double(fleetSize) / sizeof(Fleet) << endl;
	*/

	std::cout << time(0) << std::endl;
}
CATCH_LOG_EXCEPTION(logger)


//! these libs are loaded by lua.c and are readily available to any Lua program
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

//! @brief open all previous libraries
//! @see luaL_openlibs in lualib.h
void openlibs(lua_State* L)
{
	static const luaL_Reg preloadedlibs[] =
	{
		{NULL, NULL}
	};

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


boost::shared_mutex roundTimeMutex; //!< mutex pour protéger le temps du round

void Simulation::loop()
try
{
	using namespace boost::chrono;

	LuaTools::LuaEngine luaEngine;
	//lua_sethook(luaEngine.state(), luaCountHook, LUA_MASKCOUNT, 20000);
	PlayerCodeMap codesMap; //Donné non partagée entre thread

	//! @todo: remplacer openlibs par luaL_openlibs qui semble faire pareil
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

	std::vector<Event> events;

	{
		//Chargement de tout les code flote/planet de tout les joueur(chargement dans python)
		std::vector<Player> const players = database_.getPlayers();
		for(Player const & player : players)
		{
			CodeData const fleetsCode = database_.getPlayerCode(player.id, CodeData::Fleet);
			CodeData const planetsCode = database_.getPlayerCode(player.id, CodeData::Planet);
			PlayerCodes newCodes =
			{
				registerCode(luaEngine, fleetsCode, events),
				registerCode(luaEngine, planetsCode, events)
			};
			codesMap.insert(make_pair(player.id, newCodes));
		}
	}


	time_t newUpdate = time(0);
	time_t newSave = newUpdate;
	newUpdate += RoundSecond;
	newSave += SaveSecond;

	size_t gcCounter = 0;

	roundStart = system_clock::now();

	while(false == boost::this_thread::interruption_requested())
	{
		time_t const now = time(0);

		if(newSave <= now)
		{
			std::string const filename = (boost::format("save/%1%_save.bta2") % time(0)).str();
			save(filename);
			removeOldSaves();
			std::cout << "OK" << std::endl;
			newSave += SaveSecond;
		}
		bool noWait = false;
		if(noWait || newUpdate <= now)
			try
			{
				boost::chrono::system_clock::time_point roundEnd = system_clock::now();
				duration<double> sec = roundEnd - roundStart;
				{
					UniqueLock lockTime(roundTimeMutex);
					univ_.roundDuration = sec.count();
					roundStart = roundEnd;
				}

				//std::cout << newUpdate << " " << now << std::endl;
				round(luaEngine, codesMap, events);
				events.clear();
				newUpdate += RoundSecond;
				gcCounter += 1;
				if((gcCounter % 1) == 0)
				{
					std::cout << "GC : " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0);
					lua_gc(luaEngine.state(), LUA_GCCOLLECT, 0);
					std::cout << " -> " << lua_gc(luaEngine.state(), LUA_GCCOUNT, 0) << std::endl;
				}
			}
		CATCH_LOG_RETHROW(logger)
		else if(noWait == false)
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}
CATCH_LOG_RETHROW(logger)


double Simulation::getUnivTime()
{
	using namespace boost::chrono;
	UniqueLock lockTime(roundTimeMutex);
	system_clock::time_point const now = system_clock::now();
	duration<double> const fromStart = now - roundStart;
	double const roundProgress = fromStart.count() / univ_.roundDuration;
	return univ_.roundCount + roundProgress;
}


//! Logger pour le thread de sauvegarde
Logger saveLogger = Logger::getInstance(LOG4CPLUS_TEXT("Save"));

void Simulation::save(std::string const& saveName) const
{
	auto savingFunc = [](std::shared_ptr<Universe const> clone, std::string const & saveName)
	{
		using namespace boost;
		try
		{
			//! todo: Utiliser boost::filesystem avec un boost plus recent
			LOG4CPLUS_TRACE(saveLogger, "saveToStream");
			using namespace std;
			std::string const newSaveName = saveName + ".new";
			{
				ofstream saveFile(newSaveName.c_str(), ios::out | ios::binary);
				if(saveFile.is_open() == false)
					BOOST_THROW_EXCEPTION(std::ios::failure("Can't save in " + newSaveName));
				saveToStream(*clone, saveFile);
			}
			LOG4CPLUS_TRACE(saveLogger, "remove/rename");
			std::string const ansSaveName = saveName + ".ans";
			remove(ansSaveName.c_str());
			struct stat buf;
			if(stat(saveName.c_str(), &buf) == 0)
			{
				if(rename(saveName.c_str(), ansSaveName.c_str()) != 0)
					BOOST_THROW_EXCEPTION(
					  std::ios::failure(str(format("Can't rename %1% to %2%") %
					                        saveName % ansSaveName)));
			}
			if(rename(newSaveName.c_str(), saveName.c_str()) != 0)
				BOOST_THROW_EXCEPTION(
				  std::ios::failure(str(format("Can't rename %1% to %2%") %
				                        newSaveName % saveName)));

			LOG4CPLUS_TRACE(saveLogger, "copy");
			std::ifstream in(saveName, ios::binary | ios::in);
			std::ofstream out("save/last_save.bta2", ios::binary | ios::out);
			boost::iostreams::copy(in, out);
		}
		CATCH_LOG_RETHROW(saveLogger)
	};


	if(savingThread_.timed_join(boost::posix_time::seconds(0)))
	{
		LOG4CPLUS_TRACE(logger, "copy universe to save");
		SharedLock lockAll(univ_.planetsFleetsReportsmutex);
		std::shared_ptr<Universe const> clone = make_shared<Universe>(univ_);
		LOG4CPLUS_TRACE(logger, "lauch save");
		savingThread_ = boost::thread(savingFunc, clone, saveName);
	}
}


void Simulation::removeOldSaves() const
{
	using namespace boost::filesystem;
	typedef boost::filesystem::directory_iterator DirIter;

	std::set<time_t> timeSet;

	DirIter beginFileIter("save/"), endFileIter;
	for(auto path : boost::make_iterator_range(beginFileIter, endFileIter))
	{
		std::string const filename = path.path().filename().string();
		if((filename.find("_save.bta") != 10)               ||
		   (filename.size() != 19 && filename.size() != 20))
			continue;//Ce n'est pas une archive normal

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
		for(time_t fileTime : boost::make_iterator_range(begin, end))
		{
			++fileCount;
			if(fileCount == 1)
				continue;
			path const filename = str(boost::format("save/%1%_save.bta") % fileTime);
			remove(filename);
			path const filename2 = str(boost::format("save/%1%_save.bta2") % fileTime);
			remove(filename2);
		}
	}
	while(end != timeSet.begin());
}
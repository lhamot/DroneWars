//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Simulation.h"

#include "LuaUniverse.h"
#include "Rules.h"
#include "Tools.h"
#include "ScriptTools.h"
#include "fighting.h"
#include "Logger.h"
#include "DataBase.h"
#include "UnivManip.h"

#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/irange.hpp>
#include <boost/format.hpp>
#include <boost/multi_array.hpp>


//! Verrou en écriture
typedef boost::unique_lock<Universe::Mutex> UniqueLock;
//! Verrou en lecture
typedef boost::shared_lock<Universe::Mutex> SharedLock;
//! Verrou en lecture, mutable en écriture
typedef boost::upgrade_lock<Universe::Mutex> UpgradeLock;
//! Verrou en écriture construit a partir d'un UpgradeLock
typedef boost::upgrade_to_unique_lock<Universe::Mutex> UpToUniqueLock;

using namespace std;
namespace BL = boost::locale;

using namespace log4cplus;
//! Logger du thread de simulation
static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("Simulation"));

//! Verifie à la sortie du scope si le joueur a appelé la fonction de log
class CheckPlayerLog : boost::noncopyable
{
public:
	//! ctor
	CheckPlayerLog(ScriptTools::Engine& L,
	               PlayerCodes::ObjectMap& codeMap,
	               Player const& player,
	               Coord const& coord,
	               vector<Event>& events) :
		state_(L),
		codeMap_(codeMap),
		player_(player),
		coord_(coord),
		events_(events)
	{
	}
	//! ctor
	CheckPlayerLog(ScriptTools::Engine& L,
	               PlayerCodes::ObjectMap& codeMap,
	               Player const& player,
	               Fleet::ID const& fleetId,
	               vector<Event>& events) :
		state_(L),
		codeMap_(codeMap),
		player_(player),
		fleetID_(fleetId),
		events_(events)
	{
	}

	//! Ajoute les log du joueur dans la base si il as la compétence.
	~CheckPlayerLog()
	try
	{
		ScriptTools::Object logger =
		  ScriptTools::refFromName(state_, "logger");
		if(ScriptTools::isValid(logger))
		{
			if(playerCanLog(player_))
			{
				Event event(player_.id, time(0), Event::PlayerLog);
				event.setComment(ScriptTools::extract<std::string>(logger));
				if(fleetID_ == Fleet::NoId)
					event.setPlanetCoord(coord_);
				else
					event.setFleetID(fleetID_);
				events_.push_back(Event(event));
			}
			else
			{
				addErrorMessage(codeMap_,
				                BL::gettext("Log failed"),
				                events_);
			}
		}
	}
	CATCH_LOG_EXCEPTION(logger);

private:
	ScriptTools::Engine& state_;        //!< Etats lua
	PlayerCodes::ObjectMap& codeMap_;   //!< Scripts du joueur
	Player const& player_;              //!< Donées du joueur
	Coord coord_ = Coord();             //!< Coordonée de la planète(optionel)
	Fleet::ID fleetID_ = Fleet::NoId;   //!< ID de la flote (optionel)
	std::vector<Event>& events_;        //!< Liste des evenements du round
};

//! @brief Verifie à la sortie du scope
//!   si le joueur à modifié la memoire de sa planète ou flotte
class CheckMemory : boost::noncopyable
{
public:
	//! ctor
	template<typename T>
	CheckMemory(PlayerCodes::ObjectMap& codeMap,
	            Player const& player,
	            T& fleetOrPlanet,
	            std::vector<Event>& events):
		codeMap_(codeMap),
		player_(player),
		oldMem_(fleetOrPlanet.memory),
		memory_(fleetOrPlanet.memory),
		events_(events)
	{
	}

	//! @brief Verifie si le joueur à modifié la memoire de sa planète / flotte
	//! pour l'autoriser ou non, en fonction de son skill mémoire.
	~CheckMemory()
	try
	{
		cleanPtreeNil(memory_);
		if(acceptPtree(player_, memory_) == false)
		{
			memory_ = oldMem_;
			addErrorMessage(codeMap_,
			                BL::gettext("Too much items in your "
			                            "planet memory for your Memory skill level."),
			                events_);
		}
	}
	CATCH_LOG_EXCEPTION(logger);

private:
	PlayerCodes::ObjectMap& codeMap_;  //!< Scripts du joueur
	Player const& player_;             //!< Donées du joueur
	TypedPtree oldMem_;                //!< Mémoire d'origine
	TypedPtree& memory_;               //!< Mémoire de sa planète / flotte (ref)
	std::vector<Event>& events_;       //!< Liste des evenements du round
};

//! @brief Wrap une function lua en c++, tout en testant la mémoire et les log
//! a l'aide de CheckPlayerLog et CheckMemory
class DWObject
{
public:
	//! ctor
	DWObject(ScriptTools::Object const& obj) : obj_(obj)
	{
	}

	//! Appel la fonction (avec retour) puis test la mémoire et les logs
	template<typename T, typename ...Args>
	void call(
	  ScriptTools::Engine& engine,
	  Player const& player,
	  PlayerCodes::ObjectMap& codeMap,
	  vector<Event>& events,
	  T& fleetOrPlanet,
	  Args const& ... args)
	{
		CheckMemory checkMem(codeMap, player, fleetOrPlanet, events);
		CheckPlayerLog checkPlayerLog(
		  engine, codeMap, player, getID(fleetOrPlanet), events);
		return obj_->call(fleetOrPlanet, args...);
	}

	//! Appel la fonction (sans retour) puis test la mémoire et les logs
	template<typename R, typename T, typename ...Args>
	R call(
	  ScriptTools::Engine& engine,
	  Player const& player,
	  PlayerCodes::ObjectMap& codeMap,
	  vector<Event>& events,
	  T& fleetOrPlanet,
	  Args const& ... args)
	{
		CheckMemory checkMem(codeMap, player, fleetOrPlanet, events);
		CheckPlayerLog checkPlayerLog(
		  engine, codeMap, player, getID(fleetOrPlanet), events);
		return obj_->call<R>(fleetOrPlanet, args...);
	}

private:
	//! @return la coordoné de la planète
	static Coord getID(Planet const& planet)
	{
		return planet.coord;
	}

	//! @return l'ID de la flotte
	static Fleet::ID getID(Fleet const& fleet)
	{
		return fleet.id;
	}

	ScriptTools::Object obj_;   //!< Fonction lua
};

//! Place un joueur dans les registre lua sous le nom "currentPlayer"
//!  et reinitialise le hook compteur d'instruction.
void prepareLuaCall(Universe const& univ, Polua::object stript, Player const& player)
{
	lua_sethook(
	  stript->state(), LuaTools::luaCountHook, LUA_MASKCOUNT, LuaTools::LuaMaxInstruction);
	Polua::pushTemp(stript->state(), player);
	lua_setglobal(stript->state(), "currentPlayer");
	lua_pushinteger(stript->state(), univ.roundCount);
	lua_setglobal(stript->state(), "roundIndex");
	lua_pushnil(stript->state());
	lua_setglobal(stript->state(), "logger");
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


//! Fait interpreter un script à lua et le transforme en PlayerCodes::ObjectMap
PlayerCodes::ObjectMap registerCode(
  ScriptTools::Engine& scriptEngine,
  CodeData const& code,
  std::vector<Event>& events)
try
{
	using namespace std;

	luaL_dostring(scriptEngine.state(), "AI = nil");
	luaL_dostring(scriptEngine.state(), "AI_action = nil");
	luaL_dostring(scriptEngine.state(), "AI_do_gather = nil");
	luaL_dostring(scriptEngine.state(), "AI_do_fight = nil");
	luaL_dostring(scriptEngine.state(), "AI_emit = nil");
	luaL_dostring(scriptEngine.state(), "AI_do_escape = nil");

	std::string codeString = code.code;

	if(luaL_dostring(scriptEngine.state(), codeString.c_str()) != 0)
	{
		char const* message = lua_tostring(scriptEngine.state(), -1);
		addErrorMessage(code, message, events);
		return PlayerCodes::ObjectMap(); //Vide car toute les fonction sont invalides
	}
	else
	{
		PlayerCodes::ObjectMap result;
		result.playerId = code.playerId;
		result.scriptID = code.id;
		result.target = code.target;
		result.functions["AI"] = ScriptTools::refFromName(scriptEngine, "AI");
		result.functions["action"] = ScriptTools::refFromName(scriptEngine, "AI_action");
		result.functions["do_gather"] = ScriptTools::refFromName(scriptEngine, "AI_do_gather");
		result.functions["do_fight"] = ScriptTools::refFromName(scriptEngine, "AI_do_fight");
		auto emitFunc = ScriptTools::refFromName(scriptEngine, "AI_emit");
		if(emitFunc->is_valid())
			result.functions["emit"] = emitFunc;
		auto escapeFunc = ScriptTools::refFromName(scriptEngine, "AI_do_escape");
		if(escapeFunc->is_valid())
			result.functions["do_escape"] = escapeFunc;
		return result;
	}
}
catch(Polua::Exception& ex)
{
	addErrorMessage(code, ex.what(), events);
	return PlayerCodes::ObjectMap(); //Vide car toute les fonction sont invalides
}


//! Excecute le code de la planete et lui ajoute des tache
boost::optional<PlanetAction> execPlanetScript(
  Universe const& univ,
  ScriptTools::Engine& engine,
  PlayerCodes::ObjectMap& codeMap,
  Player const& player,
  Planet& planetCopy,
  std::vector<Fleet const*> const& fleetList,
  std::vector<Event>& events)
try
{
	auto codeIter = codeMap.functions.find("AI");
	if(codeIter == codeMap.functions.end())
		return boost::none; //Le code a été invalidé
	ScriptTools::Object code = codeIter->second;
	if(ScriptTools::isFunction(code) == false)
	{
		//call_function crash quand on lui passe autre chose qu'une fonction
		boost::format trans(BL::gettext("Procedure \"%1%\" not found."));
		std::string const message = str(trans % "AI");
		addErrorMessage(codeMap, message, events);
		return boost::none;
	}

	if(planetCopy.buildingList.size() != Building::Count)
		BOOST_THROW_EXCEPTION(
		  std::logic_error("planet.buildingList.size() != Building::Count"));

	prepareLuaCall(univ, code, player);
	DWObject code2(code);
	PlanetAction action = code2.call<PlanetAction>(
	                        engine, player, codeMap, events,
	                        planetCopy, fleetList);
	return action;
}
catch(Polua::Exception const& ex)
{
	addErrorMessage(codeMap, ex.what(), events);
	return boost::none;
}

//! Applique l'action demandé par le script de la planete
//! C-à-d ajoute ou arrete une tache de la planète
void applyPlanetAction(
  Universe const& univ_,
  std::map<Player::ID, Player> const& playerMap,
  Planet& sciptModifiedPlanet,
  Planet& planet,
  PlanetAction const& action,
  size_t const playerFleetCount)
{
	planet.memory.swap(sciptModifiedPlanet.memory);
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
		Player const& player = mapFind(playerMap, planet.playerId)->second;
		if(canBuild(player, planet, action.ship, playerFleetCount))
			addTask(planet, univ_.roundCount, action.ship, 1);
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
	static_assert(PlanetAction::Count == 4, "Missing PlanetAction case");
}


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
		ScriptTools::Object methode = codeIter->second;
		if(ScriptTools::isFunction(methode))
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


void gatherIfWant(
  Universe const& univ,
  Player const& player,
  ScriptTools::Engine& scriptEngine,
  PlayerCodes::ObjectMap& codeMap,
  Fleet& fleet,
  FleetCoordMap& fleetMap,
  std::vector<Event>& events)
{
	auto localFleetsKV = fleetMap.equal_range(fleet.coord);
	if(checkLuaMethode(codeMap, "do_gather", events))
	{
		ScriptTools::Object do_gather = mapFind(codeMap.functions, "do_gather")->second;
		auto fleetIter = localFleetsKV.first;
		while(fleetIter != localFleetsKV.second)
		{
			Fleet& otherFleet = fleetIter->second;
			if((otherFleet.id > fleet.id) &&
			   (otherFleet.playerId == fleet.playerId) &&
			   canGather(player, fleet, otherFleet))
			{
				bool wantGather1 = false, wantGather2 = false;
				{
					prepareLuaCall(univ, do_gather, player);
					DWObject do_gather2(do_gather);
					wantGather1 = do_gather2.call<bool>(
					                scriptEngine, player, codeMap, events,
					                fleet, otherFleet);
					prepareLuaCall(univ, do_gather, player);
					wantGather2 = do_gather2.call<bool>(
					                scriptEngine, player, codeMap, events,
					                otherFleet, fleet);
				}

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
}

//! std::shared_ptr de TypedPtree
typedef std::shared_ptr<TypedPtree> TypedPtreePtr;
//! std::vector de TypedPtree*
typedef std::vector<TypedPtree*> MailBox;

//! Excecute le script de la flotte
boost::optional<FleetAction> execFleetScript(
  Universe const& univ_,
  ScriptTools::Engine& engine,
  PlayerCodes::ObjectMap& codeMap,
  Player const& player,
  Fleet& fleet,
  MailBox const& mails,
  std::vector<Event>& events)
try
{
	if(checkLuaMethode(codeMap, "action", events) == false)
		return boost::none;
	ScriptTools::Object actionFunc = mapFind(codeMap.functions, "action")->second;
	auto planetIter = univ_.planetMap.find(fleet.coord);
	Planet const* planet = nullptr;
	if(planetIter != univ_.planetMap.end())
		planet = &(planetIter->second);
	FleetAction action(FleetAction::Nothing);
	if(planet && fleetCanSeePlanet(fleet, *planet, univ_) == false)
		planet = nullptr;
	prepareLuaCall(univ_, actionFunc, player);
	DWObject actionFunc2(actionFunc);
	if(planet)
		action = actionFunc2.call<FleetAction>(
		           engine, player, codeMap, events,
		           fleet, *planet, mails);
	else
		action = actionFunc2.call<FleetAction>(
		           engine, player, codeMap, events,
		           fleet, false, mails);
	return action;
}
catch(Polua::Exception const& ex)
{
	addErrorMessage(codeMap, ex.what(), events);
	return boost::none;
}

//! Applique l'action demandé par le script de la flotte
void applyFleetScript(Universe& univ_,
                      std::map<Player::ID, Player> const& playerMap,
                      Fleet& scriptModifiedFleet,
                      Fleet& fleet,
                      FleetAction const& action,
                      std::map<Player::ID, size_t>& playersPlanetCount,
                      std::vector<Event>& events)
{
	scriptModifiedFleet.memory.swap(fleet.memory);

	auto planetIter = univ_.planetMap.find(fleet.coord);
	Planet* planet = nullptr;
	if(planetIter != univ_.planetMap.end())
		planet = &(planetIter->second);

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
	{
		Player const& player = mapFind(playerMap, fleet.playerId)->second;
		if(planet &&
		   canColonize(player, fleet, *planet, playersPlanetCount[player.id]))
			addTaskColonize(fleet, univ_.roundCount, *planet);
		break;
	}
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
}



//! Rechargement des codes flote/planet des joueurs dont le code a été changé,
//! Modifie la codesMap et playerToReload_
void Simulation::updatePlayersCode(ScriptTools::Engine& luaEngine,
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


//! Genere une map le joueur ID=>Joueur a partir de la base de donnée SQL
std::map<Player::ID, Player> getPlayerMap(DataBase const& database)
{
	std::vector<Player> players = database.getPlayers();
	std::map<Player::ID, Player> const playerMap = [&]()
	{
		std::map<Player::ID, Player> playerMap;
		for(Player const & player : players)
			playerMap.insert(make_pair(player.id, player));
		return playerMap;
	}();
	return playerMap;
}

//! Genere une map le joueur ID=>Joueur a partir de la base de donnée SQL
std::map<Alliance::ID, Alliance> getAllianceMap(DataBase const& database)
{
	std::vector<Alliance> alliances = database.getAlliances();
	std::map<Alliance::ID, Alliance> allianceMap;
	for(Alliance const & alli : alliances)
		allianceMap.insert(make_pair(alli.id, alli));
	return allianceMap;
}

//! Simule le round pour toute les planètes
void execPlanets(Universe& univ_,
                 ScriptTools::Engine& engine,
                 std::map<Player::ID, Player> const& playerMap,
                 PlayerCodeMap& codesMap,
                 std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	std::map<Player::ID, size_t> playerFleetCounts;

	FleetCoordMap fleetMap;
	for(Fleet const & fleet : univ_.fleetMap | boost::adaptors::map_values)
	{
		fleetMap.insert(make_pair(fleet.coord, fleet));
		playerFleetCounts[fleet.playerId] += 1;
	}

	//Les planètes
	{
		for(Planet & planet : univ_.planetMap | boost::adaptors::map_values)
			planetRound(univ_, planet, events);
	}

	struct ScriptInputs
	{
		Planet planet;
		std::vector<Fleet const*> fleetList;
	};
	std::vector<ScriptInputs> ownedPlanetList;
	ownedPlanetList.reserve(univ_.planetMap.size());
	for(Universe::PlanetMap::value_type const & planetNVP : univ_.planetMap)
	{
		Planet const& planet = planetNVP.second;
		if(planet.playerId != Player::NoId)
		{
			ownedPlanetList.push_back(ScriptInputs());
			ownedPlanetList.back().planet = planet;

			auto localFleets = fleetMap.equal_range(planet.coord);
			auto getFleetPointer = []
			                       (FleetCoordMap::value_type const & coordFleet)
			{
				return &coordFleet.second;
			};
			boost::transform(localFleets,
			                 back_inserter(ownedPlanetList.back().fleetList),
			                 getFleetPointer);
		}
	}

	struct ExtPlanetAction
	{
		Coord planetCoord;
		Planet planet;
		boost::optional<PlanetAction> optAction;

		explicit ExtPlanetAction(Coord const& coord): planetCoord(coord) {}
		ExtPlanetAction(Coord const& coord,
		                Planet const& planet,
		                boost::optional<PlanetAction> const& action
		               ):
			planetCoord(coord), planet(planet), optAction(action)
		{
		}
	};
	std::vector<ExtPlanetAction> planetActionList;
	planetActionList.reserve(ownedPlanetList.size());
	for(ScriptInputs const sciptInputs : ownedPlanetList)
	{
		Planet planet = sciptInputs.planet;
		Player const& player = mapFind(playerMap, planet.playerId)->second;
		boost::optional<PlanetAction> action =
		  execPlanetScript(univ_,
		                   engine,
		                   codesMap[planet.playerId].planetsCode,
		                   player,
		                   planet,
		                   sciptInputs.fleetList,
		                   events);
		planetActionList.push_back(
		  ExtPlanetAction(planet.coord, planet, action));
	}

	{
		for(ExtPlanetAction & extAction : planetActionList)
		{
			Planet& planet = univ_.planetMap[extAction.planetCoord];
			if(extAction.optAction)
				applyPlanetAction(univ_,
				                  playerMap,
				                  extAction.planet,
				                  planet,
				                  *extAction.optAction,
				                  playerFleetCounts[planet.playerId]);
		}
	}
	LOG4CPLUS_TRACE(logger, "exit");
}

//! Retire les flotte qui veulent et parviennent à s'échaper
//! @return La liste des flottes echapées
std::vector<Fleet*> removeEscapedFleet(
  Universe const& univ,
  ScriptTools::Engine& engine,
  std::map<Player::ID, Player> const playerMap,
  Planet const* planetPtr,
  PlayerCodeMap& codesMap,
  std::vector<Event>& events,
  std::map<Fleet::ID, double>& escapeProbaMap,
  std::vector<Fleet*>& fleetVect)
{
	std::vector<Fleet*> escapedFleets;
	std::vector<Fleet*> fightingFleets;
	std::vector<Fleet const*> otherFleets(fleetVect.begin() + 1, fleetVect.end());
	for(size_t i = 0; i < fleetVect.size(); ++i)
	{
		Fleet* fleet = fleetVect[i];
		Player const& player = mapFind(playerMap, fleet->playerId)->second;
		auto fleetScripts = codesMap[player.id].fleetsCode;
		ScriptTools::Object doEscape = fleetScripts.functions["do_escape"];
		bool wantEscape = false;
		if(ScriptTools::isFunction(doEscape))
			try
			{
				prepareLuaCall(univ, doEscape, player);
				DWObject doEscape2(doEscape);
				if(fleet->player == nullptr)
				{
					std::cout << "fleet->player == nullptr" << std::endl;
					BOOST_THROW_EXCEPTION(
					  std::logic_error("fleet->player == nullptr"));
				}
				wantEscape = doEscape2.call<bool>(
				               engine, player, codesMap[player.id].fleetsCode, events,
				               *fleet, planetPtr, otherFleets);
			}
			catch(Polua::Exception& ex)
			{
				addErrorMessage(fleetScripts, ex.what(), events);
			}
		if(wantEscape)
		{
			double const escapeProba = calcEscapeProba(player, *fleet, planetPtr, otherFleets);
			escapeProbaMap[fleet->id] = escapeProba;
			if(isEscapeSuccess(escapeProba) == false)
			{
				//if(fleet->playerId == 52)
				//	std::cout << "La flotte : " << fleet->id << " c'est faite repérer!!" << std::endl;
				fightingFleets.push_back(fleet);
			}
			else
			{
				//if(fleet->playerId == 52)
				//	std::cout << "La flotte : " << fleet->id << " c'est echapé avec succes!!" << std::endl;
				escapedFleets.push_back(fleet);
			}
		}
		else
			fightingFleets.push_back(fleet);

		if(i < otherFleets.size())
			otherFleets[i] = fleet;
	}
	std::swap(fightingFleets, fleetVect);
	return escapedFleets;
}


//! @brief Verifie si player1 est ostile contre player2
//!
//! Si l'information n'est pas déja playerFightPlayer,
//! demande au script do_fight pour savoir si player1 veut attaquer player2
//! et ajoute le résultat dans playerFightPlayer
void addIfTheyWantFight(
  Universe const& univ,                  //!< L'Univers
  Player const& player1,                 //!< Player attaquant
  Player const& player2,                 //!< Player attaqué
  PlayersFightingMap& playerFightPlayer, //!< Qui veut attaquer qui
  PlayerCodeMap& codesMap,               //!< Scripts de tout les joueurs
  std::vector<Event>& events             //!< Evenements du rounds
)
{
	auto iter = playerFightPlayer.find(std::make_pair(player1.id, player2.id));
	if(iter == playerFightPlayer.end())
	{
		bool result = true;
		auto fleetScripts = codesMap[player1.id].fleetsCode;
		ScriptTools::Object doFight = fleetScripts.functions["do_fight"];
		if(ScriptTools::isFunction(doFight))
		{
			prepareLuaCall(univ, doFight, player1);
			try
			{
				result = doFight->call<bool>(player1, player2);
			}
			catch(Polua::Exception& ex)
			{
				addErrorMessage(fleetScripts, ex.what(), events);
			}
		}
		playerFightPlayer[make_pair(player1.id, player2.id)] = result;
	}
}


//! Les combats
void execFights(Universe& univ_,
                ScriptTools::Engine& engine,
                DataBase& database,
                std::map<Player::ID, Player> const& playerMap,
                PlayerCodeMap& codesMap,
                std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	if(univ_.fleetMap.empty()) //si il y as des flottes
		return;

	std::map<Player::ID, uint32_t> experienceMap;

	vector<Fleet::ID> deadFleets;
	deadFleets.reserve(univ_.fleetMap.size());
	vector<Coord> lostPlanets;
	lostPlanets.reserve(univ_.planetMap.size());
	typedef std::multimap<Coord, FighterPtr, CompCoord> FleetCoordMultimap;
	FleetCoordMultimap fleetMultimap;

	for(Planet & planet : univ_.planetMap | boost::adaptors::map_values)
	{
		if(planet.playerId != Player::NoId)
			fleetMultimap.insert(make_pair(planet.coord, FighterPtr(&planet)));
	}

	//! Remplissage de la multimap de flote Coord=>flote
	for(Fleet & fleet : univ_.fleetMap | boost::adaptors::map_values)
		fleetMultimap.insert(make_pair(fleet.coord, FighterPtr(&fleet)));

	std::vector<Fleet*> fleetVect;
	std::vector<Event> tempEvents;
	std::vector<FightReport> tempReports;
	PlayersFightingMap playerFightPlayer;
	tempEvents.reserve(1000000);
	tempReports.reserve(1000000);
	//! Pour chaque coordonées, on accede au range des flotes
	auto fleetMultimapEnd = fleetMultimap.end();
	for(FleetCoordMultimap::iterator iter1 = fleetMultimap.begin(), iter2 = nextNot(fleetMultimap, iter1);
	    iter1 != fleetMultimapEnd;
	    iter1 = iter2, iter2 = nextNot(fleetMultimap, iter1))
	{
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
		std::set<Player::ID> playerSet;
		for(FighterPtr const & fighterPtr : fleetRange | boost::adaptors::map_values)
		{
			if(fighterPtr.isPlanet())
			{
				planetPtr = fighterPtr.getPlanet();
				playerSet.insert(planetPtr->playerId);
			}
			else
			{
				fleetVect.push_back(fighterPtr.getFleet());
				playerSet.insert(fleetVect.back()->playerId);
			}
		}
		if(playerSet.size() < 2)
			continue;

		//if(planetPtr && planetPtr->playerId != Player::NoId)
		//	planetPtr = nullptr;

		//! - On excecute le combats

		//! - Retrait de ceux qui s'échapent
		std::map<Fleet::ID, double> escapeProbaMap;
		std::vector<Fleet*> escapedFleets =
		  removeEscapedFleet(
		    univ_, engine, playerMap, planetPtr, codesMap, events, escapeProbaMap, fleetVect);

		playerSet.clear();
		for(auto fleet : fleetVect)
			playerSet.insert(fleet->playerId);
		if(planetPtr)
			playerSet.insert(planetPtr->playerId);
		if(playerSet.size() < 2)
			continue;

		{
			std::vector<Player::ID> playerVect(playerSet.begin(), playerSet.end());
			for(auto iter1 = playerVect.begin(); iter1 != playerVect.end(); ++iter1)
			{
				for(auto iter2 = iter1 + 1; iter2 != playerVect.end(); ++iter2)
				{
					Player const& player1 = mapFind(playerMap, *iter1)->second;
					Player const& player2 = mapFind(playerMap, *iter2)->second;
					addIfTheyWantFight(univ_,
					                   player1,
					                   player2,
					                   playerFightPlayer,
					                   codesMap,
					                   events);
					addIfTheyWantFight(univ_,
					                   player2,
					                   player1,
					                   playerFightPlayer,
					                   codesMap,
					                   events);
				}
			}
		}

		FightReport fightReport;
		fight(fleetVect, playerFightPlayer, planetPtr, codesMap, fightReport);
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

		//! - On ajoute les evenement/message dans les flottes/joueur
		std::set<Player::ID> informedPlayer;
		for(auto fleetReportPair : range)
		{
			//! --Pour les flotes
			Fleet* fleetPtr = fleetReportPair.get<0>();
			Report<Fleet>& report = fleetReportPair.get<1>();
			Fleet const& fleet = *fleetPtr;
			auto escapeMapIter = escapeProbaMap.find(fleet.id);
			report.wantEscape = escapeMapIter != escapeProbaMap.end();
			report.escapeProba = report.wantEscape ? escapeMapIter->second : 0.;
			if(report.isDead)
			{
				deadFleets.push_back(fleet.id);
				if(informedPlayer.count(fleet.playerId) == 0)
				{
					tempEvents.push_back(
					  Event(fleet.playerId, time(0), Event::FleetLose));
					if(playerCanSeeFightReport(
					     mapFind(playerMap, fleet.playerId)->second))
						tempEvents.back().setValue(tempReports.size());
					informedPlayer.insert(fleet.playerId);
				}
			}
			else if(report.hasFight)
			{
				tempEvents.push_back(
				  Event(fleet.playerId, time(0), Event::FleetWin)
				  .setValue(tempReports.size())
				  .setFleetID(fleet.id));
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
					tempEvents.push_back(
					  Event(planet.playerId, time(0), Event::PlanetLose));
					if(playerCanSeeFightReport(
					     mapFind(playerMap, planet.playerId)->second))
						tempEvents.back().setValue(tempReports.size());
				}
			}
			else if(fightReport.planet.get().hasFight)
			{
				if(planet.playerId == Player::NoId)
					BOOST_THROW_EXCEPTION(
					  std::logic_error("planet.playerId == Player::NoId"));
				tempEvents.push_back(
				  Event(planet.playerId, time(0), Event::PlanetWin)
				  .setValue(tempReports.size())
				  .setPlanetCoord(planet.coord));
			}
		}
		for(Fleet * fleet : escapedFleets)
		{
			double escaProba = escapeProbaMap[fleet->id];
			tempEvents.push_back(
			  Event(fleet->playerId, time(0), Event::FightAvoided)
			  .setValue(tempReports.size())
			  .setValue2(boost::numeric_cast<intptr_t>(escaProba * 1000000))
			  .setFleetID(fleet->id));
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

		//! - On ajoute le rapport dans la base de donné
		tempReports.push_back(fightReport);
	}

	size_t const firstReportID = 1 + database.addFightReports(tempReports) - tempReports.size();
	for(Event & event : tempEvents)
		if(event.value != -1)
			event.value += firstReportID;
	events.insert(events.end(), tempEvents.begin(), tempEvents.end());

	//! On envoie dans la base les gain d'eperience
	database.updateXP(experienceMap);

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


//! Récupère les messages emit par une flotte
TypedPtreePtr getFleetEmission(
  Universe const& univ_,
  ScriptTools::Engine& engine,
  PlayerCodes::ObjectMap& codeMap,
  Player const& player,
  Fleet& fleet,
  std::vector<Event>& events)
try
{
	if(checkLuaMethode(codeMap, "emit", events) == false)
		return TypedPtreePtr();
	ScriptTools::Object emitFunc = mapFind(codeMap.functions, "emit")->second;
	auto planetIter = univ_.planetMap.find(fleet.coord);
	Planet const* planet = nullptr;
	if(planetIter != univ_.planetMap.end())
		planet = &(planetIter->second);
	if(planet && fleetCanSeePlanet(fleet, *planet, univ_) == false)
		planet = nullptr;
	prepareLuaCall(univ_, emitFunc, player);
	TypedPtreePtr pt = make_shared<TypedPtree>();
	DWObject emitFunc2(emitFunc);
	if(planet)
		*pt = emitFunc2.call<TypedPtree>(engine, player, codeMap, events,
		                                 fleet, *planet);
	else
		*pt = emitFunc2.call<TypedPtree>(engine, player, codeMap, events,
		                                 fleet, false);
	return pt;
}
catch(Polua::Exception const& ex)
{
	addErrorMessage(codeMap, ex.what(), events);
	return TypedPtreePtr();
}

//! Excecutes les code des flottes
void execFleets(
  Universe& univ_,
  std::map<Player::ID, Player> const& playerMap,
  ScriptTools::Engine& engine,
  PlayerCodeMap& codesMap,
  std::vector<Event>& events)
{
	LOG4CPLUS_TRACE(logger, "enter");

	FleetCoordMap fleetMap;
	for(Fleet & fleet : univ_.fleetMap | boost::adaptors::map_values)
		fleetMap.insert(make_pair(fleet.coord, fleet));

	std::map<Player::ID, size_t> playersPlanetCount;
	for(Planet const & planet : univ_.planetMap | boost::adaptors::map_values)
		++playersPlanetCount[planet.playerId];

	for(auto iter = fleetMap.begin(); iter != fleetMap.end(); ++iter)
	{
		gatherIfWant(univ_,
		             mapFind(playerMap, iter->second.playerId)->second,
		             engine,
		             codesMap[iter->second.playerId].fleetsCode,
		             iter->second,
		             fleetMap,
		             events);
		fleetRound(univ_, iter->second, events, playersPlanetCount);
	}

	map_remove_erase_if(univ_.fleetMap, []
	                    (Universe::FleetMap::value_type const & nvp)
	{
		return boost::accumulate(nvp.second.shipList, 0) == 0;
	});

	//Excecution et stockage des émissions
	using namespace boost;
	std::vector<std::shared_ptr<TypedPtree> > mailOwner;
	typedef std::map<Player::ID, MailBox> PlayersMailBoxes;
	typedef multi_array<PlayersMailBoxes, 3> UniverseMailBoxes;
	UniverseMailBoxes univMailBoxes(
	  extents[Universe::MapSizeZ][Universe::MapSizeY][Universe::MapSizeX]);

	size_t emitedCount = 0;
	size_t storedCount = 0;
	size_t earedCount = 0;
	for(auto iter = fleetMap.begin(); iter != fleetMap.end(); ++iter)
	{
		Player const& player = mapFind(playerMap, iter->second.playerId)->second;
		Fleet fleet = iter->second;
		Coord const coord = fleet.coord;
		TypedPtreePtr pt = getFleetEmission(univ_,
		                                    engine,
		                                    codesMap[iter->second.playerId].fleetsCode,
		                                    player,
		                                    fleet,
		                                    events);
		if(!pt)
			continue;
		++emitedCount;
		int const dist = numeric_cast<int>(playerEmissionRange(player));
		if(dist == 0)
			continue;
		mailOwner.push_back(pt);
		auto bornedRange = [](int first, int pastLast, int bornSup)
		{
			return
			  boost::irange(std::max<int>(std::min<int>(first, pastLast), 0),
			                std::min<int>(pastLast, bornSup));
		};
		auto xrange = bornedRange(
		                coord.X - (dist - 1), coord.X + dist, Universe::MapSizeX);
		auto yrange = bornedRange(
		                coord.Y - (dist - 1), coord.Y + dist, Universe::MapSizeY);
		auto zrange = bornedRange(
		                coord.Z - (dist - 1), coord.Z + dist, Universe::MapSizeZ);
		for(size_t x : xrange)
		{
			for(size_t y : yrange)
			{
				for(size_t z : zrange)
				{
					univMailBoxes[x][y][z][player.id].push_back(pt.get());
					++storedCount;
				}
			}
		}
	}

	struct FleetAndAction
	{
		Fleet* fleet;
		Fleet scriptModifiedFleet;
		boost::optional<FleetAction> action;
	};
	std::vector<FleetAndAction> scriptInputsList;
	scriptInputsList.reserve(fleetMap.size());
	for(auto iter = fleetMap.begin(); iter != fleetMap.end(); ++iter)
	{
		Player const& player = mapFind(playerMap, iter->second.playerId)->second;
		Fleet scriptModifiedFleet = iter->second;
		Coord coord = scriptModifiedFleet.coord;
		MailBox mails = univMailBoxes[coord.X][coord.Y][coord.Z][player.id];
		earedCount += mails.size();
		boost::optional<FleetAction> action =
		  execFleetScript(univ_,
		                  engine,
		                  codesMap[iter->second.playerId].fleetsCode,
		                  player,
		                  scriptModifiedFleet,
		                  mails,
		                  events);
		FleetAndAction fleetAndAction =
		{
			&iter->second, scriptModifiedFleet, action
		};
		scriptInputsList.push_back(fleetAndAction);
	}

	for(FleetAndAction & fleetAndAction : scriptInputsList)
	{
		if(fleetAndAction.action)
			applyFleetScript(univ_,
			                 playerMap,
			                 fleetAndAction.scriptModifiedFleet,
			                 *fleetAndAction.fleet,
			                 *fleetAndAction.action,
			                 playersPlanetCount,
			                 events);
	}

	std::map<Fleet::ID, Fleet> newFleetMap;
	for(Fleet & fleet : fleetMap | boost::adaptors::map_values)
	{
		if(fleet.empty() == false) //Si flotte vide, on garde pas
			newFleetMap.insert(make_pair(fleet.id, fleet));
	}
	newFleetMap.swap(univ_.fleetMap);

	LOG4CPLUS_TRACE(logger, "exit");
}

void Simulation::createNewPlayersPlanets(Universe& univCopy)
{
	UniqueLock lock(planetToCreateMutex_);
	while(planetToCreate_.empty() == false)
	{
		Player::ID const pid = planetToCreate_.front();
		planetToCreate_.pop();
		lock.unlock();
		Coord const coord = ::createMainPlanet(univCopy, pid);
		database_.setPlayerMainPlanet(pid, coord);
		reloadPlayer(pid);
		lock.lock();
	}
}

void Simulation::round(ScriptTools::Engine& scriptEngine,
                       PlayerCodeMap& codesMap,
                       std::vector<Event>& events)
try
{
	//! @todo: Ne plus passer events en argument
	LOG4CPLUS_TRACE(logger, "enter");

	std::cout << time(0) << " ";

	univ_.roundCount += 1; //1 round

	Universe univCopy = univ_;

	//! Création des planètes des nouveaux joueurs
	createNewPlayersPlanets(univCopy);

	std::map<Player::ID, Player> playerMap = getPlayerMap(database_);
	for(Fleet & fleet : univCopy.fleetMap | boost::adaptors::map_values)
		fleet.player = &mapFind(playerMap, fleet.playerId)->second;

	for(Planet & planet : univCopy.planetMap | boost::adaptors::map_values)
		if(planet.playerId != Player::NoId)
			planet.player = &mapFind(playerMap, planet.playerId)->second;

	std::map<Alliance::ID, Alliance> allienceMap = getAllianceMap(database_);
	for(Player & player : playerMap | boost::adaptors::map_values)
		if(player.allianceID)
			player.alliance = &mapFind(allienceMap, player.allianceID)->second;

	for(Alliance & alliance : allienceMap | boost::adaptors::map_values)
		alliance.master = &mapFind(playerMap, alliance.masterID)->second;

	//! Désactivation de tout les codes qui echoue
	//disableFailingCode(univCopy, codesMap);

	//! Rechargement des codes flote/planet des joueurs dont le code a été changé
	updatePlayersCode(scriptEngine, codesMap, events);

	//! Excecution du code des planetes(modifie l'univers)
	execPlanets(univCopy, scriptEngine, playerMap, codesMap, events);

	//! Les combats
	execFights(univCopy, scriptEngine, database_, playerMap, codesMap, events);

	//! Les flottes
	execFleets(univCopy, playerMap, scriptEngine, codesMap, events);

	//! Supprime evenement trop vieux dans les Player et les Rapport plus utile
	{
		std::map<Player::ID, size_t> maxEventPerPlayer;
		for(Player const & player : database_.getPlayers())
			maxEventPerPlayer[player.id] = getMaxEventCount(player);
		database_.removeOldEvents(maxEventPerPlayer);
	}

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
	updateScore(univCopy, database_);

	//! CheckTutos
	LOG4CPLUS_TRACE(logger, "checkTutos start");
	checkTutos(univCopy, database_, events);
	LOG4CPLUS_TRACE(logger, "checkTutos end");

	for(Fleet & fleet : univCopy.fleetMap | boost::adaptors::map_values)
		fleet.player = nullptr;

	{
		UniqueLock lock(univ_.mutex);
		univ_.swap(univCopy);
	}

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
		{LUA_MATHLIBNAME, luaopen_math},
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

	ScriptTools::Engine scriptEngine;
	PlayerCodeMap codesMap; //Donné non partagée entre thread

	//! @todo: remplacer openlibs par luaL_openlibs qui semble faire pareil
	openlibs(scriptEngine.state());
	//luaL_openlibs(scriptEngine.state());
	//luaopen_base(scriptEngine.state());
	//luaopen_package(scriptEngine.state());
	//luaopen_string(scriptEngine.state());
	//luaopen_table(scriptEngine.state());
	//luaopen_math(scriptEngine.state());
	//luaopen_io(scriptEngine.state());
	//luaopen_os(scriptEngine.state());
	//luaopen_debug(scriptEngine.state());

	initDroneWars(scriptEngine);

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
				registerCode(scriptEngine, fleetsCode, events),
				registerCode(scriptEngine, planetsCode, events)
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
		bool noWait = true;
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
				round(scriptEngine, codesMap, events);
				events.clear();
				newUpdate += RoundSecond;
				gcCounter += 1;
				if((gcCounter % 1) == 0)
				{
					std::cout << "GC : " << lua_gc(scriptEngine.state(), LUA_GCCOUNT, 0);
					lua_gc(scriptEngine.state(), LUA_GCCOLLECT, 0);
					std::cout << " -> " << lua_gc(scriptEngine.state(), LUA_GCCOUNT, 0) << std::endl;
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

	bool canSave = savingThread_.joinable() == false;
	if(canSave == false)
		canSave = savingThread_.try_join_for(boost::chrono::milliseconds(10));

	if(canSave)
	{
		LOG4CPLUS_TRACE(logger, "copy universe to save");
		SharedLock lockAll(univ_.mutex);
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

void Simulation::createMainPlanet(Player::ID pid)
{
	UniqueLock lock(planetToCreateMutex_);
	planetToCreate_.push(pid);
}
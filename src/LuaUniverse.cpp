//! @file
//! @author Loïc HAMOT
//! @todo: tous ces runtime_error sont surement à évité car sont envoyé à lua

#include "stdafx.h"
#include "LuaUniverse.h"

#include "Model.h"
#include <boost/format.hpp>

#include "Polua/Class.h"
#include "Polua/RegFunc.h"
#include "Polua/Indexer/boost_array.h"
#include "Polua/Indexer/std_vector.h"
#include "Polua/Ref.h"

#include "UnivManip.h"
#include "Skills.h"
#include "PTreeLuaHelper.h"
#include "fighting.h"
#include "Rules.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace BL = boost::locale;
using namespace boost;


//! Extrait la quantité d'une ressource donnée, dans un RessourceSet
size_t getRessource(RessourceSet const& ress, size_t i)
{
	return ress.tab.at(i);
}


//! Génère une direction aléatoirement
Direction directionRandom()
{
	auto randDir = [] {return Direction::Value((rand() % 3) - 1);};
	return Direction(randDir(), randDir(), randDir());
}


//! Retourn la direction pour aller de ori vers targ
Direction directionFromTo(Coord const& ori, Coord const& targ)
{
	// Ramene un entier dans l'intervale [-1: 1]
	auto toOne = [](Coord::Value val) -> Coord::Value
	{
		if(val < 0)
			return -1;
		else if(val > 0)
			return 1;
		else
			return 0;
	};
	Direction target;
	target.X += toOne(targ.X - ori.X);
	target.Y += toOne(targ.Y - ori.Y);
	target.Z += toOne(targ.Z - ori.Z);
	return target;
}


//! true si la planète est libre
//! @todo: remplacable par Planet::isFree
bool planetIsFree(Planet const& planet)
{
	return planet.isFree();
}


//! Crée un PlanetAction pour construir le building demandé
PlanetAction makeBuilding(Building::Enum building)
{
	return PlanetAction(PlanetAction::Building, Building::Enum(building - 1));
}


//! Crée un PlanetAction pour construir le vaisseau demandé
PlanetAction makeShip(Ship::Enum ship)
{
	return PlanetAction(PlanetAction::Ship, Ship::Enum(ship - 1), 1);
}


//! Crée un PlanetAction pour construir le canon demandé
PlanetAction makeCannon(Cannon::Enum cannon)
{
	return PlanetAction(PlanetAction::Cannon, Cannon::Enum(cannon - 1), 1);
}


//! Crée un PlanetAction qui demande de ne rien faire
PlanetAction noPlanetAction()
{
	return PlanetAction();
}


//! Retourne le prix d'un vaisseau donné
//! @throw std::runtime_error si ship n'est pas compris dans [1; Ship::Count]
RessourceSet shipPrice(Ship::Enum ship)
{
	if(ship < 1 || ship > Ship::Count)
		BOOST_THROW_EXCEPTION(std::runtime_error(str(format(BL::gettext(
		                        "%1% expect a number in the range [1: %2%]")) %
		                      "shipPrice" %
		                      Ship::Count)));
	return Ship::List[ship - 1].price;
}


//! Retourne le prix d'un canon donné
//! @throw std::runtime_error si cannon n'est pas dans [1; Cannon::Count]
RessourceSet cannonPrice(Cannon::Enum cannon)
{
	if(cannon < 1 || cannon > Cannon::Count)
		BOOST_THROW_EXCEPTION(std::runtime_error(str(format(BL::gettext(
		                        "%1% expect a number in the range [1: %2%]")) %
		                      "cannonPrice" %
		                      Cannon::Count)));
	return Cannon::List[cannon - 1].price;
}


//! Retourne le prix d'un Building donné, à un niveau donné
//! @throw std::runtime_error si building n'est pas dans [1; Building::Count]
//! @throw std::runtime_error si level < 1
RessourceSet buildingPrice(Building::Enum building, size_t level)
{
	if(building < 1 || building > Building::Count)
	{
		std::string message =
		  str(format(BL::gettext(
		               "%1% expect a building number in the range [1: %2%]")) %
		      "buildingPrice" % Building::Count);
		BOOST_THROW_EXCEPTION(std::runtime_error(message));
	}
	if(level < 1)
		BOOST_THROW_EXCEPTION(
		  std::runtime_error("buildingPrice expect a level greater than 1"));
	return getBuilingPrice(Building::Enum(building - 1), level);
}

//! lua_CFunction qui pousse l'age d'une flotte/planet sur la pile lua
template<typename T>
int luaCFunction_age(lua_State* L)
{
	Polua::object playerObj = Polua::refFromName(L, "currentPlayer");
	Polua::object roundObj = Polua::refFromName(L, "roundIndex");
	Player currentPlayer = playerObj->get<Player>();
	size_t round = roundObj->get<size_t>();
	T const& fleetOrPlanet = Polua::fromstackAny<T>(L, -1);
	if(currentPlayer.skilltab[Skill::Chronos] < 1 ||
	   fleetOrPlanet.playerId != currentPlayer.id)
		lua_pushnil(L);
	else
		Polua::push(L, round - fleetOrPlanet.firstRound);
	return 1;
}

//! luaCFunction qui renvoie forcement true
int luaCFunction_true(lua_State* L)
{
	lua_pushboolean(L, 1);
	return 1;
}

//! luaCFunction qui renvoie forcement false
int luaCFunction_false(lua_State* L)
{
	lua_pushboolean(L, 0);
	return 1;
}

//! luaCFunction ajoute un message de l'utilisateur dans la global "logger"
int luaCFunction_log(lua_State* L)
{
	std::string newMessage = Polua::fromstack<std::string>(L, -1);
	Polua::object logger = Polua::refFromName(L, "logger");
	std::string const oldText =
	  logger->is_valid() ? logger->get<std::string>() : std::string("");
	lua_Debug ar;
	lua_getstack(L, 1, &ar);
	lua_getinfo(L, "nSl", &ar);
	int const line = ar.currentline;
	std::string const newText =
	  oldText + "__LINE__ " + LEXICAL_CAST(std::string, line) +
	  " - " + newMessage + "\n";
	Polua::push(L, newText);
	lua_setglobal(L, "logger");
	return 0;
}

int luaCFunction_simul_fight(lua_State* L)
{
	//Recupération des arguments
	Fleet* playerFleet = Polua::fromstackAny<Fleet*>(L, -3);
	Planet* planetOri = lua_type(L, -2) == LUA_TNIL ?
	                    nullptr :
	                    Polua::fromstackAny<Planet*>(L, -2);
	std::vector<Fleet>& fleetVectOri =
	  Polua::fromstackAny<std::vector<Fleet>&>(L, -1);
	if(playerFleet->player == nullptr)
		BOOST_THROW_EXCEPTION(
		  std::logic_error("playerFleet->player == nullptr"));

	//Création de la liste des joueurs
	struct PlayerComp
	{
		bool operator()(Player const& a, Player const& b)
		{
			return a.id < b.id;
		}
	};
	std::set<Player, PlayerComp> playerSet;
	playerSet.insert(*playerFleet->player);
	if(planetOri && planetOri->player)
		playerSet.insert(*planetOri->player);
	for(Fleet const& fleet : fleetVectOri)
		playerSet.insert(*fleet.player);

	//Création de la PlayersFightingMap
	PlayersFightingMap playerFighting;
	bool willFight = false;
	for(Player const& player : playerSet)
	{
		for(Player const& player2 : playerSet)
		{
			if(player.id != player2.id)
			{
				bool const areEnemy =
				  player.alliance == nullptr ||
				  player2.alliance == nullptr ||
				  player.alliance->id != player2.alliance->id;
				playerFighting[std::make_pair(player.id, player2.id)] = areEnemy;
				willFight = willFight || areEnemy;
			}
		}
	}
	if(willFight == false)
	{
		lua_pushnumber(L, 100);
		return 1;
	}

	//Simulations N fois
	size_t const simulCount = playerFightSimulationCount(*playerFleet->player);
	size_t sum = 0;
	for(size_t i = 0; i < simulCount; ++i)
	{
		//Copie des arguments pour ne pas que la simulation ne les modifie
		std::vector<Fleet> fleetVectCpy;
		std::vector<Fleet*> fleetPtrVect;
		fleetVectCpy.reserve(fleetVectOri.size() + 1);
		for(Fleet const& fleet : fleetVectOri)
		{
			fleetVectCpy.emplace_back(Fleet(fleet, DontCopyMemory));
			fleetPtrVect.push_back(&fleetVectCpy.back());
		}
		fleetVectCpy.push_back(*playerFleet);
		fleetPtrVect.push_back(&fleetVectCpy.back());
		Planet planetCpy = planetOri ? *planetOri : Planet();

		//Lancement de la simualtion
		FightReport report;
		PlayerCodeMap codesMap;
		fight(fleetPtrVect,
		      playerFighting,
		      planetOri ? &planetCpy : nullptr,
		      codesMap,
		      report);
		sum += report.fleetList.back().isDead ? 0 : 1;
	}
	lua_pushnumber(L, static_cast<lua_Number>((sum * 100.) / simulCount));
	return 1;
}

int coord_tostring(lua_State* L)
{
	Coord const& coord = Polua::fromstackAny<Coord>(L, 1);
	std::string const& path =
	  str(boost::format("Coord(X=%1%, Y=%2%, Z=%3%)") %
	      int(coord.X) % int(coord.Y) % int(coord.Z));
	lua_pushstring(L, path.data());
	return 1;
}

int planetAction_tostring(lua_State* L)
{
	PlanetAction const& action = Polua::fromstackAny<PlanetAction>(L, 1);
	std::string const& path =
	  str(boost::format(
	        "PlanetAction(action=%1%, building=%2%, ship=%3%, cannon=%4%)") %
	      action.action % action.building % action.ship % action.cannon);
	lua_pushstring(L, path.data());
	return 1;
}

int fleetAction_tostring(lua_State* L)
{
	FleetAction const& action = Polua::fromstackAny<FleetAction>(L, 1);
	std::string const& path =
	  str(boost::format("FleetAction(action=%1%)") % action.action);
	lua_pushstring(L, path.data());
	return 1;
}

int initDroneWars(LuaTools::Engine& engine)
{
	using namespace Polua;
	lua_State* L = engine.state();

	//! @todo: ne plus avoir besoin de ca
	Class<std::vector<Fleet> >(L, "FleetVector");
	Class<Planet::BuildingTab>(L, "");
	Class<Planet::CannonTab>(L, "");
	Class<ShipTab>(L, "");

	regFunc(L, "shipPrice", shipPrice);
	regFunc(L, "cannonPrice", cannonPrice);
	regFunc(L, "buildingPrice", buildingPrice);
	regFunc(L, "directionRandom", directionRandom);
	regFunc(L, "directionFromTo", directionFromTo);
	regFunc(L, "makeBuilding", makeBuilding);
	regFunc(L, "makeShip", makeShip);
	regFunc(L, "makeCannon", makeCannon);
	regFunc(L, "noPlanetAction", noPlanetAction);
	regFunc(L, "log", luaCFunction_log);
	regFunc(L, "print", luaCFunction_log);
	regFunc(L, "simulates", luaCFunction_simul_fight);
	Class<Ressource>(L, "Ressource")
	.enumValue("Metal", Ressource::Metal)
	.enumValue("Carbon", Ressource::Carbon)
	.enumValue("Loicium", Ressource::Loicium)
	;
	Class<RessourceSet>(L, "RessourceSet")
	.ctor<Ressource::Value, Ressource::Value, Ressource::Value>()
	.ctor()
	.opEqual()
	.methode("contains", &canPay)
	.methode("at", &getRessource)
	;
	Class<Coord>(L, "Coord")
	.ctor()
	.ctor<Coord::Value, Coord::Value, Coord::Value>()
	.ctor<Coord>()
	.opEqual()
	.property("X", &Coord::X)
	.property("Y", &Coord::Y)
	.property("Z", &Coord::Z)
	.toString(&coord_tostring)
	;
	Class<Direction>(L, "Direction")
	.ctor()
	.ctor<Direction::Value, Direction::Value, Direction::Value>()
	.ctor<Direction>()
	.opEqual()
	.property("X", &Direction::X)
	.property("Y", &Direction::Y)
	.property("Z", &Direction::Z)
	;
	Class<Building>(L, "Building")
	.enumValue("CommandCenter",     Building::CommandCenter     + 1) //+1 pour correspondre au tableau de batiment (commence à 1 en lua)
	.enumValue("MetalMine",         Building::MetalMine         + 1)
	.enumValue("CarbonMine",        Building::CarbonMine        + 1)
	.enumValue("LoiciumFilter",     Building::LoiciumFilter     + 1)
	.enumValue("Factory",           Building::Factory           + 1)
	.enumValue("Laboratory",        Building::Laboratory        + 1)
	.enumValue("CarbonicCentral",   Building::CarbonicCentral   + 1)
	.enumValue("SolarCentral",      Building::SolarCentral      + 1)
	.enumValue("GeothermicCentral", Building::GeothermicCentral + 1)
	;
	Class<Cannon>(L, "Cannon")
	.enumValue("Cannon1", Cannon::Cannon1 + 1)
	.enumValue("Cannon2", Cannon::Cannon2 + 1)
	.enumValue("Cannon3", Cannon::Cannon3 + 1)
	.enumValue("Cannon4", Cannon::Cannon4 + 1)
	.enumValue("Cannon5", Cannon::Cannon5 + 1)
	.enumValue("Cannon6", Cannon::Cannon6 + 1)
	;
	Class<Planet>(L, "Planet")
	.methode("isFree", &planetIsFree)
	.methode("age", luaCFunction_age<Planet>)
	.methode("is_planet", luaCFunction_true)
	.methode("is_fleet", luaCFunction_false)
	.property("memory", &Planet::memory)
	.read_only("player", &Planet::player)
	.read_only("coord", &Planet::coord)
	.read_only("playerId", &Planet::playerId)
	.read_only("buildingList", &Planet::buildingList)
	.read_only("cannonTab", &Planet::cannonTab)
	.read_only("ressourceSet", &Planet::ressourceSet)
	;
	Class<Ship>(L, "Ship")
	.enumValue("Mosquito",     Ship::Mosquito   + 1)
	.enumValue("Hornet",       Ship::Hornet     + 1)
	.enumValue("Vulture",      Ship::Vulture    + 1)
	.enumValue("Dragon",       Ship::Dragon     + 1)
	.enumValue("Behemoth",     Ship::Behemoth   + 1)
	.enumValue("Azathoth",     Ship::Azathoth   + 1)
	.enumValue("Queen",        Ship::Queen      + 1)
	.enumValue("Cargo",        Ship::Cargo      + 1)
	.enumValue("LargeCargo",   Ship::LargeCargo + 1)
	;
	Class<Fleet>(L, "Fleet")
	.methode("age", luaCFunction_age<Fleet>)
	.methode("is_planet", luaCFunction_false)
	.methode("is_fleet", luaCFunction_true)
	.property("memory", &Fleet::memory)
	.read_only("player", &Fleet::player)
	.read_only("id", &Fleet::id)
	.read_only("playerId", &Fleet::playerId)
	.read_only("coord", &Fleet::coord)
	.read_only("origin", &Fleet::origin)
	.read_only("name", &Fleet::name)
	.read_only("shipList", &Fleet::shipList)
	.read_only("ressourceSet", &Fleet::ressourceSet)
	;
	Class<PlanetAction>(L, "PlanetAction")
	.enumValue("Building", PlanetAction::Building)
	.enumValue("Ship", PlanetAction::Ship)
	.enumValue("Cannon", PlanetAction::Cannon)
	.read_only("action", &PlanetAction::action)
	.toString(&planetAction_tostring)
	;
	Class<FleetAction>(L, "FleetAction")
	.enumValue("Nothing",   FleetAction::Nothing)
	.enumValue("Move",      FleetAction::Move)
	.enumValue("Harvest",   FleetAction::Harvest)
	.enumValue("Colonize",  FleetAction::Colonize)
	.enumValue("Drop",      FleetAction::Drop)
	.ctor<FleetAction::Type, Direction>()
	.ctor<FleetAction::Type>()
	.read_only("action", &FleetAction::action)
	.read_only("target", &FleetAction::target)
	.toString(&fleetAction_tostring)
	;
	Class<Player>(L, "Player")
	.methode("getMaxFleetCount", getMaxFleetCount)
	.methode("getMaxPlanetCount", getMaxPlanetCount)
	.methode("getMaxFleetSize", getMaxFleetSize)
	.read_only("id", &Player::id)
	.read_only("login", &Player::login)
	.read_only("mainPlanet", &Player::mainPlanet)
	.read_only("score", &Player::score)
	.read_only("alliance", &Player::alliance)
	.read_only("planetCount", &Player::planetCount)
	.read_only("fleetCount", &Player::fleetCount)
	;
	Class<TypedPtree>(L, "UserData")
	.ctor()
	.methode("size", &TypedPtree::size)
	.methode("empty", &TypedPtree::empty)
	.methode("clear", &TypedPtree::clear)
	.methode("put_child", &ptree_put_child)
	.methode("add_child", &ptree_add_child)
	.methode("get_child", &ptree_get_child)
	.methode("put", &ptree_put)
	.methode("add", &ptree_add)
	.methode("get", &ptree_get)
	.methode("get_value", &ptree_get_value)
	.methode("put_value", &ptree_put_value)
	.methode("erase", &ptree_erase)
	.methode("full_count", &countPtreeItem)
	.toString(&ptree_tostring)
	;
	Class<TypedPtree::iterator>(L, "ptree_iterator")
	.methode("get_key", &ptree_iter_key)
	.toString(&ptree_iter_tostring)
	;
	Class<std::vector<TypedPtree*> >(L, "MailBox")
	.methode("size", &std::vector<TypedPtree*>::size)
	;
	Class<Alliance>(L, "Alliance")
	.read_only("id",          &Alliance::id)
	.read_only("master",      &Alliance::master)
	.read_only("name",        &Alliance::name)
	.read_only("description", &Alliance::description)
	;
	Class<Event>(L, "Event")
	.read_only("comment",     &Event::comment)
	.read_only("fleetID",     &Event::fleetID)
	.read_only("planetCoord", &Event::planetCoord)
	.read_only("playerID",    &Event::playerID)
	.read_only("time",        &Event::time)
	.read_only("type",        &Event::type)
	.read_only("value",       &Event::value)
	.read_only("value2",      &Event::value2)
	.read_only("viewed",      &Event::viewed)
	.enumValue("FleetCodeError",      Event::FleetCodeError)
	.enumValue("FleetCodeExecError",  Event::FleetCodeExecError)
	.enumValue("PlanetCodeError",     Event::PlanetCodeError)
	.enumValue("PlanetCodeExecError", Event::PlanetCodeExecError)
	.enumValue("Upgraded",            Event::Upgraded)
	.enumValue("ShipMade",            Event::ShipMade)
	.enumValue("PlanetHarvested",     Event::PlanetHarvested)
	.enumValue("FleetWin",            Event::FleetWin)
	.enumValue("FleetDraw",           Event::FleetDraw)
	.enumValue("FleetsGather",        Event::FleetsGather)
	.enumValue("PlanetColonized",     Event::PlanetColonized)
	.enumValue("FleetLose",           Event::FleetLose)
	.enumValue("FleetDrop",           Event::FleetDrop)
	.enumValue("PlanetLose",          Event::PlanetLose)
	.enumValue("PlanetWin",           Event::PlanetWin)
	.enumValue("CannonMade",          Event::CannonMade)
	.enumValue("FightAvoided",        Event::FightAvoided)
	.enumValue("PlayerLog",           Event::PlayerLog)
	.enumValue("Count",               Event::Count)
	;

	return 0;
}
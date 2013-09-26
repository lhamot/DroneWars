//! @file
//! @author Loïc HAMOT
//! @todo: tous ces runtime_error sont surement à évité car sont envoyé à lua

#include "stdafx.h"
#include "LuaUniverse.h"

#include "Model.h"
#pragma warning(push)
#pragma warning(disable: 4189 4100)
#pragma warning(pop)
#include <boost/format.hpp>

#include "Polua/Class.h"
#include "Polua/RegFunc.h"
#include "Polua/Indexer/boost_array.h"
#include "Polua/Indexer/std_vector.h"
#include "Polua/Ref.h"

#include "UnivManip.h"
#include "Skills.h"
#include "PTreeLuaHelper.h"

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

//! lua_CFunction qui pousse l'age d'une flotte sur la pile lua
int luaCFunction_fleetage(lua_State* L)
{
	Polua::object playerObj = Polua::refFromName(L, "currentPlayer");
	Player currentPlayer = playerObj->get<Player>();
	Fleet const& fleet = Polua::fromstackAny<Fleet>(L, -1);
	if(currentPlayer.skilltab[Skill::Chronos] < 1 ||
	   fleet.playerId != currentPlayer.id)
		Polua::push(L, 0);
	else
		Polua::push(L, time(0) - fleet.time);
	return 1;
}

//! lua_CFunction qui pousse l'age d'une planète sur la pile lua
int luaCFunction_planetage(lua_State* L)
{
	Polua::object playerObj = Polua::refFromName(L, "currentPlayer");
	Player const& currentPlayer = playerObj->get<Player const&>();
	Planet const& planet = Polua::fromstackAny<Planet>(L, -1);
	if(currentPlayer.skilltab[Skill::Chronos] < 1 ||
	   planet.playerId != currentPlayer.id)
		Polua::push(L, 0);
	else
		Polua::push(L, time(0) - planet.time);
	return 1;
}


int luaCFunction_true(lua_State* L)
{
	lua_pushboolean(L, 1);
	return 1;
}

int luaCFunction_false(lua_State* L)
{
	lua_pushboolean(L, 0);
	return 1;
}


int initDroneWars(lua_State* L)
{
	using namespace Polua;

	//! @todo: ne plus avoir besoin de ca
	Class<std::vector<Fleet const*> >(L, "FleetVector");
	Class<Planet::BuildingTab>(L, "");
	Class<Planet::CannonTab>(L, "");
	Class<Fleet::ShipTab>(L, "");

	regFunc(L, "shipPrice", shipPrice);
	regFunc(L, "cannonPrice", cannonPrice);
	regFunc(L, "buildingPrice", buildingPrice);
	regFunc(L, "directionRandom", directionRandom);
	regFunc(L, "directionFromTo", directionFromTo);
	regFunc(L, "makeBuilding", makeBuilding);
	regFunc(L, "makeShip", makeShip);
	regFunc(L, "makeCannon", makeCannon);
	regFunc(L, "noPlanetAction", noPlanetAction);
	Class<Ressource>(L, "Ressource")
	.enumValue("Metal", Ressource::Metal)
	.enumValue("Carbon", Ressource::Carbon)
	.enumValue("Loicium", Ressource::Loicium);
	Class<RessourceSet>(L, "RessourceSet")
	.ctor<Ressource::Value, Ressource::Value, Ressource::Value>()
	.ctor()
	.opEqual()
	.methode("contains", &canPay)
	.methode("at", &getRessource);
	Class<Coord>(L, "Coord")
	.ctor()
	.ctor<Coord::Value, Coord::Value, Coord::Value>()
	.opEqual()
	.property("X", &Coord::X)
	.property("Y", &Coord::Y)
	.property("Z", &Coord::Z);
	Class<Direction>(L, "Direction")
	.ctor()
	.ctor<Direction::Value, Direction::Value, Direction::Value>()
	.opEqual()
	.opEqual()
	.property("X", &Direction::X)
	.property("Y", &Direction::Y)
	.property("Z", &Direction::Z);
	Class<Building>(L, "Building")
	.enumValue("CommandCenter",     Building::CommandCenter     + 1) //+1 pour correspondre au tableau de batiment (commence à 1 en lua)
	.enumValue("MetalMine",         Building::MetalMine         + 1)
	.enumValue("CarbonMine",        Building::CarbonMine        + 1)
	.enumValue("LoiciumFilter",     Building::LoiciumFilter     + 1)
	.enumValue("Factory",           Building::Factory           + 1)
	.enumValue("Laboratory",        Building::Laboratory        + 1)
	.enumValue("CarbonicCentral",   Building::CarbonicCentral   + 1)
	.enumValue("SolarCentral",      Building::SolarCentral      + 1)
	.enumValue("GeothermicCentral", Building::GeothermicCentral + 1);
	Class<Cannon>(L, "Cannon")
	.enumValue("Cannon1", Cannon::Cannon1 + 1)
	.enumValue("Cannon2", Cannon::Cannon2 + 1)
	.enumValue("Cannon3", Cannon::Cannon3 + 1)
	.enumValue("Cannon4", Cannon::Cannon4 + 1)
	.enumValue("Cannon5", Cannon::Cannon5 + 1)
	.enumValue("Cannon6", Cannon::Cannon6 + 1);
	Class<Planet>(L, "Planet")
	.methode("isFree", &planetIsFree)
	.methode("age", luaCFunction_planetage)
	.methode("is_planet", luaCFunction_true)
	.methode("is_fleet", luaCFunction_false)
	.property("memory", &Planet::memory)
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
	.enumValue("LargeCargo",   Ship::LargeCargo + 1);
	Class<Fleet>(L, "Fleet")
	.methode("age", luaCFunction_fleetage)
	.methode("is_planet", luaCFunction_false)
	.methode("is_fleet", luaCFunction_true)
	.property("memory", &Fleet::memory)
	.property("id", &Fleet::id)
	.property("playerId", &Fleet::playerId)
	.property("coord", &Fleet::coord)
	.property("origin", &Fleet::origin)
	.property("name", &Fleet::name)
	.property("shipList", &Fleet::shipList)
	.property("ressourceSet", &Fleet::ressourceSet)
	;
	Class<PlanetAction>(L, "PlanetAction")
	.enumValue("Building", PlanetAction::Building)
	.enumValue("Ship",     PlanetAction::Ship)
	.enumValue("Cannon",   PlanetAction::Cannon);
	Class<FleetAction>(L, "FleetAction")
	.enumValue("Nothing",   FleetAction::Nothing)
	.enumValue("Move",      FleetAction::Move)
	.enumValue("Harvest",   FleetAction::Harvest)
	.enumValue("Colonize",  FleetAction::Colonize)
	.enumValue("Drop",      FleetAction::Drop)
	.ctor<FleetAction::Type, Direction>()
	.ctor<FleetAction::Type>()
	.read_only("action", &FleetAction::action)
	.read_only("target", &FleetAction::target);
	Class<Player>(L, "Player")
	.read_only("id", &Player::id)
	.read_only("allianceID", &Player::allianceID)
	.read_only("mainPlanet", &Player::mainPlanet)
	.read_only("score", &Player::score)
	;
	Class<TypedPtree>(L, "userdata")
	.ctor()
	.methode("size", &TypedPtree::size)
	.methode("empty", &TypedPtree::empty)
	.methode("clear", &TypedPtree::clear)
	.methode("get_child", &ptree_get_child)
	.methode("put", &ptree_put)
	.methode("add", &ptree_add)
	.methode("get", &ptree_get)
	.methode("get_value", &ptree_get_value)
	.methode("put_value", &ptree_put_value)
	.methode("full_count", &countPtreeItem)
	.toString(&ptree_tostring)
	;
	Class<TypedPtree::iterator>(L, "ptree_iterator")
	.methode("gey_key", &ptree_iter_key)
	.toString(&ptree_iter_tostring)
	;
	Class<std::vector<TypedPtree*> >(L, "MailBox")
	.methode("size", &std::vector<TypedPtree*>::size)
	;

	return 0;
}
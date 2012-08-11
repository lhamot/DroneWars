#ifndef __BTA_LUA_UNIVERSE__
#define __BTA_LUA_UNIVERSE__

#include "Model.h"
#include <luabind/class.hpp>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/function.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/stl_container_converter.hpp>
#include "Tools.h"


void PlanetActionListPushBack(PlanetActionList& list, PlanetAction const& pa) {list.push_back(pa);}

size_t getRessource(RessourceSet const& ress, size_t i)
{
	return ress.tab.at(i);
}

Coord directionRandom()
{
	Coord target;
	target.X += (rand() % 3) - 1;
	target.Y += (rand() % 3) - 1;
	target.Z += (rand() % 3) - 1;
	return target;
}

Coord::Value toOne(Coord::Value val)
{
	if(val < 0)
		return -1;
	else if(val > 0)
		return 1;
	else
		return 0;
}

Coord directionFromTo(Coord const& ori, Coord const& targ)
{
	Coord target;
	target.X += toOne(targ.X - ori.X);
	target.Y += toOne(targ.Y - ori.Y);
	target.Z += toOne(targ.Z - ori.Z);
	return target;
}


bool planetIsFree(Planet planet)
{
	return planet.playerId == Player::NoId;
}


PlanetAction makeBuilding(Building::Enum building)
{
	return PlanetAction(PlanetAction::Building, Building::Enum(building - 1));
}

PlanetAction makeShip(Ship::Enum ship)
{
	return PlanetAction(PlanetAction::Ship, Ship::Enum(ship - 1), 1);
}

PlanetAction makeCannon(Cannon::Enum cannon)
{
	return PlanetAction(PlanetAction::Cannon, Cannon::Enum(cannon - 1), 1);
}

PlanetAction noPlanetAction()
{
	return PlanetAction();
}

extern "C" int initDroneWars(lua_State* L)
{
	using namespace luabind;

	open(L);

	module(L)
	[
	  def("directionRandom", directionRandom),
	  def("directionFromTo", directionFromTo),
	  def("makeBuilding", makeBuilding),
	  def("makeShip", makeShip),
	  def("makeCannon", makeCannon),
	  def("noPlanetAction", noPlanetAction),
	  class_<Ressource>("Ressource")
	  .enum_("Type")
	  [
	    value("Metal",   Ressource::Metal),
	    value("Carbon",  Ressource::Carbon),
	    value("Loicium", Ressource::Loicium)
	  ],
	  class_<Planet>("Planet")
	  .def("isFree", &planetIsFree)
	  .def_readonly("coord", &Planet::coord)
	  .def_readonly("playerId", &Planet::playerId)
	  .def_readonly("buildingList", &Planet::buildingList)
	  .def_readonly("cannonTab", &Planet::cannonTab)
	  .def_readonly("ressourceSet", &Planet::ressourceSet),
	  class_<RessourceSet>("RessourceSet")
	  .def(constructor<size_t, size_t, size_t>())
	  .def(constructor<>())
	  .def(const_self == other<RessourceSet>())
	  .def("at", getRessource),
	  class_<Coord>("Coord")
	  .def(constructor<>())
	  .def(constructor<Coord::Value, Coord::Value, Coord::Value>())
	  .def_readonly("X", &Coord::X)
	  .def_readonly("Y", &Coord::Y)
	  .def_readonly("Z", &Coord::Z),
	  class_<Building>("Building")
	  .enum_("Type")
	  [
	    value("CommandCenter",     Building::CommandCenter + 1),
	    value("MetalMine",         Building::MetalMine + 1),
	    value("CarbonMine",        Building::CarbonMine + 1),
	    value("CristalMine",       Building::LoiciumFilter + 1),
	    value("Factory",           Building::Factory + 1),
	    value("Laboratory",        Building::Laboratory + 1),
	    value("CarbonicCentral",   Building::CarbonicCentral + 1),
	    value("SolarCentral",      Building::SolarCentral + 1),
	    value("GeothermicCentral", Building::GeothermicCentral + 1)
	  ],
	  class_<Ship>("Ship")
	  .enum_("Type")
	  [
	    value("Mosquito",     Ship::Mosquito + 1),
	    value("Hornet",       Ship::Hornet + 1),
	    value("Vulture",      Ship::Vulture + 1),
	    value("Dragon",       Ship::Dragon + 1),
	    value("Behemoth",     Ship::Behemoth + 1),
	    value("Azathoth",     Ship::Azathoth + 1),
	    value("Queen",        Ship::Queen + 1),
	    value("Cargo",        Ship::Cargo + 1),
	    value("LargeCargo",   Ship::LargeCargo + 1)
	  ],
	  class_<Cannon>("Cannon")
	  .enum_("Type")
	  [
	    value("Cannon1", Cannon::Cannon1 + 1),
	    value("Cannon2", Cannon::Cannon2 + 1),
	    value("Cannon3", Cannon::Cannon3 + 1),
	    value("Cannon4", Cannon::Cannon4 + 1),
	    value("Cannon5", Cannon::Cannon5 + 1),
	    value("Cannon6", Cannon::Cannon6 + 1)
	  ],
	  class_<Fleet>("Fleet")
	  .def_readonly("id", &Fleet::id)
	  .def_readonly("playerId", &Fleet::playerId)
	  .def_readonly("coord", &Fleet::coord)
	  .def_readonly("origine", &Fleet::origine)
	  .def_readonly("name", &Fleet::name)
	  .def_readonly("shipList", &Fleet::shipList)
	  .def_readonly("ressourceSet", &Fleet::ressourceSet),
	  class_<PlanetAction>("PlanetAction")
	  .enum_("Type")
	  [
	    value("Building", PlanetAction::Building),
	    value("Ship",     PlanetAction::Ship),
	    value("Cannon",   PlanetAction::Cannon)
	  ],
	  class_<FleetAction>("FleetAction")
	  .def(constructor<FleetAction::Type, Coord>())
	  .def(constructor<FleetAction::Type>())
	  .def_readonly("action", &FleetAction::action)
	  .def_readonly("target", &FleetAction::target)
	  .enum_("Type")
	  [
	    value("Nothing",   FleetAction::Nothing),
	    value("Move",      FleetAction::Move),
	    value("Harvest",   FleetAction::Harvest),
	    value("Colonize",  FleetAction::Colonize),
	    value("Drop",      FleetAction::Drop)
	  ],

	  //Pour l'instant inutil:
	  class_<Universe>("Universe")
	  .def_readonly("playerMap", &Universe::playerMap)
	  .def_readonly("planetMap", &Universe::planetMap)
	  .def_readonly("fleetMap", &Universe::fleetMap)
	  .enum_("MapSize")
	  [
	    value("X", Universe::MapSizeX),
	    value("Y", Universe::MapSizeY),
	    value("Z", Universe::MapSizeZ)
	  ]
	];

	return 0;
}

#endif //__BTA_LUA_UNIVERSE__
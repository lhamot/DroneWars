#ifndef __BTA_LUA_UNIVERSE__
#define __BTA_LUA_UNIVERSE__

#include "Model.h"
#include <luabind/class.hpp>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/function.hpp>
#include <luabind/operator.hpp>
#include "Tools.h"

size_t findBuilding(Planet::BuildingMap const& buil, Building::Enum type)
{
	auto iter = buil.find(type);
	if(iter == buil.end())
		return 0;
	else
		return iter->second;
}

void PlanetActionListPushBack(PlanetActionList& list, PlanetAction const& pa){list.push_back(pa);}

template<typename V>
typename V::value_type const&
vectorAt(V const& v, size_t i)
{
	return v.at(i);
}

bool planetIsFree(Planet planet)
{
	return planet.playerId == Player::NoId;
}


extern "C" int initDroneWars(lua_State* L)
{
	using namespace luabind;

	//register_optional<Planet>();
	//register_optional<FleetAction>();
	open(L);

	module(L)
	[
		class_<Planet>("Planet")
		.def("is_free", &planetIsFree)
	  .def_readonly("coord", &Planet::coord)
	  .def_readonly("playerId", &Planet::playerId)
	  .def_readonly("buildingMap", &Planet::buildingMap)
	  .def_readonly("ressourceSet", &Planet::ressourceSet),
	  //.def(constructor<Coord>())
	  class_<RessourceSet>("RessourceSet")
	  .def(constructor<size_t, size_t, size_t>())
	  .def(constructor<>())
	  .def(const_self == other<RessourceSet>()),
	  class_<Coord>("Coord")
	  .def(constructor<Coord::Value, Coord::Value, Coord::Value>())
	  .def_readonly("X", &Coord::X)
	  .def_readonly("Y", &Coord::Y)
	  .def_readonly("Z", &Coord::Z),
	  class_<Building>("Building")
	  .enum_("Enum")
	  [
	    value("CommandCenter",     Building::CommandCenter),
	    value("MetalMine",         Building::MetalMine),
	    value("CarbonMine",        Building::CarbonMine),
	    value("CristalMine",       Building::LoiciumFilter),
	    value("Factory",           Building::Factory),
	    value("Laboratory",        Building::Laboratory),
	    value("CarbonicCentral",   Building::CarbonicCentral),
	    value("SolarCentral",      Building::SolarCentral),
	    value("GeothermicCentral", Building::GeothermicCentral)
	  ],
	  class_<Ship>("Ship")
	  .enum_("Enum")
	  [
	    value("Mosquito",     Ship::Mosquito),
	    value("Hornet",       Ship::Hornet),
	    value("Vulture",      Ship::Vulture),
	    value("Dragon",       Ship::Dragon),
	    value("Behemoth",     Ship::Behemoth),
	    value("Apocalyps",    Ship::Apocalyps)
	  ],
	  class_<Planet::BuildingMap>("BuildingMap")
		.def("count", &Planet::BuildingMap::count)
		.def("find", findBuilding),
	  //.def(boost::python::map_indexing_suite<Planet::BuildingMap>())
	  class_<Fleet::ShipTab>("ShipTab")
		.def("size", &Fleet::ShipTab::size)
		.def("at", vectorAt<Fleet::ShipTab>),
	  //.def(boost::python::vector_indexing_suite<Fleet::ShipTab>());
	  class_<Fleet>("Fleet")
	  .def(constructor<Fleet::ID, Player::ID, Coord>())
	  .def_readonly("id", &Fleet::id)
	  .def_readonly("playerId", &Fleet::playerId)
	  .def_readonly("coord", &Fleet::coord)
	  .def_readonly("name", &Fleet::name)
	  .def_readonly("shipList", &Fleet::shipList),
	  class_<Universe>("Universe")
	  .def_readonly("playerMap", &Universe::playerMap)
	  .def_readonly("planetMap", &Universe::planetMap)
	  .def_readonly("fleetMap", &Universe::fleetMap)
	  .enum_("MapSize")
	  [
	    value("X", Universe::MapSizeX),
	    value("Y", Universe::MapSizeY),
	    value("Z", Universe::MapSizeZ)
	  ],
	  class_<PlanetAction>("PlanetAction")
	  .def(constructor<PlanetAction::Type, Building::Enum>())
	  .def(constructor<PlanetAction::Type, Ship::Enum, unsigned int>())
	  .def_readonly("action",   &PlanetAction::action)
	  .def_readonly("building", &PlanetAction::building)
	  .enum_("Type")
	  [
	    value("Building",  PlanetAction::Building),
	    value("Ship",  PlanetAction::Ship)
	  ],
	  class_<PlanetActionList>("PlanetActionList")
	  //.def(boost::python::vector_indexing_suite<PlanetActionList>());
		.def("append", PlanetActionListPushBack),
	  class_<FleetAction>("FleetAction")
	  .def(constructor<FleetAction::Type, Coord>())
	  .def(constructor<FleetAction::Type>())
	  .def_readonly("action",   &FleetAction::action)
	  .def_readonly("target", &FleetAction::target)
	  .enum_("Type")
	  [
	    value("Nothing",   FleetAction::Nothing),
	    value("Move",      FleetAction::Move),
	    value("Harvest",   FleetAction::Harvest),
	    value("Colonize",  FleetAction::Colonize)
	  ]
	];

	return 0;
}

#endif //__BTA_LUA_UNIVERSE__
#ifndef __BTA_PYTHON_UNIVERSE__
#define __BTA_PYTHON_UNIVERSE__


#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4251 4121 4100)
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/scope.hpp>
#pragma warning(pop)

#include "Model.h"

BOOST_PYTHON_MODULE(DroneWars)
{
	using namespace boost::python;

	class_<Coord>("Coord", init<Coord::Value, Coord::Value, Coord::Value>())
	.def_readonly("X", &Coord::X)
	.def_readonly("Y", &Coord::Y)
	.def_readonly("Z", &Coord::Z)
	;

	/*class_<Building>("Building", init<Building::Type>())
		.def_readonly("type", &Building::type)
		.def_readonly("level", &Building::level)
		;*/

	enum_<Building::Type>("Building")
	.value("CommandCenter",     Building::CommandCenter)
	.value("MetalMine",         Building::MetalMine)
	.value("CarbonMine",        Building::CarbonMine)
	.value("CristalMine",       Building::CristalMine)
	.value("Factory",           Building::Factory)
	.value("Laboratory",        Building::Laboratory)
	.value("CarbonicCentral",   Building::CarbonicCentral)
	.value("SolarCentral",      Building::SolarCentral)
	.value("GeothermicCentral", Building::GeothermicCentral)
	;

	class_<Planet::BuildingMap>("BuildingMap")
	//.def("count", &Planet::BuildingSet::count);
	.def(boost::python::map_indexing_suite<Planet::BuildingMap>());
	;

	class_<Planet>("Planet", init<Coord>())
	.def_readonly("coord", &Planet::coord)
	.def_readonly("playerId", &Planet::playerId)
	.def_readonly("buildingMap", &Planet::buildingMap)
	;

	class_<Ship>("Ship", init<Ship::Type>())
	.def_readonly("type", &Ship::type)
	;

	class_<Fleet>("Fleet", init<Fleet::ID, Player::ID, Coord>())
	.def_readonly("id", &Fleet::id)
	.def_readonly("playerId", &Fleet::playerId)
	.def_readonly("coord", &Fleet::coord)
	.def_readonly("name", &Fleet::name)
	.def_readonly("shipList", &Fleet::shipList)
	;

	class_<Universe>("Universe")
	.def_readonly("playerMap", &Universe::playerMap)
	.def_readonly("planetMap", &Universe::planetMap)
	.def_readonly("fleetList", &Universe::fleetList)
	.def_readonly("MapSizeX", &Universe::MapSizeX)
	.def_readonly("MapSizeY", &Universe::MapSizeY)
	.def_readonly("MapSizeZ", &Universe::MapSizeZ)
	//.def_readonly("nextPlayerID", &Universe::nextPlayerID)
	;

	class_<PlanetActionList>("PlanetActionList")
	.def(boost::python::vector_indexing_suite<PlanetActionList>());
	;

	scope outer =
	  class_<PlanetAction>("PlanetAction", init<PlanetAction::Type, Building::Type>())
	  .def_readonly("action",   &PlanetAction::action)
	  .def_readonly("building", &PlanetAction::building)
	  ;

	enum_<PlanetAction::Type>("Type")
	.value("Building",  PlanetAction::Building);
}

#endif //__BTA_PYTHON_UNIVERSE__
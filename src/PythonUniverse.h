#ifndef __BTA_PYTHON_UNIVERSE__
#define __BTA_PYTHON_UNIVERSE__


#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4251 4121 4100)
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
//#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/scope.hpp>
#pragma warning(pop)

#include "PyOptional.h"

#include "Model.h"


BOOST_PYTHON_MODULE(DroneWars)
{
	using namespace boost::python;

	register_optional<Planet>(); 
	//register_optional<FleetAction>(); 

	
	class_<RessourceSet>("RessourceSet", init<size_t, size_t, size_t>())
	.def(init<>())
	.def(self == self)
	.def(self != self)
	;

	class_<Coord>("Coord", init<Coord::Value, Coord::Value, Coord::Value>())
	.def_readonly("X", &Coord::X)
	.def_readonly("Y", &Coord::Y)
	.def_readonly("Z", &Coord::Z)
	;

	/*class_<Building>("Building", init<Building::Type>())
		.def_readonly("type", &Building::type)
		.def_readonly("level", &Building::level)
		;*/

	enum_<Building::Enum>("Building")
	.value("CommandCenter",     Building::CommandCenter)
	.value("MetalMine",         Building::MetalMine)
	.value("CarbonMine",        Building::CarbonMine)
	.value("CristalMine",       Building::LoiciumFilter)
	.value("Factory",           Building::Factory)
	.value("Laboratory",        Building::Laboratory)
	.value("CarbonicCentral",   Building::CarbonicCentral)
	.value("SolarCentral",      Building::SolarCentral)
	.value("GeothermicCentral", Building::GeothermicCentral)
	;

	enum_<Ship::Enum>("Ship")
	.value("Mosquito",     Ship::Mosquito)
	.value("Hornet",       Ship::Hornet)
	.value("Vulture",      Ship::Vulture)
	.value("Dragon",       Ship::Dragon)
	.value("Behemoth",     Ship::Behemoth)
	.value("Apocalyps",    Ship::Apocalyps)
	;

	class_<Planet::BuildingMap>("BuildingMap")
	//.def("count", &Planet::BuildingSet::count);
	.def(boost::python::map_indexing_suite<Planet::BuildingMap>())
	;

	class_<Planet>("Planet", init<Coord>())
	.def("is_free", planetIsFree)
	.def_readonly("coord", &Planet::coord)
	.def_readonly("playerId", &Planet::playerId)
	.def_readonly("buildingMap", &Planet::buildingMap)
	.def_readonly("ressourceSet", &Planet::ressourceSet)
	;

	//class_<Ship>("Ship", init<Ship::Enum>())
	//.def_readonly("type", &Ship::Enum)
	//;

	class_<Fleet::ShipTab>("ShipTab")
	.def(boost::python::vector_indexing_suite<Fleet::ShipTab>());
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
	.def_readonly("fleetMap", &Universe::fleetMap)
	.def_readonly("MapSizeX", &Universe::MapSizeX)
	.def_readonly("MapSizeY", &Universe::MapSizeY)
	.def_readonly("MapSizeZ", &Universe::MapSizeZ)
	//.def_readonly("nextPlayerID", &Universe::nextPlayerID)
	;

	{
		scope outer =
		  class_<PlanetAction>("PlanetAction", init<PlanetAction::Type, Building::Enum>())
		  .def(init<PlanetAction::Type, Ship::Enum, size_t>())
		  .def_readonly("action",   &PlanetAction::action)
		  .def_readonly("building", &PlanetAction::building)
		  ;

		enum_<PlanetAction::Type>("Type")
		.value("Building",  PlanetAction::Building)
		.value("Ship",  PlanetAction::Ship);
	}
	class_<PlanetActionList>("PlanetActionList")
	.def(boost::python::vector_indexing_suite<PlanetActionList>());
	;

	{
		scope outer =
		  class_<FleetAction>("FleetAction", init<FleetAction::Type, Coord>())
		  .def(init<FleetAction::Type>())
		  .def_readonly("action",   &FleetAction::action)
		  .def_readonly("target", &FleetAction::target)
		  ;

		enum_<FleetAction::Type>("Type")
		.value("Nothing",      FleetAction::Nothing)
		.value("Move",      FleetAction::Move)
		.value("Harvest",   FleetAction::Harvest)
		.value("Colonize",  FleetAction::Colonize)
		;
	}
}

#endif //__BTA_PYTHON_UNIVERSE__
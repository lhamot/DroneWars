#include "stdafx.h"
#include "TranslationTools.h"
#include <boost/thread/mutex.hpp>


std::string getBuildingName(Building::Enum type)
{
	switch(type)
	{
	case Building::CommandCenter:     return gettext("CommandCenter");
	case Building::MetalMine:         return gettext("MetalMine");
	case Building::CarbonMine:        return gettext("CarbonMine");
	case Building::LoiciumFilter:     return gettext("LoiciumFilter");
	case Building::Factory:           return gettext("Factory");
	case Building::Laboratory:        return gettext("Laboratory");
	case Building::CarbonicCentral:   return gettext("CarbonicCentral");
	case Building::SolarCentral:      return gettext("SolarCentral");
	case Building::GeothermicCentral: return gettext("GeothermicCentral");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown buiding type"));
	static_assert(Building::GeothermicCentral == Building::Count - 1, "Building cases missing");
}


std::string getTaskName(PlanetTask::Enum type)
{
	switch(type)
	{
	case PlanetTask::UpgradeBuilding: return gettext("UpgradeBuilding");
	case PlanetTask::MakeShip:        return gettext("MakeShip");
	case PlanetTask::MakeCannon:      return gettext("MakeCannon");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(PlanetTask::MakeCannon == PlanetTask::Count - 1, "Task cases missing");
}


std::string getTaskName(FleetTask::Enum type)
{
	switch(type)
	{
	case FleetTask::Move:     return gettext("Move");
	case FleetTask::Harvest:  return gettext("Harvest");
	case FleetTask::Colonize: return gettext("Colonize");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(FleetTask::Colonize == FleetTask::Count - 1, "Task cases missing");
}


std::string getShipName(Ship::Enum ship)
{
	switch(ship)
	{
	case Ship::Mosquito:   return gettext("Mosquito");
	case Ship::Hornet:     return gettext("Hornet");
	case Ship::Vulture:    return gettext("Vulture");
	case Ship::Dragon:     return gettext("Dragon");
	case Ship::Behemoth:   return gettext("Behemoth");
	case Ship::Azathoth:   return gettext("Azathoth");
	case Ship::Queen:      return gettext("Queen");
	case Ship::Cargo:      return gettext("Cargo");
	case Ship::LargeCargo: return gettext("LargeCargo");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ship type"));
	static_assert(Ship::LargeCargo == Ship::Count - 1, "Ship cases missing");
}


std::string getRessourceName(Ressource::Enum ress)
{
	switch(ress)
	{
	case Ressource::Metal:   return gettext("Metal");
	case Ressource::Carbon:  return gettext("Carbon");
	case Ressource::Loicium: return gettext("Loicium");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ressource type"));
	static_assert(Ressource::Loicium == Ressource::Count - 1, "Ressource cases missing");
}


std::string getEventName(Event::Type evtype)
{
	switch(evtype)
	{
	case Event::FleetCodeError:      return gettext("FleetCodeError");
	case Event::FleetCodeExecError:  return gettext("FleetCodeExecError");
	case Event::PlanetCodeError:     return gettext("PlanetCodeError");
	case Event::PlanetCodeExecError: return gettext("PlanetCodeExecError");
	case Event::Upgraded:            return gettext("Upgraded");
	case Event::ShipMade:            return gettext("ShipMade");
	case Event::PlanetHarvested:     return gettext("PlanetHarvested");
	case Event::FleetWin:            return gettext("FleetWin");
	case Event::FleetDraw:           return gettext("FleetDraw");
	case Event::FleetsGather:        return gettext("FleetsGather");
	case Event::PlanetColonized:     return gettext("PlanetColonized");
	case Event::FleetLose:           return gettext("FleetLose");
	case Event::FleetDrop:           return gettext("FleetDrop");
	case Event::PlanetLose:          return gettext("PlanetLose");
	case Event::PlanetWin:           return gettext("PlanetWin");
	case Event::CannonMade:          return gettext("CannonMade");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ressource type"));
	static_assert(Event::CannonMade == Event::Count - 1, "Event cases missing");
}


boost::mutex timeToStringMutex;

std::string timeToString(time_t time)
{
	boost::unique_lock<boost::mutex> lock(timeToStringMutex);
	static size_t const BufferSize = 80;
	char buffer[BufferSize];
	struct tm* timeinfo = localtime(&time);
	strftime(buffer, BufferSize, "%Y-%m-%d %H:%M:%S", timeinfo);
	return buffer;
}


std::string getCannonName(Cannon::Enum cannon)
{
	switch(cannon)
	{
	case Cannon::Cannon1: return gettext("Cannon1");
	case Cannon::Cannon2: return gettext("Cannon2");
	case Cannon::Cannon3: return gettext("Cannon3");
	case Cannon::Cannon4: return gettext("Cannon4");
	case Cannon::Cannon5: return gettext("Cannon5");
	case Cannon::Cannon6: return gettext("Cannon6");
	}

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Cannon type"));
	static_assert(Cannon::Cannon6 == Cannon::Count - 1, "Cannon cases missing");
}


std::string getTutoText(size_t tutoLevel)
{
	static char const* tagTab[] =
	{
		gettext("BLOCKLY_TUTO_0"),
		gettext("BLOCKLY_TUTO_1"),
		gettext("BLOCKLY_TUTO_2"),
		gettext("BLOCKLY_TUTO_3"),
		gettext("BLOCKLY_TUTO_4"),
		gettext("BLOCKLY_TUTO_5"),
		gettext("BLOCKLY_TUTO_6"),
		gettext("BLOCKLY_TUTO_7"),
		gettext("BLOCKLY_TUTO_8"),
		gettext("BLOCKLY_TUTO_9"),
		gettext("BLOCKLY_TUTO_10"),
		gettext("BLOCKLY_TUTO_11"),
		gettext("BLOCKLY_TUTO_12"),
		gettext("BLOCKLY_TUTO_13"),
		gettext("BLOCKLY_TUTO_14"),
		gettext("BLOCKLY_TUTO_15"),
		gettext("BLOCKLY_TUTO_16"),
		gettext("BLOCKLY_TUTO_17"),
		gettext("BLOCKLY_TUTO_18"),
		gettext("BLOCKLY_TUTO_19"),
		gettext("BLOCKLY_TUTO_20")
	};
	return tagTab[tutoLevel];
}

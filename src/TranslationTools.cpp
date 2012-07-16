#include "stdafx.h"
#include "TranslationTools.h"
#include <boost/thread/mutex.hpp>

std::string getTaskName(PlanetTask::Enum type)
{
#define BTA_STR(X) case PlanetTask::X: return #X;
	switch(type)
	{
		BTA_STR(UpgradeBuilding)
		BTA_STR(MakeShip)
		BTA_STR(MakeCannon)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(PlanetTask::MakeCannon == PlanetTask::Count - 1, "Task cases missing");
}

std::string getTaskName(FleetTask::Enum type)
{
#define BTA_STR(X) case FleetTask::X: return #X;
	switch(type)
	{
		BTA_STR(Move)
		BTA_STR(Harvest)
		BTA_STR(Colonize)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(FleetTask::Colonize == FleetTask::Count - 1, "Task cases missing");
}

std::string getShipName(Ship::Enum ship)
{
#define BTA_STR(X) case Ship::X: return #X;
	switch(ship)
	{
		BTA_STR(Mosquito)
		BTA_STR(Hornet)
		BTA_STR(Vulture)
		BTA_STR(Dragon)
		BTA_STR(Behemoth)
		BTA_STR(Azathoth)
		BTA_STR(Queen)
		BTA_STR(Cargo)
		BTA_STR(LargeCargo)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ship type"));
	static_assert(Ship::LargeCargo == Ship::Count - 1, "Ship cases missing");
}

std::string getRessourceName(Ressource::Enum ress)
{
#define BTA_STR(X) case Ressource::X: return #X;
	switch(ress)
	{
		BTA_STR(Metal)
		BTA_STR(Carbon)
		BTA_STR(Loicium)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ressource type"));
	static_assert(Ressource::Loicium == Ressource::Count - 1, "Ressource cases missing");
}


std::string getEventName(Event::Type evtype)
{
#define BTA_STR(X) case Event::X: return #X;
	switch(evtype)
	{
		BTA_STR(FleetCodeError)
		BTA_STR(FleetCodeExecError)
		BTA_STR(PlanetCodeError)
		BTA_STR(PlanetCodeExecError)
		BTA_STR(Upgraded)
		BTA_STR(ShipMade)
		BTA_STR(PlanetHarvested)
		BTA_STR(FleetWin)
		BTA_STR(FleetDraw)
		BTA_STR(FleetsGather)
		BTA_STR(PlanetColonized)
		BTA_STR(FleetLose)
		BTA_STR(FleetDrop)
		BTA_STR(PlanetLose)
		BTA_STR(PlanetWin)
		BTA_STR(CannonMade)
	}
#undef BTA_STR

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
#define BTA_STR(X) case Cannon::X: return #X;
	switch(cannon)
	{
		BTA_STR(Cannon1)
		BTA_STR(Cannon2)
		BTA_STR(Cannon3)
		BTA_STR(Cannon4)
		BTA_STR(Cannon5)
		BTA_STR(Cannon6)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Cannon type"));
	static_assert(Cannon::Cannon6 == Cannon::Count - 1, "Cannon cases missing");
}
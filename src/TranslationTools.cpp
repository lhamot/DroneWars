#include "TranslationTools.h"
#include <boost/thread/mutex.hpp>

std::string getTaskName(PlanetTask::Enum type)
{
#define BTA_STR(X) case PlanetTask::X: return #X;
	switch(type)
	{
		BTA_STR(UpgradeBuilding)
		BTA_STR(MakeShip)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(PlanetTask::MakeShip == PlanetTask::Count - 1, "Task cases missing");
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
		BTA_STR(Apocalyps)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ship type"));
	static_assert(Ship::Apocalyps == Ship::Count - 1, "Ship cases missing");
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
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown Ressource type"));
	static_assert(Event::FleetsGather == Event::Count - 1, "Event cases missing");
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
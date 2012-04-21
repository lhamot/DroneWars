#include "TranslationTools.h"


std::string getTaskName(Task::Enum type)
{
#define BTA_STR(X) case Task::X: return #X;
	switch(type)
	{
		BTA_STR(UpgradeBuilding)
		BTA_STR(MakeShip)
	}
#undef BTA_STR

	BOOST_THROW_EXCEPTION(std::logic_error("Unknown task type"));
	static_assert(Task::MakeShip == Task::Count - 1, "Task cases missing");
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
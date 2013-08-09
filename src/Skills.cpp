//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Skills.h"
#include "Rules.h"

#include <boost/locale.hpp>


class DummySkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return skillCurrentLevel + 1;
	}
	virtual std::string effectMessageImpl(Player const&) const
	{
		return std::string();
	}
public:
	explicit DummySkill(std::string const& name): ISkill(name) {}
};

class ConquestSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return skillCurrentLevel + 1;
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("You can create up to {1} planets.")) %
		        getMaxPlanetCount(player)).str();
	}
public:
	ConquestSkill(): ISkill("Conquest") {}
};

class StrategySkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return skillCurrentLevel + 1;
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("You can create up to {1} fleets.")) %
		        getMaxFleetCount(player)).str();
	}
public:
	StrategySkill(): ISkill("Strategy") {}
};

class CohesionSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return skillCurrentLevel + 1;
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your fleets can contains up to {1} ships.")) %
		        getMaxFleetSize(player)).str();
	}
public:
	CohesionSkill(): ISkill("Cohesion") {}
};

std::vector<std::shared_ptr<ISkill> > InitSkills()
{
	std::vector<std::shared_ptr<ISkill> > list;
	list.push_back(std::make_shared<ConquestSkill>());
	list.push_back(std::make_shared<StrategySkill>());
	list.push_back(std::make_shared<CohesionSkill>());
	list.push_back(std::make_shared<DummySkill>("InformationService"));
	list.push_back(std::make_shared<DummySkill>("ServerFarm"));
	list.push_back(std::make_shared<DummySkill>("Chronos"));
	list.push_back(std::make_shared<DummySkill>("Memory"));
	list.push_back(std::make_shared<DummySkill>("Communication"));
	list.push_back(std::make_shared<DummySkill>("Spy"));
	list.push_back(std::make_shared<DummySkill>("BlackBox"));
	list.push_back(std::make_shared<DummySkill>("Log"));
	return list;
};

std::vector<std::shared_ptr<ISkill> > const Skill::List = InitSkills();
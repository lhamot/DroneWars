//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Skills.h"
#include "Rules.h"

#include <boost/locale.hpp>

//! Skill non abstrait de rempacement, pour ceux qui ne sont pas encore codés
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
	//! Constructeur
	//! @param name Nom du skill (avant traduction)
	explicit DummySkill(std::string const& name): ISkill(name) {}
};

//! Compétance Conquest
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

//! Compétance Strategy
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

//! Compétance Cohesion
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

//! Compétance Service d'information
class InfoServiceSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::InformationService] < 1;
	}
	virtual size_t skillCostImpl(size_t //skillCurrentLevel
	                            ) const
	{
		return 1000;
	}
	virtual std::string effectMessageImpl(Player const& //player
	                                     ) const
	{
		using namespace boost::locale;
		return translate("You can filter your messages");
	}
public:
	InfoServiceSkill(): ISkill("InformationService") {}
};

//! Compétance Ferme de serveur
class ServerFarmSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return skillCurrentLevel + 1;
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("You can now store up to {1} events.")) %
		        getMaxEventCount(player)).str();
	}
public:
	ServerFarmSkill(): ISkill("ServerFarm") {}
};

//! Compétance Chronos
class ChronosSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::Chronos] < 1;
	}
	virtual size_t skillCostImpl(size_t //skillCurrentLevel
	                            ) const
	{
		return 1000;
	}
	virtual std::string effectMessageImpl(Player const& //player
	                                     ) const
	{
		using namespace boost::locale;
		return translate("You can use the \"age()\" procedure "
		                 "on your fleets and planets");
	}
public:
	ChronosSkill(): ISkill("Chronos") {}
};

//! Compétance Memoire
class MemorySkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return size_t(pow(2., (int(skillCurrentLevel) - 1) * 0.6) * 4.);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your store up to {1} items in the \"memory\""
		                         " of your fleets and planets")) %
		        playerPtreeSize(player)).str();
	}
public:
	MemorySkill(): ISkill("Memory") {}
};


//! Compétance Emition
class EmissionSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return size_t(pow(skillCurrentLevel + 3, 4));
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your messages can reachs up to {1} cases")) %
		        playerEmissionRange(player)).str();
	}
public:
	EmissionSkill(): ISkill("Emission") {}
};

//! Initialise la liste de skill
std::vector<std::shared_ptr<ISkill> > InitSkills()
{
	std::vector<std::shared_ptr<ISkill> > list;
	list.push_back(std::make_shared<ConquestSkill>());
	list.push_back(std::make_shared<StrategySkill>());
	list.push_back(std::make_shared<CohesionSkill>());
	list.push_back(std::make_shared<InfoServiceSkill>());
	list.push_back(std::make_shared<ServerFarmSkill>());
	list.push_back(std::make_shared<ChronosSkill>());
	list.push_back(std::make_shared<MemorySkill>());
	list.push_back(std::make_shared<EmissionSkill>());
	list.push_back(std::make_shared<DummySkill>("Spy"));
	list.push_back(std::make_shared<DummySkill>("BlackBox"));
	list.push_back(std::make_shared<DummySkill>("Log"));
	return list;
};

std::vector<std::shared_ptr<ISkill> > const Skill::List = InitSkills();
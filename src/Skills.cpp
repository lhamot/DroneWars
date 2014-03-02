//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Skills.h"
#include "Rules.h"

#include <boost/locale.hpp>

size_t powInt(size_t val, size_t power)
{
	return size_t(pow(val, power) + 0.5);
}


//! Skill non abstrait de rempacement, pour ceux qui ne sont pas encore codés
class DummySkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const&) const {return true;};
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel);
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
		return powInt(2, skillCurrentLevel);
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
		return powInt(2, skillCurrentLevel);
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
		return powInt(2, skillCurrentLevel);
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
//! @toto: tout
class InfoServiceSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::InformationService] < 1;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel);
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
		return powInt(2, skillCurrentLevel);
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
	virtual size_t skillCostImpl(size_t skillCurrentLevel
	                            ) const
	{
		return powInt(2, skillCurrentLevel);
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
		return powInt(2, skillCurrentLevel + 1);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your store up to {1} items in the \"memory\""
		                         " of your fleets and planets")) %
		        memoryPtreeSize(player)).str();
	}
public:
	MemorySkill(): ISkill("Memory") {}
};


//! Compétance Portée d'Emition
class EmissionRangeSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(3, skillCurrentLevel + 1);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your messages can reachs up to {1} cases")) %
		        playerEmissionRange(player)).str();
	}
public:
	EmissionRangeSkill() : ISkill("EmissionRange") {}
};


//! Compétance Evasion
class EvasionSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		size_t const level = player.skilltab[Skill::Escape];
		return (format(translate( //xgettext:no-c-format
		                 "A fleet with {1} ships has 50% of probability to escape.")) %
		        (level * level)).str();
	}
public:
	EvasionSkill(): ISkill("Evasion") {}
};


//! Compétance Journal
class LogSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::Log] < 1;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		if(player.skilltab[Skill::Log])
			return translate("You can use the \"log\" "
			                 "function to create custom events");
		else
			return translate("You can't use the \"log\" "
			                 "function to create custom events");
	}
public:
	LogSkill() : ISkill("Log") {}
};


//! Compétance Simulation
//! @toto: tout
class SimulationSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel + 1);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		if(player.skilltab[Skill::Simulation])
			return (format(translate("Each call to \"simulates\" "
			                         "function process {1} simulations")) %
			        playerFightSimulationCount(player)).str();
		else
			return translate(
			         "Each call to \"simulates\" function return false");
	}
public:
	SimulationSkill() : ISkill("Simulation") {}
};

//! Compétance Boite noir
class BlackBoxSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::BlackBox] < 1;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel + 1);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return player.skilltab[Skill::BlackBox] == 0 ?
		       (format(translate("You can't receive fight report from destroyed fleets"))).str() :
		       (format(translate("You receive fight report from destroyed fleets"))).str();
	}

public:
	BlackBoxSkill() : ISkill("BlackBox") {}
};


//! Compétance Débit d'Emition
class EmissionRateSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return powInt(2, skillCurrentLevel + 2);
	}
	virtual std::string effectMessageImpl(Player const& player) const
	{
		using namespace boost::locale;
		return (format(translate("Your messages emit up to {1} values per round")) %
		        emitionPTreeSize(player)).str();
	}
public:
	EmissionRateSkill() : ISkill("EmissionRate") {}
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
	list.push_back(std::make_shared<EmissionRangeSkill>());
	list.push_back(std::make_shared<SimulationSkill>());
	list.push_back(std::make_shared<BlackBoxSkill>());
	list.push_back(std::make_shared<LogSkill>());
	list.push_back(std::make_shared<EvasionSkill>());
	list.push_back(std::make_shared<EmissionRateSkill>());
	return list;
};

std::vector<std::shared_ptr<ISkill> > const Skill::List = InitSkills();
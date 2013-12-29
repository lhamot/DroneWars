//! @file
//! @author Lo�c HAMOT

#include "stdafx.h"
#include "Skills.h"
#include "Rules.h"

#include <boost/locale.hpp>

//! Skill non abstrait de rempacement, pour ceux qui ne sont pas encore cod�s
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

//! Comp�tance Conquest
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

//! Comp�tance Strategy
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

//! Comp�tance Cohesion
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

//! Comp�tance Service d'information
//! @toto: tout
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

//! Comp�tance Ferme de serveur
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

//! Comp�tance Chronos
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

//! Comp�tance Memoire
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


//! Comp�tance Emition
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


//! Comp�tance Evasion
class EvasionSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& //player
	                           ) const
	{
		return true;
	}
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const
	{
		return size_t(pow(skillCurrentLevel + 1, 5));
	}
	virtual std::string effectMessageImpl(Player const&) const
	{
		return boost::locale::translate(
		         "More you upgrade, better is your chance of evasion");
	}
public:
	EvasionSkill(): ISkill("Evasion") {}
};


//! Comp�tance Journal
class LogSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::Log] < 1;
	}
	virtual size_t skillCostImpl(size_t //skillCurrentLevel
	                            ) const
	{
		return 1000;
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


//! Comp�tance Simulation
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
		return size_t(pow(skillCurrentLevel + 1, 2));
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

//! Comp�tance Boite noir
class BlackBoxSkill : public ISkill
{
	virtual bool canUpgradeImpl(Player const& player) const
	{
		return player.skilltab[Skill::BlackBox] < 1;
	}
	virtual size_t skillCostImpl(size_t //skillCurrentLevel
	                            ) const
	{
		return 10;
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
	list.push_back(std::make_shared<SimulationSkill>());
	list.push_back(std::make_shared<BlackBoxSkill>());
	list.push_back(std::make_shared<LogSkill>());
	list.push_back(std::make_shared<EvasionSkill>());
	return list;
};

std::vector<std::shared_ptr<ISkill> > const Skill::List = InitSkills();
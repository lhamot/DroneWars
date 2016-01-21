//! @file
//! @author Loïc HAMOT

#ifndef __NDW_SKILLS__
#define __NDW_SKILLS__

#include "Player.h"

//! Interface de tout les skills
class ISkill : boost::noncopyable
{
	std::string const name_; //!< Nom du skill

	//! Décide si un joueur peut upgrader ce skill
	virtual bool canUpgradeImpl(Player const& player) const = 0;
	//! Calcul le cout pour upgrader ce skill
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const = 0;
	//! Génere le message décrivent l'effet actuel du skill
	virtual std::string effectMessageImpl(Player const& player) const = 0;

public:
	//! constructeur
	//! @param name : Nom du skill
	explicit ISkill(std::string const& name): name_(name) {}

	virtual ~ISkill() {};

	//! Retourne le nom du skill
	std::string const& getName() const
	{
		return name_;
	}

	//! Décide si un joueur peut upgrader ce skill (NVI)
	bool canUpgrade(Player const& player) const
	{
		return canUpgradeImpl(player);
	}

	//! Calcul le cout pour upgrader ce skill (NVI)
	size_t skillCost(size_t skillCurrentLevel) const
	{
		return skillCostImpl(skillCurrentLevel);
	}

	//! Génere le message décrivent l'effet actuel du skill (NVI)
	std::string effectMessage(Player const& player) const
	{
		return effectMessageImpl(player);
	}
};


//! Caractéristique des competances
struct Skill
{
	//! Identifiants numerique des competances
	enum Type
	{
		Conquest,
		Strategy,
		Cohesion,
		InformationService,
		ServerFarm,
		Chronos,
		Memory,
		EmissionRange,
		Simulation,
		BlackBox,
		Log,
		Escape,
		EmissionRate
	};

	static size_t const Count = SkillCount; //!< Nombre total de skill
	static std::vector<std::shared_ptr<ISkill> > const List; //!< Liste des skills
};



#endif //__NDW_SKILLS__
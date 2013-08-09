//! @file
//! @author Loïc HAMOT

#ifndef __NDW_SKILLS__
#define __NDW_SKILLS__

#include "stdafx.h"
#include "Player.h"

class ISkill
{
	ISkill(ISkill const&);
	ISkill& operator=(ISkill const&);

	std::string const name_;

	virtual bool canUpgradeImpl(Player const& player) const = 0;
	virtual size_t skillCostImpl(size_t skillCurrentLevel) const = 0;
	virtual std::string effectMessageImpl(Player const& player) const = 0;

public:
	explicit ISkill(std::string const& name): name_(name) {}
	virtual ~ISkill() = 0 {};

	std::string const& getName() const
	{
		return name_;
	}

	bool canUpgrade(Player const& player) const
	{
		return canUpgradeImpl(player);
	}

	size_t skillCost(size_t skillCurrentLevel) const
	{
		return skillCostImpl(skillCurrentLevel);
	}

	std::string effectMessage(Player const& player) const
	{
		return effectMessageImpl(player);
	}
};


//! Caractéristique d'une competance
struct Skill
{
	enum Type
	{
	  Conquest,
	  Strategy,
	  Cohesion,
	  InformationService,
	  ServerFarm,
	  Chronos,
	  Memory,
	  Communication,
	  Spy,
	  BlackBox,
	  Log
	};

	static size_t const Count = SkillCount; //!< Nombre total de skill
	static std::vector<std::shared_ptr<ISkill> > const List; //!< Liste des skills
};



#endif //__NDW_SKILLS__
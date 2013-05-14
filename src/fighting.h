//! @file
//! @author Loïc HAMOT

#ifndef __BTA_FIGHTING__
#define __BTA_FIGHTING__

#include "Model.h"
#include "PlayerCodes.h"

//! @brief Combatant : Pointeur de flotte ou de planète
//!
//! Union réencapsulée d'un pointeur de Fleet et d'un pointeur de Planet
struct FighterPtr
{
private:
	bool isPlanet_; //!< true si l'objet pointe une planète
	union
	{
		Fleet* fleet_;
		Planet* planet_;
	};
public:

	//! Constructeur à partir d'une flotte
	explicit FighterPtr(Fleet* fleet): isPlanet_(false), fleet_(fleet) {};
	//! Constructeur à partir d'une planète
	explicit FighterPtr(Planet* planet): isPlanet_(true), planet_(planet) {};
	//! true si l'objet pointe une planète
	bool isPlanet() const {return isPlanet_;}
	//! @brief Recupère le pointeur de la flotte
	//! @pre l'objet pointe sur une flotte
	Fleet* getFleet() const
	{
		if(isPlanet_)
			BOOST_THROW_EXCEPTION(std::logic_error("It is a planet!!"));
		else
			return fleet_;
	}
	//! @brief Recupère le pointeur de la planète
	//! @pre l'objet pointe sur une planète
	Planet* getPlanet() const
	{
		if(false == isPlanet_)
			BOOST_THROW_EXCEPTION(std::logic_error("It is a fleet!!"));
		else
			return planet_;
	}
};

//! Planet peut etre NULL
void fight(std::vector<Fleet*> const& fleetList,
           Planet* planet,
           PlayerCodeMap& codesMap,
           FightReport& reportList);


#endif //__BTA_FIGHTING__
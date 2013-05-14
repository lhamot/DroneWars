//! @file
//! @author Lo�c HAMOT

#ifndef __BTA_FIGHTING__
#define __BTA_FIGHTING__

#include "Model.h"
#include "PlayerCodes.h"

//! @brief Combatant : Pointeur de flotte ou de plan�te
//!
//! Union r�encapsul�e d'un pointeur de Fleet et d'un pointeur de Planet
struct FighterPtr
{
private:
	bool isPlanet_; //!< true si l'objet pointe une plan�te
	union
	{
		Fleet* fleet_;
		Planet* planet_;
	};
public:

	//! Constructeur � partir d'une flotte
	explicit FighterPtr(Fleet* fleet): isPlanet_(false), fleet_(fleet) {};
	//! Constructeur � partir d'une plan�te
	explicit FighterPtr(Planet* planet): isPlanet_(true), planet_(planet) {};
	//! true si l'objet pointe une plan�te
	bool isPlanet() const {return isPlanet_;}
	//! @brief Recup�re le pointeur de la flotte
	//! @pre l'objet pointe sur une flotte
	Fleet* getFleet() const
	{
		if(isPlanet_)
			BOOST_THROW_EXCEPTION(std::logic_error("It is a planet!!"));
		else
			return fleet_;
	}
	//! @brief Recup�re le pointeur de la plan�te
	//! @pre l'objet pointe sur une plan�te
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
//! @file
//! @author Loïc HAMOT

#ifndef __BTA_FIGHTING__
#define __BTA_FIGHTING__

#include "Model.h"
#include "PlayerCodes.h"


//! @brief Traite une erreur de scripte en ajoutant un Event
inline Event makeCodeErrorEvent(Player::ID pid,
                                CodeData::Target target,
                                size_t codeID,
                                std::string const& message)
{
	return Event(pid,
	             time(0),
	             target == CodeData::Fleet ?
	             Event::FleetCodeError :
	             Event::PlanetCodeError)
	       .setComment(message)
	       .setValue(codeID);
}


//! @brief Traite une erreur de scripte en ajoutant un Event
inline void addErrorMessage(CodeData const& codeData,
                            std::string const& message,
                            std::vector<Event>& events)
{
	events.push_back(
	  makeCodeErrorEvent(
	    codeData.playerId, codeData.target, codeData.id, message));
}


//! @brief Traite une erreur de scripte en ajoutant un Event
inline void addErrorMessage(PlayerCodes::ObjectMap& objMap,
                            std::string const& message,
                            std::vector<Event>& events)
{
	events.push_back(
	  makeCodeErrorEvent(
	    objMap.playerId, objMap.target, objMap.scriptID, message));
	objMap.functions.clear();
}


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

//! Pour chaque pairs de joueur, A attaque t'il B ou non?
typedef std::map<std::pair<Player::ID, Player::ID>, bool> PlayersFightingMap;

//! Planet peut etre NULL
void fight(std::vector<Fleet*> const& fleetList,//!< [in/out] Liste des flottes
           PlayersFightingMap const&,           //!< Qui veut attaquer qui?
           Planet* planet,           //!< [in/out] Planet de la zone (optionel)
           PlayerCodeMap& codesMap,  //!< [in/out] Codes de tout les joueurs
           FightReport& reportList   //!< [in/out] Rapport de combat
          );


#endif //__BTA_FIGHTING__
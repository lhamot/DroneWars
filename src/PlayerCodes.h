//! @file
//! @author Loïc HAMOT

#ifndef __NDW_PLAYERCODES__
#define __NDW_PLAYERCODES__

#include "stdafx.h"
#include <luabind/object.hpp>


//! Contient les codes d'un joueurs (préchangé dans le moteur lua)
struct PlayerCodes
{
	//! @brief Scripts préchargés pour planète ou flottes, plus infos divers
	//! @remark: Si une clef n'est pas trouvé dans une ObjectMap,
	//! ca veut dire que le code à été invalidé
	struct ObjectMap
	{
		//! Tableau assosiatif [Nom de fonction] => luabind::object
		typedef std::map<std::string, luabind::object> Map;
		size_t scriptID;         //!< ID su script
		Player::ID playerId;     //!< ID du joueur
		CodeData::Target target; //!< Planète ou Flotte?
		Map functions;           //!< Tab [Nom de fonction] => luabind::object

		//! Constructeur
		ObjectMap(): scriptID(0), playerId(0), target(CodeData::Undefined) {}
	};
	ObjectMap fleetsCode;  //!< Fonctions pour les flottes d'un joueur
	ObjectMap planetsCode; //!< Fonctions pour les planètes d'un joueur
};
//! Tableau associatif [Player::ID] => PlayerCodes
typedef std::unordered_map<Player::ID, PlayerCodes> PlayerCodeMap;
//! Tab asso [Coord] => Fleets, authaurisant plusieur flotte pour la mème coord
typedef std::unordered_multimap<Coord, Fleet> FleetCoordMap;


#endif //__NDW_PLAYERCODES__
#ifndef __NDW_PLAYERCODES__
#define __NDW_PLAYERCODES__

#include "stdafx.h"
#include <luabind/object.hpp>


//! @brief Contient les codes des joueurs (pr�chang� dans le moteur lua)
struct PlayerCodes
{
	//Si une clef n'est pas trouv� dans une ObjectMap, ca veut dire que le code
	// � �t� invalid�
	struct ObjectMap
	{
		typedef std::map<std::string, luabind::object> Map;
		size_t scriptID;
		Player::ID playerId;
		CodeData::Target target;
		Map functions;

		ObjectMap(): scriptID(0), playerId(0), target(CodeData::Undefined) {}
	};
	ObjectMap fleetsCode;
	ObjectMap planetsCode;
};
typedef std::unordered_map<Player::ID, PlayerCodes> PlayerCodeMap;
typedef std::unordered_multimap<Coord, Fleet> FleetCoordMap;


#endif //__NDW_PLAYERCODES__
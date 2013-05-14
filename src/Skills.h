//! @file
//! @author Loïc HAMOT

#ifndef __NDW_SKILLS__
#define __NDW_SKILLS__

#include "stdafx.h"

//! Caractéristique d'une competance
struct Skill
{
	std::string name;               //!< Nom de code du skill
	static size_t const Count = 11; //!< Nombre total de skill
	static Skill const List[Count]; //!< Liste des skills
};


#endif //__NDW_SKILLS__
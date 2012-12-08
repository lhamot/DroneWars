#ifndef __BTA_RULES__
#define __BTA_RULES__

#include "Model.h"

inline void onPlanetLose(Coord planetCoord, Universe& univ)
{
	Planet& planet = univ.planetMap[planetCoord];
	planet.playerId = Player::NoId;
	planet.buildingList.assign(planet.buildingList.size(), 0);
	planet.eventList.clear();
	planet.taskQueue.clear();
}


void getBlocklyHTML(size_t tutoLevel, std::string const& codecontext, std::ostream& out);


//! Verifie si il faut augrmenter le niveau du tutoriel
void checkTutos(Universe& univ_, boost::upgrade_lock<Universe::Mutex>& lock, std::vector<Signal>& signals);


#endif //__BTA_RULES__
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


#endif //__BTA_RULES__
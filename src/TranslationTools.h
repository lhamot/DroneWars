#ifndef __BTA_TRANSLATION_TOOLS__
#define __BTA_TRANSLATION_TOOLS__


#include "Model.h"

std::string getBuildingName(Building::Enum type);

std::string getTaskName(FleetTask::Enum type);

std::string getTaskName(PlanetTask::Enum type);

std::string getShipName(Ship::Enum ship);

std::string getCannonName(Cannon::Enum cannon);

std::string getRessourceName(Ressource::Enum ress);

std::string getEventName(Event::Type evtype);

std::string timeToString(time_t time);

std::string getTutoText(size_t tutoLevel);

#endif //__BTA_TRANSLATION_TOOLS__
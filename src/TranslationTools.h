#ifndef __BTA_TRANSLATION_TOOLS__
#define __BTA_TRANSLATION_TOOLS__


#include "Model.h"

std::string getTaskName(FleetTask::Enum type);

std::string getTaskName(PlanetTask::Enum type);

std::string getShipName(Ship::Enum ship);

std::string getRessourceName(Ressource::Enum ress);

std::string getEventName(Event::Type evtype);

std::string timeToString(time_t time);

#endif //__BTA_TRANSLATION_TOOLS__
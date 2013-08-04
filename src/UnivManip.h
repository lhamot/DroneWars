#ifndef __NDW_UNIV_MANIP__
#define __NDW_UNIV_MANIP__

#include "Model.h"

class DataBase;

void construct(Universe& univ, DataBase& database);

RessourceSet getBuilingPrice(Building::Enum id, size_t level);

void getNewPlayerCode(std::vector<std::string>& codes);

Coord createMainPlanet(Universe& univ, Player::ID pid);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream_v2(std::istream& in, Universe& univ);

bool canBuild(Planet const& planet, Ship::Enum type, size_t number);

bool canBuild(Planet const& planet, Building::Enum type);

void addTask(Planet& planet, uint32_t roundCount, Building::Enum building);

void addTask(Planet& planet,
             uint32_t roundCount,
             Ship::Enum ship,
             uint32_t number);

bool canBuild(Planet const& planet, Cannon::Enum type, size_t number);

void addTask(Planet& planet,
             uint32_t roundCount,
             Cannon::Enum cannon,
             uint32_t number);

bool canStop(Planet const& planet, Building::Enum type);

void stopTask(Planet& planet,
              PlanetTask::Enum tasktype,
              Building::Enum building);

//! Gere l'écoulement du temps sur la planète.
//! Peut modifier la liste dse flotte et des planètes
void planetRound(Universe& univ,
                 Planet& planet,
                 std::vector<Event>& events);

//! Gere l'écoulement du temps sur la flotte.
//! Peut modifier la liste des flottes et des planètes
void fleetRound(Universe& univ,
                Fleet& fleet,
                std::vector<Event>& events,
                std::map<Player::ID, size_t> const& playersPlanetCount);

void gather(Fleet& fleet, Fleet const& otherFleet);

bool canMove(Fleet const& fleet, Coord const& coord);

void addTaskMove(Fleet& fleet, uint32_t roundCount, Coord const& coord);

bool canHarvest(Fleet const& fleet, Planet const& planet);

void addTaskHarvest(Fleet& fleet, uint32_t roundCount, Planet const& planet);

bool canColonize(Fleet const& fleet, Planet const& planet);

void addTaskColonize(Fleet& fleet, uint32_t roundCount, Planet const& planet);

bool canDrop(Fleet const& fleet, Planet const& planet);

void drop(Fleet& fleet, Planet& planet);

bool canPay(RessourceSet const& stock, RessourceSet const& price);


#endif __NDW_UNIV_MANIP__
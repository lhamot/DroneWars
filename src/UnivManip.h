//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef __NDW_UNIV_MANIP__
#define __NDW_UNIV_MANIP__

#include "Model.h"
#include "Rules.h"

class DataBase;

void construct(Universe& univ, DataBase& database);

boost::optional<RessourceSet> getBuilingPrice(Building::Enum id, size_t level);

void getNewPlayerCode(std::vector<std::string>& codes);

Coord createMainPlanet(Universe& univ, Player::ID pid);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream_v2(std::istream& in, Universe& univ);

//! Gere l'écoulement du temps sur la planète.
//! Peut modifier la liste dse flotte et des planètes
void planetRound(Player const& player,
                 Universe& univ,
                 Planet& planet,
                 std::vector<Event>& events);

//! Gere l'écoulement du temps sur la flotte.
//! Peut modifier la liste des flottes et des planètes
void fleetRound(Universe& univ,
                Player const& player,
                Fleet& fleet,
                std::vector<Event>& events);

bool canPay(RessourceSet const& stock, RessourceSet const& price);


BuildTestState canBuild(Planet const& planet, Ship::Enum type);
BuildTestState canBuild(Planet const& planet, Building::Enum type);
BuildTestState canBuild(Planet const& planet, Cannon::Enum type);
bool canStop(Planet const& planet, Building::Enum type);

void addTask(Planet& planet,
             uint32_t roundCount,
             Ship::Enum ship,
             uint32_t number);

void addTask(Planet& planet, uint32_t roundCount, Building::Enum building);

void addTask(Planet& planet,
             uint32_t roundCount,
             Cannon::Enum cannon,
             uint32_t number);

void stopTask(Planet& planet,
              PlanetTask::Enum tasktype,
              Building::Enum building);


FleetActionTest canMove(Fleet const& fleet, Coord const& coord);

FleetActionTest canHarvest(Fleet const& fleet, Planet const* planet);

FleetActionTest canColonize(
  Player const& player,
  Fleet const& fleet,
  Planet const* planet,
  size_t planetCount);

FleetActionTest canDrop(Fleet const& fleet, Planet const* planet);

FleetActionTest canGather(
  Player const& player,
  Fleet const& fleet1,
  Fleet const& fleet2);


void addTaskMove(Fleet& fleet, uint32_t roundCount, Coord const& coord);

void addTaskHarvest(Fleet& fleet, uint32_t roundCount, Planet const& planet);

void addTaskColonize(Fleet& fleet, uint32_t roundCount, Planet const& planet);

void drop(Fleet& fleet, Planet& planet);

void gather(Fleet& fleet, Fleet const& otherFleet);

//! test si la flotte est vide
bool emptyFleet(Fleet const& fleet);

#endif //__NDW_UNIV_MANIP__
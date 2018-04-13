//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __BTA_RULES__
#define __BTA_RULES__

#include "Model.h"

class DataBase;

static size_t const XPPerSkillPoints = 1000;

extern double coefXP;

//! Reset la planète + MaJ des liens de parentés en cas de perte de planète.
void onPlanetLose(Coord planetCoord,
                  Universe& univ,
                  std::map<Player::ID, Player> const& playerMap,
                  std::unordered_map<Coord, Coord>& newParentMap);


//! Test si il faut augmenter le niveau du tutoriel blockly de chaque joueur
void checkTutos(Universe& univ_,
                DataBase& database,
                std::vector<Event> const& events);


//! @brief Test si une flotte est capable de voir une planète locale
//!
//! Par example, si la difference de niveau est trop grande,
//! la flotte ne pourra pas voir la planète.
bool fleetCanSeePlanet(Fleet const& fleet,
                       Planet const& planet,
                       Universe const& univ);


//! Recalcul les scores des joueur et les met a jour dans la base de donnée
void updateScore(Universe const& univ, DataBase& database);

//! Tableau qui associe chaque Player à son ID
typedef std::map<Player::ID, Player> PlayerMap;
//! Calcule l'experience que chaque armés dans un FightReport a gagnées
void calcExperience(PlayerMap const& planerMap, FightReport& report);

//! Nombre maximum de planètes qu'un joueur peut posseder
size_t getMaxPlanetCount(Player const& player);

//! Nombre maximum de flottes qu'un joueur peut posseder
size_t getMaxFleetCount(Player const& player);

//! Nombre maximum de vaisseaux dans la flotte du joueur
size_t getMaxFleetSize(Player const& player);

//! Nombre maximum d'evenements du joueur qui sont concervés
size_t getMaxEventCount(Player const& player);

//! Nombre maximum d'élément acceptés dans les "memory" des flottes et planètes
size_t memoryPtreeSize(Player const& player);

//! Teste si ce TypedPtree est acceptable (nombre l'élément)
bool acceptMemoryPtree(Player const& player, TypedPtree const& pt);

//! Quantité d'emision des message des flotte de ce joueur (nombre d'item)
size_t emitionPTreeSize(Player const& player);

bool acceptEmitionPtree(Player const& player, TypedPtree const& pt);

//! Distance d'emision des message des flotte de ce joueur
size_t playerEmissionRange(Player const& player);

//! Test si une tentative de fuite réussit
bool isEscapeSuccess(double escapeProba);

//! Calcule la probabilité de fuite d'une flotte face a plusieurs flotte
//! @return probabilité de fuite entre 0 et 1
double calcEscapeProba(Player const& player,
                       Fleet const& fighter,
                       Planet const* planet,
                       std::vector<Fleet> const& otherFleets);

//! Test si ce joueur a le droit de logger des messages dans ces scripts
bool playerCanLog(Player const& player);

//! Test si le joueur à le droit de recevoir ces rapport de combat
bool playerCanSeeFightReport(Player const& player);

//! Calcul le nombre de passe effectuées pour chaque simulation de combat
size_t playerFightSimulationCount(Player const& player);

enum class BuildTestState
{
	Ok,
	OtherTaskRunning,
	NotEnoughRessources,
	BadValue,
	FactoryMissing,
	CommendCenterMissing,
	HangarFull,
	Count
};

enum class FleetActionTest
{
	Ok,
	OtherTaskRunning,
	NotEnoughRessources,
	TooFarAway,
	OutOfGalaxy,
	PlanetHasOwner,
	QueenMissing,
	PlanetLimitReached,
	NotYourOwnPlanet,
	FleetLimitReached,
	NoPlanet,
	BadValue,
	Count
};


namespace InternalRules
{
//! Test si la flote de ce joueur peut coloniser cette planète
FleetActionTest canColonize(
  Player const& player,
  Fleet const& fleet,
  Planet const& planet,
  size_t planetCount);

//! Test si les deux flottes peuvent ce réunir
FleetActionTest canGather(
  Player const& player,
  Fleet const& fleet1,
  Fleet const& fleet2);
}




#endif //__BTA_RULES__
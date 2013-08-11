//! @file
//! @author Lo�c HAMOT

#ifndef __BTA_RULES__
#define __BTA_RULES__

#include "Model.h"
#include <boost/range/adaptor/map.hpp>
#include "DataBase.h"

//! Reset la plan�te + MaJ des liens de parent�s en cas de perte de plan�te.
void onPlanetLose(Coord planetCoord,
                  Universe& univ,
                  std::map<Player::ID, Player> const& playerMap,
                  std::unordered_map<Coord, Coord>& newParentMap);


//! Test si il faut augmenter le niveau du tutoriel blockly de chaque joueur
void checkTutos(Universe& univ_,
                DataBase& database,
                std::vector<Event> const& events);


//! @brief Test si une flotte est capable de voir une plan�te locale
//!
//! Par example, si la difference de niveau est trop grande,
//! la flotte ne pourra pas voir la plan�te.
bool fleetCanSeePlanet(Fleet const& fleet,
                       Planet const& planet,
                       Universe const& univ);


//! Recalcul les scores des joueur et les met a jour dans la base de donn�e
void updateScore(Universe const& univ, DataBase& database);

//! Tableau qui associe chaque Player � son ID
typedef std::map<Player::ID, Player> PlayerMap;
//! Calcule l'experience que chaque arm�s dans un FightReport a gagn�es
void calcExperience(PlayerMap const& planerMap, FightReport& report);

size_t getMaxPlanetCount(Player const& player);

size_t getMaxFleetCount(Player const& player);

size_t getMaxFleetSize(Player const& player);

size_t getMaxEventCount(Player const& player);

namespace InternalRules
{

bool canColonize(Player const& player,
                 Fleet const& fleet,
                 Planet const& planet,
                 size_t planetCount);

bool canBuild(Player const& player,
              Planet const& planet,
              Ship::Enum type,
              size_t const playerFleetCount);

bool canGather(Player const& player,
               Fleet const& fleet1,
               Fleet const& fleet2);
}




#endif //__BTA_RULES__
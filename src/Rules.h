#ifndef __BTA_RULES__
#define __BTA_RULES__

#include "Model.h"
#include <boost/range/adaptor/map.hpp>

void onPlanetLose(Coord planetCoord,
                  Universe& univ,
                  std::unordered_map<Coord, Coord>& newParentMap);


void getBlocklyHTML(size_t tutoLevel, std::string const& codecontext, std::ostream& out);


//! Verifie si il faut augrmenter le niveau du tutoriel
void checkTutos(Universe& univ_, DataBase& database, std::vector<Signal>& signals);

bool fleetCanSeePlanet(Fleet const& fleet, Planet const& planet, Universe const& univ);

//! Recalcul le score des joueurs (modifie les joueurs)
void updateScore(Universe& univ);

#endif //__BTA_RULES__
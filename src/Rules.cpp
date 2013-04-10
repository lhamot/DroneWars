#include "stdafx.h"
#include "Rules.h"
#include <boost/format.hpp>

#include "DataBase.h"

using namespace boost;

void onPlanetLose(Coord planetCoord,
                  Universe& univ,
                  std::map<size_t, Player> const& playerMap,
                  std::unordered_map<Coord, Coord>& newParentMap)
{
	Planet& planet = univ.planetMap[planetCoord];
	if(mapFind(playerMap, planet.playerId)->second.mainPlanet != planetCoord)
	{
		planet.playerId = Player::NoId;
		planet.buildingList.assign(0);
		planet.taskQueue.clear();
		newParentMap[planet.coord] = planet.parentCoord;
		planet.parentCoord = planet.coord;
	}
}


typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;
typedef boost::upgrade_lock<Universe::Mutex> UpgradeLock;
typedef boost::upgrade_to_unique_lock<Universe::Mutex> UpToUniqueLock;

void checkTutos(Universe& univ_,
                DataBase& database,
                std::vector<Event> const& events)
{
	SharedLock lockAll(univ_.planetsFleetsReportsmutex);

	std::vector<Player::ID> wisePlayer;
	wisePlayer.reserve(10000);
	std::map<Player::ID, DataBase::PlayerTutoMap> allTutoMap =
	  database.getAllTutoDisplayed();
	std::vector<Player> players = database.getPlayers();
	for(Player & player: players)
	{
		DataBase::PlayerTutoMap& tutoMap = allTutoMap[player.id];
		size_t const plLvl = tutoMap[CoddingLevelTag];
		switch(plLvl)
		{
		case 0: //! Cas 0 : Créer une mine de métal
		{
			Planet const& planet =
			  mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] > 0)
				wisePlayer.push_back(player.id);
			break;
		}
		case 1: //! Cas 1 : Créer fabrique SI mine de métal a 4
		{
			Planet const& planet =
			  mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] >= 4 &&
			   planet.buildingList[Building::Factory] > 0)
				wisePlayer.push_back(player.id);
			break;
		}
		case 2: //! Cas 2: Créer Vaisseau
		{
			for(Event const & ev: events)
			{
				if(ev.playerID == player.id &&
				   ev.type == Event::ShipMade)
				{
					wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 3: //! Cas 3: Créer 3 flottes
		{
			for(Event const & sig: events)
			{
				if(sig.playerID == player.id &&
				   sig.type == Event::ShipMade)
				{
					size_t const count =
					  range::count_if(
					    univ_.fleetMap | boost::adaptors::map_values,
					    bind(&Fleet::playerId, _1) == player.id);
					if(count == 3)
						wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 4: //! Cas 4: Créer 3 flottes composé de 5 mosquito exactement
		{
			for(Event const & sig: events)
			{
				if(sig.playerID == player.id &&
				   sig.type == Event::FleetsGather)
				{
					auto mosqu5 = [&](Fleet const & fleet)
					{
						return fleet.playerId == player.id &&
						       fleet.shipList[Ship::Mosquito] == 5;
					};
					size_t const count =
					  range::count_if(
					    univ_.fleetMap | boost::adaptors::map_values,
					    mosqu5);
					if(count >= 3)
						wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 5: //! Cas 5: Envoyez 6 flottes dans 6 endroit différent
		{
			std::set<Coord, CompCoord> fleetCoords;
			for(Fleet const & fleet: univ_.fleetMap | adaptors::map_values)
			{
				if(fleet.playerId == player.id)
				{
					fleetCoords.insert(fleet.coord);
					if(fleetCoords.size() >= 6)
					{
						wisePlayer.push_back(player.id);
						break;
					}
				}
			}
			break;
		}
		case 6: //! Cas 6: Récoltez ressources
		{
			for(Event const & sig: events)
			{
				if(sig.playerID == player.id &&
				   sig.type == Event::PlanetHarvested)
				{
					wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 7: //! Cas 7: Rapporter ressources
		{
			for(Event const & sig: events)
			{
				if(sig.playerID == player.id &&
				   sig.type == Event::FleetDrop)
				{
					wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 8: //! Cas 8: Colonisation
		{
			for(Event const & sig: events)
			{
				if(sig.playerID == player.id &&
				   sig.type == Event::PlanetColonized)
				{
					wisePlayer.push_back(player.id);
					break;
				}
			}
			break;
		}
		case 9: break;
		case 10: break;
		case 11: break;
		case 12: break;
		case 13: break;
		}
	}

	database.incrementTutoDisplayed(wisePlayer, CoddingLevelTag);
}


bool fleetCanSeePlanet(Fleet const& fleet,
                       Planet const& planet,
                       Universe const&) //univ
{
	if(fleet.playerId == planet.playerId || planet.playerId == Player::NoId)
		return true;

	//uint64_t const score1 = mapFind(univ.playerMap, fleet.playerId)->second.score;
	//uint64_t const score2 = mapFind(univ.playerMap, planet.playerId)->second.score;
	//Bloquage si trop d'équart de niveaux
	//return (score1 * 5) > score2 && (score2 * 5) > score1;
	//TODO : Gerer la difference de score
	return true;
}

void updateScore(Universe& univ, DataBase& database)
{
	std::map<Player::ID, uint64_t> playerScore;

	for(Planet const & pl: univ.planetMap | boost::adaptors::map_values)
	{
		uint64_t score = 0;
		for(size_t type = 0; type < Building::Count; ++type)
			score += Building::List[type].price.tab[0] * pl.buildingList[type];
		for(size_t type = 0; type < Cannon::Count; ++type)
			score += Cannon::List[type].price.tab[0] * pl.cannonTab[type];

		playerScore[pl.playerId] += score;
	}

	for(Fleet const & fleet: univ.fleetMap | boost::adaptors::map_values)
	{
		uint64_t score = 0;
		for(size_t type = 0; type < Ship::Count; ++type)
			score += Ship::List[type].price.tab[0] * fleet.shipList[type];

		playerScore[fleet.playerId] += score;
	}

	database.updateScore(playerScore);
}
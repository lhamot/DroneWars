//! @file
//! @author Lo�c HAMOT

#include "stdafx.h"
#include "Rules.h"
#include <boost/format.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include "DataBase.h"

using namespace boost;


void onPlanetLose(Coord planetCoord,
                  Universe& univ,
                  std::map<Player::ID, Player> const& playerMap,
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


void checkTutos(Universe& univ_,
                DataBase& database,
                std::vector<Event> const& events)
{
	typedef boost::shared_lock<Universe::Mutex> SharedLock;
	SharedLock lockAll(univ_.planetsFleetsReportsmutex);

	std::vector<Player::ID> wisePlayer;
	wisePlayer.reserve(10000);
	std::map<Player::ID, DataBase::PlayerTutoMap> allTutoMap =
	  database.getAllTutoDisplayed();
	std::vector<Player> players = database.getPlayers();
	for(Player & player : players)
	{
		DataBase::PlayerTutoMap& tutoMap = allTutoMap[player.id];
		size_t const plLvl = tutoMap[CoddingLevelTag];
		switch(plLvl)
		{
		case 0: //! Cas 0 : Cr�er une mine de m�tal
		{
			Planet const& planet =
			  mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] > 0)
				wisePlayer.push_back(player.id);
			break;
		}
		case 1: //! Cas 1 : Cr�er fabrique SI mine de m�tal a 4
		{
			Planet const& planet =
			  mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] >= 4 &&
			   planet.buildingList[Building::Factory] > 0)
				wisePlayer.push_back(player.id);
			break;
		}
		case 2: //! Cas 2: Cr�er Vaisseau
		{
			for(Event const & ev : events)
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
		case 3: //! Cas 3: Cr�er 3 flottes
		{
			for(Event const & sig : events)
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
		case 4: //! Cas 4: Cr�er 3 flottes compos� de 5 mosquito exactement
		{
			for(Event const & sig : events)
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
		case 5: //! Cas 5: Envoyez 6 flottes dans 6 endroit diff�rent
		{
			std::set<Coord, CompCoord> fleetCoords;
			for(Fleet const & fleet : univ_.fleetMap | adaptors::map_values)
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
		case 6: //! Cas 6: R�coltez ressources
		{
			for(Event const & sig : events)
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
			for(Event const & sig : events)
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
			for(Event const & sig : events)
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
	//Bloquage si trop d'�quart de niveaux
	//return (score1 * 5) > score2 && (score2 * 5) > score1;
	//! @todo: Gerer la difference de score
	return true;
}


void updateScore(Universe const& univ, DataBase& database)
{
	std::map<Player::ID, uint64_t> playerScore;

	for(Planet const & pl : univ.planetMap | boost::adaptors::map_values)
	{
		uint64_t score = 0;
		for(size_t type = 0; type < Building::Count; ++type)
			score += Building::List[type].price.tab[0] * pl.buildingList[type];
		for(size_t type = 0; type < Cannon::Count; ++type)
			score += Cannon::List[type].price.tab[0] * pl.cannonTab[type];

		playerScore[pl.playerId] += score;
	}

	for(Fleet const & fleet : univ.fleetMap | boost::adaptors::map_values)
	{
		uint64_t score = 0;
		for(size_t type = 0; type < Ship::Count; ++type)
			score += Ship::List[type].price.tab[0] * fleet.shipList[type];

		playerScore[fleet.playerId] += score;
	}

	database.updateScore(playerScore);
}


//! Calcule l'XP qu'un combat entre deux arm�s va rapporter
template<typename A, typename E>
uint32_t calcExp(PlayerMap const& playerMap,
                 Report<A> const& allyReport,
                 Report<E> const& enemyReport)
{
	struct ArmyPrice
	{
		// Donne la valeur de la flotte pour le calcul d'XP
		size_t operator()(Fleet const& army)
		{
			size_t res = 0;
			for(size_t i = 0; i < Ship::Count; ++i)
				res += army.shipList[i] * Ship::List[i].price.tab[0];
			return res;
		}

		// Donne la valeur de la plan�te pour le calcul d'XP
		size_t operator()(Planet const& army)
		{
			size_t res = 0;
			for(size_t i = 0; i < Cannon::Count; ++i)
				res += army.cannonTab[i] * Cannon::List[i].price.tab[0];
			return res;
		}
	} armyPrice;

	bool const isDead = allyReport.isDead;
	bool const enemyIsDead = enemyReport.isDead;
	if(isDead)
		return 1;
	double exp = (enemyIsDead ? 4. : 1.);
	Player const& player = mapFind(playerMap, allyReport.fightInfo.before.playerId)->second;
	Player const& enemy = mapFind(playerMap, enemyReport.fightInfo.before.playerId)->second;
	//std::cout << exp << std::endl;
	exp *= log(double(enemy.experience + 2)) / log(double(player.experience + 2));
	size_t const fleetPrice = armyPrice(enemyReport.fightInfo.before) / 10;
	size_t const enemyFleetPrice = armyPrice(enemyReport.fightInfo.before) / 10;
	//std::cout << exp << std::endl;
	exp *= (enemyFleetPrice + 2) / log(fleetPrice + 2);
	//std::cout << exp << std::endl;
	if(boost::math::isnormal(exp) == false)
		BOOST_THROW_EXCEPTION(std::logic_error("isnormal(exp) == false"));
	return boost::numeric::converter<uint32_t, double>::convert(exp);
}


void calcExperience(PlayerMap const& playerMap,
                    FightReport& report)
{
	//! - Experience de chaque flotte
	for(Report<Fleet>& fleetReport : report.fleetList)
	{
		for(intptr_t enIdx : fleetReport.enemySet)
		{
			if(enIdx == -1)
			{
				if(!report.planet)
					BOOST_THROW_EXCEPTION(std::logic_error("report.planet == NULL!!"));
				if(report.planet->fightInfo.before.playerId == Player::NoId)
					BOOST_THROW_EXCEPTION(std::logic_error("playerId == Player::NoId"));

				fleetReport.experience +=
				  calcExp(playerMap, fleetReport, *report.planet);
			}
			else
				fleetReport.experience +=
				  calcExp(playerMap, fleetReport, report.fleetList[enIdx]);
		}
	}

	//! - Experience de la plan�te
	if(report.planet)
	{
		Report<Planet>& planetReport = *report.planet;
		for(intptr_t enemiIndex : planetReport.enemySet)
		{
			planetReport.experience += calcExp(
			                             playerMap, planetReport, report.fleetList[enemiIndex]);
		}
	}
}
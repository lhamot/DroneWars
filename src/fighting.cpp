//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "fighting.h"
#include "ScriptTools.h"

#pragma warning(push)
#pragma warning(disable: 4724 6385 6294 6201)
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>
#include <boost/random/taus88.hpp>
#include <numeric>
#include <unordered_set>
#pragma warning(pop)

//! C'est une experience
#define UNTIL(exp) while(!(exp))


using namespace std;
using namespace boost::adaptors;
using namespace boost;


//! Données d'un vaisseau ou d'une flotte pendant un combat
struct Unit
{
	//! @brief Type d'unité
	//!
	//! Si type < Ship::Count, type est un Ship::Type
	//! Sinon type - Ship::Count est un Cannon::Type
	uint16_t type;
	uint16_t life;   //!< Point de vie de l'unité
	uint16_t shield; //!< Point de bouclier de l'unité
	int16_t armyIndex = -1;
	Player::ID playerID = Player::NoId;

	//! Constructeur
	Unit(uint16_t type,
	     uint16_t life,
	     uint16_t shield,
	     int16_t armyIndex,
	     Player::ID pid) :
		type(type),
		life(life),
		shield(shield),
		armyIndex(armyIndex),
		playerID(pid)
	{
	}
};

typedef std::vector<Unit> UnitTab; //!< Liste d'unité (cannons ou vaisseaux)

//! Remplit un UnitTab a partir d'une flotte (Fleet)
void fillShipList(
  Fleet const& fleet, int16_t armyIndex, Player::ID pid, UnitTab& shipTab)
{
	for(uint16_t type = 0; type < Ship::Count; ++type)
	{
		Ship const& def = Ship::List[type];
		size_t const count = fleet.shipList[type];
		shipTab.resize(shipTab.size() + count,
		               Unit(type, def.life, def.shield, armyIndex, pid));
	}
}


//! Remplit un UnitTab a partir d'une planète (Planet)
void fillShipList(
  Planet const& planet, int16_t armyIndex, Player::ID pid, UnitTab& shipTab)
{
	for(uint16_t type = 0; type < Cannon::Count; ++type)
	{
		Cannon const& def = Cannon::List[type];
		size_t const count = planet.cannonTab[type];
		uint16_t const unitType = type + Ship::Count;
		shipTab.resize(shipTab.size() + count,
		               Unit(unitType, def.life, def.shield, armyIndex, pid));
	}
}

//! Générateur de nombre pseudo-aléatoire
boost::random::mt19937 gen(static_cast<uint32_t>(time(0)));

//! Recharge les boucliers des unités
void loadShield(UnitTab& unitTab)  throw()
{
	for(Unit& unit : unitTab)
	{
		uint16_t const shieldMax =
		  (unit.type < Ship::Count) ?
		  Ship::List[unit.type].shield :
		  Cannon::List[unit.type - Ship::Count].shield;
		unit.shield = shieldMax;
	}
}

//! Interface de toute les competances de combats
class SkillInterface
{
	//C'est plus pratique de les implementer dans la class mère,
	//car ces methodes ne seront pas forcement reimplementer.
	//! Appelé avant l'attaque
	virtual void beforeAttack_(UnitTab&, UnitTab&) {};

	//! Appelé avant le netoyage des unités mortes
	virtual void beforeCleaning_(UnitTab&, UnitTab&) {};

	//! Appelé avant le rechargement des boucliers
	virtual void beforeReload_(UnitTab&, UnitTab&) {};

public:
	//! Appelé avant l'attaque : Appel beforeAttack_
	void beforeAttack(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeAttack_(ownUnits, theirUnits);
	}
	//! Appelé avant le netoyage des unités mortes : Appel beforeCleaning_
	void beforeCleaning(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeCleaning_(ownUnits, theirUnits);
	}
	//! Appelé avant le rechargement des boucliers : Appel beforeReload_
	void beforeReload(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeReload_(ownUnits, theirUnits);
	}
};


//! Un skill factice
class NoSkill : public SkillInterface
{
};


//! Statistique d'un type d'unité
struct UnitStat
{
	size_t livingCount; //!< Nombre d'unités vivantes
	size_t deadCount;   //!< Nombre d'unités mortes
	size_t meanPV;      //!< Nombre de PV moyen

	UnitStat(): livingCount(0), deadCount(0), meanPV(0) {}
};


//! Statistique de chaque type d'unitées d'une armé
struct ArmyStats
{
	boost::array<UnitStat, Ship::Count> shipStats;     //!< Stat des vaisseaux
	boost::array<UnitStat, Cannon::Count> cannonStats; //!< Stat des canons
};


//! Calcul les stats d'une armé
ArmyStats calcArmyStat(UnitTab const& unitTab)
{
	ArmyStats result;

	auto addUnitInStat = [](Unit const & unit, UnitStat & unitStat)
	{
		if(unit.life <= 0)
			unitStat.deadCount += 1;
		else
		{
			unitStat.livingCount += 1;
			unitStat.meanPV += unit.life;
		}
	};

	for(Unit const& unit : unitTab)
	{
		//! Si c'est un vaisseau
		if(unit.type < Ship::Count)
		{
			size_t const type = unit.type;
			UnitStat& unitStat = result.shipStats[type];
			addUnitInStat(unit, unitStat);
		}
		//! Sinon
		else
		{
			size_t const type = unit.type - Ship::Count;
			UnitStat& unitStat = result.cannonStats[type];
			addUnitInStat(unit, unitStat);
		}
	}
	auto divideMeanByCount = [](UnitStat & stat)
	{
		if(stat.livingCount)
			stat.meanPV /= stat.livingCount;
	};
	boost::for_each(result.shipStats, divideMeanByCount);
	boost::for_each(result.cannonStats, divideMeanByCount);
	return result;
}


namespace std
{
//! Traits de calcul de valeur de hachage d'une coordonée Coord
template<>
struct hash<std::pair<Player::ID, Player::ID> >
{
	//! Calcul la valeur de hachage d'une coordonée Coord
	size_t operator()(std::pair<Player::ID, Player::ID> const& p) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.first);
		boost::hash_combine(seed, p.second);
		return seed;
	}
};
}

typedef std::map<Player::ID, std::set<Player::ID> > PlayersFightingSet;

bool fightAgain(UnitTab const& allUnits,
                PlayersFightingSet const& playerFightPlayer
               )
{
	std::vector<Player::ID> playerSet;
	for(Unit const& unit : allUnits)
	{
		if(boost::range::find(playerSet, unit.playerID) == playerSet.end())
			playerSet.push_back(unit.playerID);
	}

	for(auto iter = playerSet.begin(), end = playerSet.end();
	    iter != end;
	    ++iter)
	{
		Player::ID const playerID1 = *iter;
		for(auto iter2 = boost::next(iter); iter2 != end; ++iter2)
		{
			Player::ID const playerID2 = *iter2;
			auto playerIter = playerFightPlayer.find(playerID1);
			if(playerIter != playerFightPlayer.end())
			{
				if(playerIter->second.count(playerID2))
					return true;
			}
		}
	}
	return false;
}

template<typename S, typename O>
void putEnemyFleetInReport(PlayersFightingSet const& playerFightingSet,
                           Report<S>& report,
                           O const* otherArmy,
                           int16_t otherIndex)
{
	Player::ID const otherPlayer = otherArmy->playerId;
	Player::ID const reportPlayer = report.fightInfo.before.playerId;
	auto iter1 = playerFightingSet.find(reportPlayer);
	if(iter1 != playerFightingSet.end())
	{
		if(iter1->second.count(otherPlayer))
		{
			report.enemySet.insert(otherIndex);
			report.hasFight = true;
		}
	}
};

size_t countUnit(Planet const& planet)
{
	return boost::accumulate(planet.cannonTab, 0);
}

size_t countUnit(Fleet const& fleet)
{
	return boost::accumulate(fleet.shipList, 0);
}

template<typename S>
void fillReport(PlayersFightingSet const& playerFightingSet,
                std::vector<Fleet*> const& fleetList,
                Planet* planet,
                Report<S>& report,
                S const& army)
{
	int16_t otherIndex = 0;
	for(Fleet* otherFleet : fleetList)
	{
		putEnemyFleetInReport(
		  playerFightingSet, report, otherFleet, otherIndex);
		++otherIndex;
	}
	if(planet)
		putEnemyFleetInReport(playerFightingSet, report, planet, -1);
	size_t const shipCount = countUnit(army);
	report.isDead = shipCount == 0;
	report.fightInfo.after = army;
}


void fire(UnitTab const& allUnits,
          std::map<Player::ID, std::vector<Unit*> >& enemyVects)
{
	//! - Pour chaque vaisseau
	for(Unit const& ship : allUnits)
	{
		//! - On calcule sa puissance
		uint16_t power =
		  (ship.type < Ship::Count) ?
		  Ship::List[ship.type].power :
		  Cannon::List[ship.type - Ship::Count].power;

		//! - On trouve un vaisseau énemie
		std::vector<Unit*> const& enemyVect = enemyVects[ship.playerID];
		size_t const enemyCount = enemyVect.size();
		boost::random::uniform_int_distribution<> dist(
		  0, NUMERIC_CAST(int, enemyCount - 1));

		size_t const pos = dist(gen);

		Unit& unity = *enemyVect[pos];
		//! - Si il est moin puissant que bouclier enemi: On diminu le bouclier
		uint16_t& shield = unity.shield;
		if(power < shield)
			shield -= power;
		else
		{
			//! - Sinon:
			//!     On mais le bouclier a zero et on diminue la puissance
			//!     Si l'attaquant est moin puissant que les PV de l'attaqué
			//!         On diminue juste les PV
			//!     Sinon l'attaqué est mort
			power -= shield;
			shield = 0;
			uint16_t& life = unity.life;
			if(power < life)
				life -= power;
			else
				life = 0;
		}
	}
}

void fight(std::vector<Fleet*> const& fleetList,
           PlayersFightingMap const& playerFightPlayer,
           Planet* planet,
           PlayerCodeMap&, //codesMap
           FightReport& reportList)
{
	static int16_t const PlanetIndex = -1;

	if(fleetList.empty())
		return;

	//! Remplissage de la liste des flottes et planete dans reportList
	reportList.fleetList.clear();
	boost::transform(
	  fleetList,
	  back_inserter(reportList.fleetList),
	[](Fleet const * fleetPtr) {return Report<Fleet>(*fleetPtr);});

	if(planet)
	{
		reportList.hasPlanet = true;
		reportList.planet = Report<Planet>(*planet);
	}

	//! Qui vase batre contre qui?
	//! (Si un seul des deux veux se batre, les deux se batrons)
	PlayersFightingSet playerFightingSet;
	// Pour chaque paires de joueurs qui veullent se batre (valeur à true)
	// On l'ajoute dans le playerFightingSet
	for(auto const& playerPair :
	    playerFightPlayer
	    | filtered(bind(&PlayersFightingMap::value_type::second, _1) == true)
	    | map_keys)
	{
		playerFightingSet[playerPair.first].insert(playerPair.second);
		playerFightingSet[playerPair.second].insert(playerPair.first);
	}

	//! On remplit la liste des unités (flottes et planete)
	UnitTab allUnits;
	// comptage des unités pour réservation du UnitTab
	size_t unitCount = 0;
	for(Fleet const* fleet : fleetList)
		unitCount += boost::accumulate(fleet->shipList, 0);
	if(planet)
		unitCount += boost::accumulate(planet->cannonTab, 0);
	allUnits.reserve(unitCount);
	// Remplissage du UnitTab
	for(auto indexFleet :
	    combine(irange(int16_t(0), int16_t(fleetList.size())), fleetList))
	{
		int16_t const index = indexFleet.get<0>();
		Fleet const* fleet = indexFleet.get<1>();
		fillShipList(*fleet, index, fleet->playerId, allUnits);
	}
	if(planet)
		fillShipList(*planet, PlanetIndex, planet->playerId, allUnits);

	//! Pour 5 round maximum
	for(size_t roundCount = 0; roundCount < 5; ++roundCount)
	{
		//! - On recharge les bouclier
		loadShield(allUnits);
		//! - Si il ne reste pas au moins 2 joueurs énemis, on arrete.
		if(fightAgain(allUnits, playerFightingSet) == false)
			break;
		//! - Pour chaque joueur on établis une liste des unités énemis
		std::map<Player::ID, std::vector<Unit*> > enemyVects;
		for(Unit& ship : allUnits)
		{
			for(Player::ID pid : playerFightingSet[ship.playerID])
				enemyVects[pid].push_back(&ship);
		}
		//! - Chaque vaisseau tire (fonction fire)
		fire(allUnits, enemyVects);
		//! - On supprime les vaisseaux detruit
		remove_erase_if(allUnits, bind(&Unit::life, _1) <= 0);
	}

	//! On vide les flottes et planete données en entré
	for(Fleet* fleet : fleetList)
		fleet->shipList.assign(0);
	if(planet)
		planet->cannonTab.assign(0);
	//! On les remplis avec ce qui reste des unités aprés le combat
	for(Unit const& unit : allUnits)
	{
		if(unit.armyIndex == PlanetIndex)
		{
			if(!planet)
				BOOST_THROW_EXCEPTION(std::logic_error("!planet"));
			++planet->cannonTab[unit.type - Ship::Count];
		}
		else
			++fleetList[unit.armyIndex]->shipList[unit.type];
	}
	//! On les copie aussi dans les rapport
	for(auto reportAndFleet : boost::combine(reportList.fleetList, fleetList))
		fillReport(playerFightingSet,
		           fleetList,
		           planet,
		           reportAndFleet.get<0>(),
		           *reportAndFleet.get<1>());
	if(planet)
		fillReport(playerFightingSet,
		           fleetList,
		           planet,
		           *reportList.planet,
		           *planet);
	//! Si une flotte a protégé la planete jusque au bout,
	//!  la planète est conservé.
	std::set<Player::ID> winingPlayers;
	for(Report<Fleet> const& fleetReport : reportList.fleetList)
		if(fleetReport.isDead == false)
			winingPlayers.insert(fleetReport.fightInfo.before.playerId);
	if(planet && winingPlayers.count(planet->playerId))
		reportList.planet->isDead = false;

	if(reportList.fleetList.size() != fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("Bad reports count!!"));
}




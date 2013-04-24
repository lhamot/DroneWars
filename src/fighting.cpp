#include "stdafx.h"
#include "fighting.h"

#include <luabind/object.hpp>
#include <luabind/detail/call_function.hpp>


//! C'est une experience
#define UNTIL(exp) while(!(exp))


using namespace std;

static size_t const PlanetIndex = size_t(-1);

//Reflexion sur les combats
//
// Objectifs:
//   Regles simple
//   Pas de strategie dominante
//   Rapide a executer
//
//
//Mechanique, en vrac
//  Rang avant, moyen , arrière
//  Boulier cachant 1 ou  2 rang
//  Choix de tirer ou non en fonction de la reserve d'energie(coup + fort si energie pleine)
//  Generateurs d'energie(dans vaisseaux?)
//  Ravitaillement des missile(vaisseu de fabrication?)
//  Ravitaillement de ressource(loicium rafiné?)
//  Choix du type d'attaque(par type de vaiseau)
//  Fuite pendant le combat
//
//Avant combat
//
//A chaque round
//  Choix du rang pour chaque type(arière, moyen, avant)
//  Fuire ou pas
//
//Algo round
//  Demander aux IA si si continue
//  Pour chaque type de vaiseau
//    Demander aux IA le rang(malus d'un changement de rang?)
//    Demander aux IA si ils tirent ou attendent
//  Pour chaque type de vaiseau
//    Si il tire
//      Chaque vaiseau tire sur un enemie aléatoire(des 2 premier rang?)(difference de precision?)
struct Unit
{
	uint16_t type;
	uint16_t life;
	uint16_t shield;


	Unit(uint16_t type, uint16_t life, uint16_t shield):
		type(type), life(life), shield(shield)
	{
	}
};

typedef std::vector<Unit> UnitTab;

void fillShipList(Fleet const& fleet, UnitTab& shipTab)
{
	//Contage
	size_t shipNumber = 0;
	for(int i = 0; i < Ship::Count; ++i)
		shipNumber += fleet.shipList[i];

	shipTab.reserve(shipNumber);
	for(Ship::Enum type = Ship::Enum(0);
	    type < Ship::Count;
	    type = Ship::Enum(type + 1))
	{
		Ship const& def = Ship::List[type];
		size_t const count = fleet.shipList[type];
		for(size_t i = 0; i < count; ++i)
			shipTab.push_back(Unit(type, def.life, def.shield));
	}
}

void fillShipList(Planet const& planet, UnitTab& shipTab)
{
	//Contage
	size_t shipNumber = 0;
	for(int i = 0; i < Cannon::Count; ++i)
		shipNumber += planet.cannonTab[i];

	shipTab.reserve(shipNumber);
	for(Cannon::Enum type = Cannon::Enum(0);
	    type < Cannon::Count;
	    type = Cannon::Enum(type + 1))
	{
		Cannon const& def = Cannon::List[type];
		size_t const count = planet.cannonTab[type];
		for(size_t i = 0; i < count; ++i)
			shipTab.push_back(Unit(type + Ship::Count, def.life, def.shield));
	}
}


//! @brief Effectue l'attaque de la flotte unitTab1 contre unitTab2
void applyRound(UnitTab& unitTab1, UnitTab& unitTab2)
{
	size_t pos = 0;
	size_t const size = unitTab2.size();
	//! Pour chaque vaisseau de la flotte unitTab1
	for(Unit & ship : unitTab1)
	{
		//! - On calcule ca puissance
		uint16_t power =
		  (ship.type < Ship::Count) ?
		  Ship::List[ship.type].power :
		  Cannon::List[ship.type - Ship::Count].power;

		//! - Si il est moin puissant que bouclier enemi: On diminu le bouclier
		Unit& unity = unitTab2[pos];
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
			{
				life = 0;
				++pos;
				if(pos == size)
					pos = 0;
			}
		}
	}
}


//! @warning Cette fonction modifie la flotte
void fillFinalFleet(UnitTab const& shipTab, Fleet& fleet) throw()
{
	Fleet::ShipTab& outTab = fleet.shipList;
	outTab.assign(0);
	for(Unit const & ship : shipTab)
		++outTab[ship.type];
}


//! @warning Cette fonction modifie la planet
void fillFinalFleet(UnitTab const& shipTab, Planet& planet) throw()
{
	Planet::CannonTab& outTab = planet.cannonTab;
	//outTab.assign(Cannon::Count, 0);
	outTab.assign(0);
	for(Unit const & ship : shipTab)
		++outTab[ship.type - Ship::Count];
}


//! @brief recharge les boucliers des unités
void loadShield(UnitTab& unitTab)  throw()
{
	for(Unit & unit : unitTab)
	{
		uint16_t const shieldMax =
		  (unit.type < Ship::Count) ?
		  Ship::List[unit.type].shield :
		  Cannon::List[unit.type - Ship::Count].shield;
		unit.shield = shieldMax;
	}
}

class SkillInterface
{
	//C'est plus pratique de les définir dans la class mère,
	//car ces methodes ne seront pas forcement redéfinits.
	virtual void beforeAttack_(UnitTab&, UnitTab&) {};
	virtual void beforeCleaning_(UnitTab&, UnitTab&) {};
	virtual void beforeReload_(UnitTab&, UnitTab&) {};

public:
	void beforeAttack(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeAttack_(ownUnits, theirUnits);
	}
	void beforeCleaning(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeCleaning_(ownUnits, theirUnits);
	}
	void beforeReload(UnitTab& ownUnits, UnitTab& theirUnits)
	{
		beforeReload_(ownUnits, theirUnits);
	}
};

class NoSkill : public SkillInterface
{
};


struct UnitStat
{
	size_t livingCount;
	size_t deadCount;
	size_t meanPV;

	UnitStat(): livingCount(0), deadCount(0), meanPV(0) {}
};
struct ArmyStats
{
	boost::array<UnitStat, Ship::Count> shipStats;
	boost::array<UnitStat, Cannon::Count> cannonStats;
};

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

	for(Unit const & unit : unitTab)
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

//! Resultat d'un combat
enum FightStatus : uint8_t
{
  Fighter1Win,
  Fighter2Win,
  NobodyWin,
  NothingRemains
};
//! @brief Simule un combat entre deux combatant
template<typename F1, typename F2>
FightStatus fight(F1& fighter1, luabind::object roundFunc1,
                  F2& fighter2, luabind::object roundFunc2
                 )
{
	//! Construction des deux listes de vaisseaux (fillShipList)
	UnitTab shipTab1;
	fillShipList(fighter1, shipTab1);
	boost::random_shuffle(shipTab1);
	UnitTab shipTab2;
	fillShipList(fighter2, shipTab2);
	boost::random_shuffle(shipTab2);

	//! Jusqu'as ce qu'une des 2 flotte soit annéantie, on repète:
	size_t round = 0;
	ArmyStats armyStat1 = calcArmyStat(shipTab1);
	ArmyStats armyStat2 = calcArmyStat(shipTab2);
	UNTIL(shipTab1.empty() || shipTab2.empty() || round == 5)
	{
		//! Excecution des scripts de round
		/*
		unique_ptr<SkillInterface> skill1(roundFunc1.is_valid() ?
		                                  (SkillInterface*)luabind::call_function<SkillInterface*>(roundFunc1, boost::cref(armyStat1), boost::cref(armyStat2)) :
		                                  new NoSkill());
		unique_ptr<SkillInterface> skill2(roundFunc2.is_valid() ?
		                                  (SkillInterface*)luabind::call_function<SkillInterface*>(roundFunc2, boost::cref(armyStat2), boost::cref(armyStat1)) :
		                                  new NoSkill());
		*/
		unique_ptr<SkillInterface> skill1(new NoSkill());
		unique_ptr<SkillInterface> skill2(new NoSkill());

		//! Action script pré-attaque
		skill1->beforeAttack(shipTab1, shipTab2);
		skill2->beforeAttack(shipTab2, shipTab1);
		//! - Flotte 1 attaque flotte 2 (applyRound)
		applyRound(shipTab1, shipTab2);
		//! - Flotte 2 attaque flotte 1 (applyRound)
		applyRound(shipTab2, shipTab1);

		//! Action script pré-nétoyage des morts
		skill1->beforeCleaning(shipTab1, shipTab2);
		skill2->beforeCleaning(shipTab2, shipTab1);

		armyStat1 = calcArmyStat(shipTab1);
		armyStat2 = calcArmyStat(shipTab2);

		//! - On supprime les vaisseaux qui n'on plus de point de vie
		remove_erase_if(shipTab1, boost::bind(&Unit::life, _1) <= 0);
		remove_erase_if(shipTab2, boost::bind(&Unit::life, _1) <= 0);

		//! Action script pré-rechargement boucliers
		skill1->beforeReload(shipTab1, shipTab2);
		skill2->beforeReload(shipTab2, shipTab1);
		//! - On recharge les boucliers
		loadShield(shipTab1);
		loadShield(shipTab2);

		round += 1;
	}

	//! On modidife le nombre d'unité de chaque combatant
	//! en fonction de ce qu'il reste de ca flotte. (fillFinalFleet)
	fillFinalFleet(shipTab1, fighter1);
	fillFinalFleet(shipTab2, fighter2);

	//! On retourne un FightStatus en fonction du contenue restant des flottes
	if(shipTab1.empty())
	{
		if(shipTab2.empty())
			return NothingRemains;
		else
			return Fighter2Win;
	}
	else
	{
		if(shipTab2.empty())
			return Fighter1Win;
		else
			return NobodyWin;
	}
}


struct FleetPair
{
	size_t index1;
	size_t index2;

	FleetPair(size_t f1, size_t f2) //Exceptionellement, je vais laisser de coté la liste d'initialization
	{
		if(f1 < f2)
		{
			index1 = f1;
			index2 = f2;
		}
		else
		{
			index1 = f2;
			index2 = f1;
		}
	}
};

template<typename F1, typename F2>
void handleFighterPair(FleetPair const& fleetPair,
                       Report<F1>& report1, F1& fighter1, luabind::object script1,
                       Report<F2>& report2, F2& fighter2, luabind::object script2
                      )
{
	report1.enemySet.insert(fleetPair.index2);
	report2.enemySet.insert(fleetPair.index1);

	report1.hasFight = true;
	report2.hasFight = true;
	FightStatus const status = fight(fighter1, script1, fighter2, script2);
	switch(status)
	{
	case Fighter1Win:
		report2.isDead = true;
		break;
	case Fighter2Win:
		report1.isDead = true;
		break;
	case NobodyWin:
		break;
	case NothingRemains:
		report1.isDead = true;
		report2.isDead = true;
		break;
	}
	report1.fightInfo.after = fighter1;
	report2.fightInfo.after = fighter2;
}

void fight(std::vector<Fleet*> const& fleetList,
           Planet* planet,
           PlayerCodeMap& codesMap,
           FightReport& reportList)
{
	if(fleetList.empty())
		return;

	reportList.fleetList.clear();
	boost::transform(fleetList, back_inserter(reportList.fleetList), []
	                 (Fleet const * fleetPtr)
	{
		return Report<Fleet>(*fleetPtr);
	});
	if(planet)
	{
		reportList.hasPlanet = true;
		reportList.planet = Report<Planet>(*planet);
	}


	//! On liste les paires combatantes
	std::vector<FleetPair> fightingPair;
	//! - Flotte/Flotte
	for(auto iter1 = fleetList.begin(),
	    end = fleetList.end();
	    iter1 != end; ++iter1)
	{
		for(auto iter2 = iter1 + 1; iter2 != end; ++iter2)
		{
			Player::ID const pla1 = (*iter1)->playerId;
			Player::ID const pla2 = (*iter2)->playerId;
			if(pla1 != pla2 && pla1 != Player::NoId && pla2 != Player::NoId)
			{
				//uint64_t const score1 = mapFind(univ.playerMap, player1)->second.score;
				//uint64_t const score2 = mapFind(univ.playerMap, player2)->second.score;
				//if((score1 * 5) > score2 && (score2 * 5) > score1)
				//TOTO: Remetre la limitation sur les score trop differents
				fightingPair.push_back(FleetPair(iter1 - fleetList.begin(),
				                                 iter2 - fleetList.begin()));
			}
		}
	}
	//! - Planete/Flotte
	if(reportList.hasPlanet)
	{
		for(auto iter1 = fleetList.begin(),
		    end = fleetList.end();
		    iter1 != end; ++iter1)
		{
			Player::ID const pla1 = (*iter1)->playerId;
			Player::ID const pla2 = planet->playerId;
			if(pla1 != pla2 && pla1 != Player::NoId && pla2 != Player::NoId)
			{
				//uint64_t const score1 = mapFind(univ.playerMap, player1)->second.score;
				//uint64_t const score2 = mapFind(univ.playerMap, player2)->second.score;
				//Bloquage si trop d'équart de niveaux
				//if((score1 * 5) > score2 && (score2 * 5) > score1)
				//TOTO: Remetre la limitation sur les score trop differents
				fightingPair.push_back(
				  FleetPair(iter1 - fleetList.begin(), PlanetIndex));
			}
		}
	}

	//! Pour extraire le script du joueur pour les round de combat
	auto getRoundScript = [&]
	                      (Player::ID pid)
	{
		return codesMap[pid].fleetsCode.functions["fight_round"];
	};

	//! Pour toute les combinaisons de 2 combatant - Combat:
	for(FleetPair const & fleetPair : fightingPair)
	{
		//! - planet vs flotte
		if(fleetPair.index1 == PlanetIndex)
		{
			Report<Planet>& report1 = reportList.planet.get();
			Report<Fleet>& report2 = reportList.fleetList[fleetPair.index2];
			Fleet& fleet = *fleetList[fleetPair.index2];
			handleFighterPair<Planet, Fleet>(
			  fleetPair,
			  report1, *planet, getRoundScript(planet->playerId),
			  report2, fleet, getRoundScript(fleet.playerId));
		}
		//! - flotte vs planet
		else if(fleetPair.index2 == PlanetIndex)
		{
			Report<Fleet>& report1 = reportList.fleetList[fleetPair.index1];
			Report<Planet>& report2 = reportList.planet.get();
			Fleet& fleet = *fleetList[fleetPair.index1];
			handleFighterPair<Fleet, Planet>(
			  fleetPair,
			  report1, fleet, getRoundScript(fleet.playerId),
			  report2, *planet, getRoundScript(planet->playerId));
		}
		//! - flotte vs flotte
		else
		{
			Report<Fleet>& report1 = reportList.fleetList[fleetPair.index1];
			Fleet* fighterPtr1 = fleetList[fleetPair.index1];
			Report<Fleet>& report2 = reportList.fleetList[fleetPair.index2];
			Fleet* fighterPtr2 = fleetList[fleetPair.index2];
			handleFighterPair<Fleet, Fleet>(
			  fleetPair,
			  report1, *fighterPtr1, getRoundScript(fighterPtr1->playerId),
			  report2, *fighterPtr2, getRoundScript(fighterPtr2->playerId));
		}
	}

	if(reportList.fleetList.size() != fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("Bad reports count!!"));
}



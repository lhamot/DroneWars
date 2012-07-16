#include "stdafx.h"
#include "fighting.h"

#include <vector>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

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
struct ShipInstance
{
	size_t type;
	long life;

	ShipInstance(size_t type, size_t life):
		life(long(life)), type(type)
	{
	}
};

void fillShipList(Fleet const& fleet, std::vector<ShipInstance>& shipTab)
{
	//Contage
	size_t shipNumber = 0;
	for(int i = 0; i < Ship::Count; ++i)
		shipNumber += fleet.shipList[i];

	shipTab.reserve(shipNumber);
	for(Ship::Enum type = Ship::Enum(0); type < Ship::Count; type = Ship::Enum(type + 1))
	{
		Ship const& def = Ship::List[type];
		size_t const count = fleet.shipList[type];
		for(size_t i = 0; i < count; ++i)
			shipTab.push_back(ShipInstance(type, def.life));
	}
}

void fillShipList(Planet const& planet, std::vector<ShipInstance>& shipTab)
{
	//Contage
	size_t shipNumber = 0;
	for(int i = 0; i < Cannon::Count; ++i)
		shipNumber += planet.cannonTab[i];

	shipTab.reserve(shipNumber);
	for(Cannon::Enum type = Cannon::Enum(0); type < Cannon::Count; type = Cannon::Enum(type + 1))
	{
		Cannon const& def = Cannon::List[type];
		size_t const count = planet.cannonTab[type];
		for(size_t i = 0; i < count; ++i)
			shipTab.push_back(ShipInstance(type + Ship::Count, def.life));
	}
}


void applyRound(std::vector<ShipInstance>& shipTab1, std::vector<ShipInstance>& shipTab2)
{
	size_t pos = 0;
	size_t const size = shipTab2.size();
	for(ShipInstance & ship: shipTab1)
	{
		size_t const power =
		  (ship.type < Ship::Count) ?
		  Ship::List[ship.type].power :
		  Cannon::List[ship.type - Ship::Count].power;
		shipTab2[pos].life -= long(power);
		++pos;
		if(pos == size)
			pos = 0;
	}
}

void fillFinalFleet(std::vector<ShipInstance> const& shipTab, Fleet& fleet) throw()
{
	Fleet::ShipTab& outTab = fleet.shipList;
	outTab.assign(Ship::Count, 0);
	for(ShipInstance const & ship: shipTab)
		++outTab[ship.type];
}

void fillFinalFleet(std::vector<ShipInstance> const& shipTab, Planet& planet) throw()
{
	Planet::CannonTab& outTab = planet.cannonTab;
	//outTab.assign(Cannon::Count, 0);
	outTab.fill(0);
	for(ShipInstance const & ship: shipTab)
		++outTab[ship.type - Ship::Count];
}


enum FightStatus
{
  Fighter1Win,
  Fighter2Win,
  NobodyWin,
  NothingRemains
};
template<typename F1, typename F2>
FightStatus fight(F1& fleet1, F2& fleet2)
{
	//Construction de la liste de vaisseaux
	std::vector<ShipInstance> shipTab1;
	fillShipList(fleet1, shipTab1);
	std::vector<ShipInstance> shipTab2;
	fillShipList(fleet2, shipTab2);

	//Rounds succesif
	while(shipTab1.empty() == false && shipTab2.empty() == false)
	{
		applyRound(shipTab1, shipTab2);
		applyRound(shipTab2, shipTab1);
		boost::remove_erase_if(shipTab1, [](ShipInstance const & ship) {return ship.life <= 0;});
		boost::remove_erase_if(shipTab2, [](ShipInstance const & ship) {return ship.life <= 0;});
	}

	//Impact sur les flotte
	fillFinalFleet(shipTab1, fleet1);
	fillFinalFleet(shipTab2, fleet2);

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
		if(index1 < index2)
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

	bool operator < (FleetPair const& other) const
	{
		if(index1 < other.index1)
			return true;
		else
			return index2 < other.index2;
	}
};

template<typename F1, typename F2>
void handleFighterPair(std::vector<Fleet*> const& fleetList,
                       FightReport& reportList,
                       FleetPair const& fleetPair,
                       Report<F1>& report1,
                       F1& fighter1,
                       Report<F2>& report2,
                       F2& fighter2
                      )
{
	report1.enemySet.insert(fleetPair.index2);
	report2.enemySet.insert(fleetPair.index1);

	report1.hasFight = true;
	report2.hasFight = true;
	FightStatus const status = fight(fighter1, fighter2);
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
	report1.fightInfo.after.eventList.clear();
	report2.fightInfo.after = fighter2;
	report2.fightInfo.after.eventList.clear();
}

void fight(std::vector<Fleet*> const& fleetList,
           Planet* planet,
           FightReport& reportList)
{
	if(fleetList.empty())
		return;

	reportList.fleetList.clear();
	boost::transform(fleetList, back_inserter(reportList.fleetList),
	[](Fleet * fleetPtr) {return Report<Fleet>(*fleetPtr);});
	if(planet)
	{
		reportList.hasPlanet = true;
		reportList.planet = Report<Planet>(*planet);
	}


	//! On liste les paires combatantes
	std::set<FleetPair> fightingPair;
	//! - Flotte/Flotte
	for(auto iter1 = fleetList.begin(), end = fleetList.end(); iter1 != end; ++iter1)
	{
		for(auto iter2 = iter1 + 1; iter2 != end; ++iter2)
		{
			Player::ID const player1 = (*iter1)->playerId;
			Player::ID const player2 = (*iter2)->playerId;
			if(player1 != player2 && player1 != Player::NoId && player2 != Player::NoId)
			{
				fightingPair.insert(FleetPair(iter1 - fleetList.begin(),
				                              iter2 - fleetList.begin()));
			}
		}
	}
	//! - Planete/Flotte
	if(reportList.hasPlanet)
	{
		for(auto iter1 = fleetList.begin(), end = fleetList.end(); iter1 != end; ++iter1)
		{
			Player::ID const player1 = (*iter1)->playerId;
			Player::ID const player2 = planet->playerId;
			if(player1 != player2 && player1 != Player::NoId && player2 != Player::NoId)
				fightingPair.insert(FleetPair(iter1 - fleetList.begin(), PlanetIndex));
		}
	}

	//! Pour toute les combinaisons de 2 combatant:
	for(FleetPair const & fleetPair: fightingPair)
	{
		//! -Combat
		if(fleetPair.index1 == PlanetIndex)
		{
			Report<Planet>& report1 = reportList.planet;
			Report<Fleet>& report2 = reportList.fleetList[fleetPair.index2];
			handleFighterPair<Planet, Fleet>(fleetList, reportList, fleetPair, report1, *planet, report2, *fleetList[fleetPair.index2]);
		}
		else if(fleetPair.index2 == PlanetIndex)
		{
			Report<Fleet>& report1 = reportList.fleetList[fleetPair.index1];
			Report<Planet>& report2 = reportList.planet;
			handleFighterPair<Fleet, Planet>(fleetList, reportList, fleetPair, report1, *fleetList[fleetPair.index1], report2, *planet);
		}
		else
		{
			Report<Fleet>& report1 = reportList.fleetList[fleetPair.index1];
			Fleet* fighterPtr1 = fleetList[fleetPair.index1];
			Report<Fleet>& report2 = reportList.fleetList[fleetPair.index2];
			Fleet* fighterPtr2 = fleetList[fleetPair.index2];
			handleFighterPair<Fleet, Fleet>(fleetList, reportList, fleetPair, report1, *fighterPtr1, report2, *fighterPtr2);
		}
	}

	if(reportList.fleetList.size() != fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("Bad reports count!!"));
}



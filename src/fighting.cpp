#include "fighting.h"

#include <vector>
#include <boost/foreach.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;


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
	Ship::Enum type;
	long life;

	ShipInstance(Ship::Enum type, size_t life):
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
		for(int i = 0; i < count; ++i)
			shipTab.push_back(ShipInstance(type, def.life));
	}
}

void applyRound(std::vector<ShipInstance>& shipTab1, std::vector<ShipInstance>& shipTab2)
{
	size_t pos = 0;
	size_t const size = shipTab2.size();
	BOOST_FOREACH(ShipInstance & ship, shipTab1)
	{
		shipTab2[pos].life -= long(Ship::List[ship.type].power);
		++pos;
		if(pos == size)
			pos = 0;
	}
}

void fillFinalFleet(std::vector<ShipInstance> const& shipTab, Fleet& fleet) throw()
{
	Fleet::ShipTab& outTab = fleet.shipList;
	outTab.assign(Ship::Count, 0);
	BOOST_FOREACH(ShipInstance const & ship, shipTab)
		++outTab[ship.type];
}


boost::logic::tribool fight(Fleet& fleet1, Fleet& fleet2)
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

	if(shipTab1.empty() == false && shipTab2.empty())
		return true;
	else if(shipTab1.empty() && shipTab2.empty() == false)
		return false;
	else
		return boost::logic::indeterminate;
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

void fight(std::vector<Fleet*> const& fleetList,
           FightReport& reportListResult)
{
	if(fleetList.empty())
		return;

	std::vector<FleetReport> reportList;
	boost::transform(fleetList,
	                 back_inserter(reportList),
	[](Fleet * fleetPtr) {return FleetReport(*fleetPtr);});

	// On list les paires combatantes
	std::set<FleetPair> fightingPair;
	for(auto iter1 = fleetList.begin(), end = fleetList.end(); iter1 != end; ++iter1)
	{
		for(auto iter2 = iter1 + 1; iter2 != end; ++iter2)
		{
			Fleet& fleet1 = **iter1;
			Fleet& fleet2 = **iter2;
			if(fleet1.playerId != fleet2.playerId)
			{
				fightingPair.insert(FleetPair(iter1 - fleetList.begin(),
				                              iter2 - fleetList.begin()));
			}
		}
	}

	//Pour toute les combinaisons de 2 flottes
	BOOST_FOREACH(FleetPair const & fleetPair, fightingPair)
	{
		//Combat
		FleetReport& report1 = reportList[fleetPair.index1];
		FleetReport& report2 = reportList[fleetPair.index2];
		report1.enemySet.insert(fleetPair.index2);
		report2.enemySet.insert(fleetPair.index1);
		Fleet& fleet1 = *fleetList[fleetPair.index1];
		Fleet& fleet2 = *fleetList[fleetPair.index2];
		report1.hasFight = true;
		report2.hasFight = true;
		boost::tribool result = fight(fleet1, fleet2);
		if(result == false)
			report1.isDead = true;
		else if(result == true)
			report2.isDead = true;
		report1.fleetsAfter = fleet1;
		report1.fleetsAfter.eventList.clear();
		report2.fleetsAfter = fleet2;
		report2.fleetsAfter.eventList.clear();
	}

	//BOOST_FOREACH(FleetState& report, reportList)
	//	report.fleetReport.fleetsAfter = *report.fleet;

	if(reportList.size() != fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("Bad reports count!!"));

	reportListResult.swap(reportList);
}



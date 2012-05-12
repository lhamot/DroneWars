#include "fighting.h"

#include <vector>
#include <boost/foreach.hpp>
#include <boost/range/algorithm.hpp>

using namespace std;

struct IncrFiller
{
	size_t value;

	IncrFiller(): value(0) {}

	size_t operator()()
	{
		return ++value;
	}
};

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
           std::vector<FleetReport>& reportListResult)
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
	}

	//BOOST_FOREACH(FleetState& report, reportList)
	//	report.fleetReport.fleetsAfter = *report.fleet;

	if(reportList.size() != fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("Bad reports count!!"));

	reportListResult.swap(reportList);
}



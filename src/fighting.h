#ifndef __BTA_FIGHTING__
#define __BTA_FIGHTING__

#include "Model.h"


struct FleetReport
{
	bool isDead;
	bool hasFight;
	std::set<size_t> enemySet; //par index dans le FightReport
	Fleet fleetsBefore;
	Fleet fleetsAfter;

	FleetReport(Fleet const& fleet): isDead(false), hasFight(false), fleetsBefore(fleet) {}
};

void fight(std::vector<Fleet*> const& fleetList, std::vector<FleetReport>& reportList);



#endif //__BTA_FIGHTING__
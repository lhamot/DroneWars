#ifndef __FLEET_VIEW__
#define __FLEET_VIEW__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4512 4275 4505)
#include <Wt/WContainerWidget>
#pragma warning(pop)

#include "Model.h"

class Engine;

class FleetViewWT : public Wt::WContainerWidget
{
	void refresh();

public:
	FleetViewWT(
	  WContainerWidget* parent,
	  Engine& eng,
	  Player::ID playerID);

	void setFleet(Fleet::ID fleetID);

private:
	Wt::WContainerWidget* createReportsTab(Wt::WContainerWidget*);

	Engine& engine_;
	Fleet::ID fleetID_;
	Wt::WTableView* reportsView_;
};

#endif //__FLEET_VIEW__
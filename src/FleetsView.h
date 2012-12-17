#ifndef __DRONEWARS_FLEETSVIEW__
#define __DRONEWARS_FLEETSVIEW__

#include "stdafx.h"
#include "Model.h"

class Engine;
class FleetViewWT;

class FleetsView : public Wt::WContainerWidget
{
public:
	FleetsView(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid);

private:
	void on_fleetTable_itemDoubleClicked(
	  Wt::WModelIndex const& index, Wt::WMouseEvent const&);

	virtual void refresh();

	Engine& engine_;
	Player::ID playerId_;
	Wt::WLayout* fleetLayout_;
	Wt::WTableView* fleetsView_;
	FleetViewWT* fleetView_;
};


#endif //__DRONEWARS_FLEETVIEW__
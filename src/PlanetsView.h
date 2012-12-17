#ifndef __DRONEWARS_PLANETSVIEW__
#define __DRONEWARS_PLANETSVIEW__

#include "stdafx.h"
#include "Player.h"

class Engine;
class PlanetViewWT;

class PlanetsView : public Wt::WContainerWidget
{
public:
	PlanetsView(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid);

private:
	void on_planetTable_itemDoubleClicked(
	  Wt::WModelIndex const& index, Wt::WMouseEvent const& me);

	virtual void refresh();

	Engine& engine_;
	Player::ID playerId_;
	Wt::WLayout* planetLayout_;
	Wt::WTableView* planetsView_;
	PlanetViewWT* planetView_;
};



#endif //__DRONEWARS_PLANETSVIEW__
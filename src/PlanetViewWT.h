#ifndef __PLANET_VIEW__
#define __PLANET_VIEW__

#include "stdafx.h"

#include "Model.h"

class Engine;

class PlanetViewWT : public Wt::WContainerWidget
{
public:
	PlanetViewWT(
	  WContainerWidget* parent,
	  Engine& eng);

	void setPlanet(Coord planet);

	static std::string getPlanetImagePath(Coord const& coord, bool miniature);

private:
	void refresh();
	Wt::WContainerWidget* createTasksTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createBuildingsTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createCannonsTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createResearchTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createDefenceTab(Wt::WContainerWidget*);

	Engine& engine_;
	Coord planetCoord_;
	Wt::WTableView* tasksView_;
	Wt::WTableView* buildingsView_;
	Wt::WTableView* cannonsView_;
	Wt::WImage* image_;
};

#endif //__PLANET_VIEW__
#ifndef __PLANET_VIEW__
#define __PLANET_VIEW__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4512 4275 4505)
//#include <QtGui/QDialog>
//#include "ui_PlanetView.h"
#include <Wt/WContainerWidget>
#pragma warning(pop)

#include "Engine.h"

class PlanetViewWT : public Wt::WContainerWidget
{
	void refresh();

public:
	PlanetViewWT(
	  WContainerWidget* parent,
	  Engine& eng,
	  Coord planet);

private:
	Wt::WContainerWidget* createTasksTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createBuildingsTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createResearchTab(Wt::WContainerWidget*);
	Wt::WContainerWidget* createDefenceTab(Wt::WContainerWidget*);

	Engine& engine_;
	Coord planetCoord_;
	Wt::WTableView* tasksView_;
	Wt::WTableView* buildingsView_;

};

#endif //__PLANET_VIEW__
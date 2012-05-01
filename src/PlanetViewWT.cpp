#include "PlanetViewWT.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <boost/foreach.hpp>
#include <Wt/WBoxLayout>
#include <Wt/WTabWidget>
#include <Wt/WText>
#include <Wt/WStandardItemModel>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WTableView>
#include <Wt/WStandardItem>
#pragma warning(pop)
#include "TranslationTools.h"

using namespace Wt;


Wt::WContainerWidget* PlanetViewWT::createTasksTab(Wt::WContainerWidget* parent)
{
	WContainerWidget *tasksTab = new WContainerWidget(parent);
	Wt::WHBoxLayout *layout = new Wt::WHBoxLayout();
	tasksTab->setLayout(layout);

	tasksView_ = new WTableView(tasksTab);

  tasksView_->setHeight(800);

	tasksView_->setAlternatingRowColors(true);
	tasksView_->setColumnWidth(0, 100);
	tasksView_->setColumnWidth(1, 150);
	tasksView_->setColumnWidth(2, 150);
	tasksView_->setColumnWidth(3, 200);
	layout->addWidget(tasksView_);
	
	return tasksTab;
}

Wt::WContainerWidget* PlanetViewWT::createBuildingsTab(Wt::WContainerWidget* parent)
{
	WContainerWidget *buildingsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout *layout = new Wt::WHBoxLayout();
	buildingsTab->setLayout(layout);

	buildingsView_ = new WTableView(buildingsTab);

  //buildingsView_->setHeight(800);

	buildingsView_->setAlternatingRowColors(true);
	buildingsView_->setColumnWidth(0, 100);
	buildingsView_->setColumnWidth(1, 40);
	layout->addWidget(buildingsView_);

	return buildingsTab;
}

/*Wt::WContainerWidget* PlanetView::createResearchTab(Wt::WContainerWidget*)
{
	WContainerWidget *researchTab = new WContainerWidget(parent);
	return researchTab;
}

Wt::WContainerWidget* PlanetView::createDefenceTab(Wt::WContainerWidget*)
{
	WContainerWidget *defenceTab = new WContainerWidget(parent);
	return defenceTab;
}*/


PlanetViewWT::PlanetViewWT(
	WContainerWidget* parent,
  Engine& eng,
  Coord plaCoord):
	WContainerWidget(parent),
	engine_(eng),
	planetCoord_(plaCoord)
{
	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout();
	setLayout(layout);


	WTabWidget *tab = new WTabWidget(this);
	tab->addTab(createBuildingsTab(this), "Buildings");
	tab->addTab(createTasksTab(this), "Tasks");
	//tab->addTab(createResearchTab(this), "Research");
	//tab->addTab(createDefenceTab(this), "Defence");
	layout->addWidget(tab);


	refreshAll();
}


void PlanetViewWT::refreshAll()
{
	int row = 0;
	Planet planet = engine_.getPlanet(planetCoord_);
	Wt::WStandardItemModel *taModel = new Wt::WStandardItemModel((int)planet.taskQueue.size(), 4, this);
	taModel->setHeaderData(0, Horizontal, WString("Type"), DisplayRole);
	taModel->setHeaderData(1, Horizontal, WString("Start"), DisplayRole);
	taModel->setHeaderData(2, Horizontal, WString("End"), DisplayRole);
	taModel->setHeaderData(3, Horizontal, WString("What"), DisplayRole);
	BOOST_FOREACH(PlanetTask const & task, planet.taskQueue)
	{
		Wt::WStandardItem *item = new Wt::WStandardItem();
		item->setData(getTaskName(task.type), DisplayRole);
		taModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(timeToString(task.lauchTime), DisplayRole);
		taModel->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(timeToString(task.lauchTime + task.duration), DisplayRole);
		taModel->setItem(row, 2, item);

		item = new Wt::WStandardItem();
		item->setData(task.value, DisplayRole);
		taModel->setItem(row, 3, item);

		row += 1;
	}
	tasksView_->setModel(taModel);

	
	row = 0;
	Wt::WStandardItemModel *buModel = new Wt::WStandardItemModel((int)planet.taskQueue.size(), 4, this);
	buModel->setHeaderData(0, Horizontal, WString("Name"), DisplayRole);
	buModel->setHeaderData(1, Horizontal, WString("Level"), DisplayRole);
	BOOST_FOREACH(auto const & typeLevel, planet.buildingMap)
	{
		Wt::WStandardItem *item = new Wt::WStandardItem();
		item->setData(getBuildingName(typeLevel.first), DisplayRole);
		buModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(typeLevel.second, DisplayRole);
		buModel->setItem(row, 1, item);

		row += 1;
	}
	buildingsView_->setModel(buModel);
}
#include "stdafx.h"
#include "PlanetViewWT.h"

#include "TranslationTools.h"
#include "Engine.h"

using namespace Wt;


Wt::WContainerWidget* PlanetViewWT::createTasksTab(Wt::WContainerWidget* parent)
{
	WContainerWidget* tasksTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
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
	WContainerWidget* buildingsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	buildingsTab->setLayout(layout);

	buildingsView_ = new WTableView(buildingsTab);

	//buildingsView_->setHeight(800);

	buildingsView_->setAlternatingRowColors(true);
	buildingsView_->setColumnWidth(0, 100);
	buildingsView_->setColumnWidth(1, 40);
	layout->addWidget(buildingsView_);

	return buildingsTab;
}


Wt::WContainerWidget* PlanetViewWT::createCannonsTab(Wt::WContainerWidget* parent)
{
	WContainerWidget* cannonsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	cannonsTab->setLayout(layout);

	cannonsView_ = new WTableView(cannonsTab);

	cannonsView_->setAlternatingRowColors(true);
	cannonsView_->setColumnWidth(0, 100);
	cannonsView_->setColumnWidth(1, 40);
	layout->addWidget(cannonsView_);

	return cannonsTab;
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
	planetCoord_(plaCoord),
	tasksView_(nullptr),
	buildingsView_(nullptr),
	cannonsView_(nullptr)
{
	Wt::WVBoxLayout* layout = new Wt::WVBoxLayout();
	setLayout(layout);

	WTabWidget* tab = new WTabWidget(this);
	tab->addTab(createBuildingsTab(this), gettext("Buildings"));
	tab->addTab(createCannonsTab(this),   gettext("Cannons"));
	tab->addTab(createTasksTab(this),     gettext("Tasks"));
	//tab->addTab(createResearchTab(this), "Research");
	//tab->addTab(createDefenceTab(this), "Defence");
	layout->addWidget(tab);


	refresh();
}


void PlanetViewWT::refresh()
{
	//! Tasks
	int row = 0;
	Planet planet = engine_.getPlanet(planetCoord_);
	Wt::WStandardItemModel* taModel = new Wt::WStandardItemModel((int)planet.taskQueue.size(), 4, this);
	taModel->setHeaderData(0, Horizontal, gettext("Type"), DisplayRole);
	taModel->setHeaderData(1, Horizontal, gettext("Start"), DisplayRole);
	taModel->setHeaderData(2, Horizontal, gettext("End"), DisplayRole);
	taModel->setHeaderData(3, Horizontal, gettext("What"), DisplayRole);
	for(PlanetTask const & task: planet.taskQueue)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
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

	//! Buildings
	row = 0;
	Wt::WStandardItemModel* buModel = new Wt::WStandardItemModel((int)planet.buildingList.size(), 4, this);
	buModel->setHeaderData(0, Horizontal, gettext("Name"), DisplayRole);
	buModel->setHeaderData(1, Horizontal, gettext("Level"), DisplayRole);
	for(size_t type = 0; type < planet.buildingList.size(); ++type)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(getBuildingName(Building::Enum(type)), DisplayRole);
		buModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(planet.buildingList[type], DisplayRole);
		buModel->setItem(row, 1, item);

		row += 1;
	}
	buildingsView_->setModel(buModel);

	//! Cannons
	row = 0;
	Wt::WStandardItemModel* caModel = new Wt::WStandardItemModel((int)planet.cannonTab.size(), 4, this);
	caModel->setHeaderData(0, Horizontal, gettext("Name"), DisplayRole);
	caModel->setHeaderData(1, Horizontal, gettext("Level"), DisplayRole);
	for(auto const & level: planet.cannonTab)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(getCannonName(static_cast<Cannon::Enum>(row)), DisplayRole);
		caModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(level, DisplayRole);
		caModel->setItem(row, 1, item);

		row += 1;
	}
	cannonsView_->setModel(caModel);

	WContainerWidget::refresh();
}
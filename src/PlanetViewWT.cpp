#include "stdafx.h"
#include "PlanetViewWT.h"

#include "TranslationTools.h"
#include "Engine.h"
#include <boost/format.hpp>
#include <Wt/WImage>

using namespace Wt;
using namespace boost;


static size_t const PlanetImageCount = 23;

std::string PlanetViewWT::getPlanetImagePath(Coord const& coord, bool miniature)
{
	static std::hash<Coord> const hashCoord;
	size_t const planetHash = hashCoord(coord);
	size_t const imgIndex = planetHash % PlanetImageCount;
	char const* const prefix = miniature ? "min" : "pl";
	std::string const link = str(format("/img/Planetes/%1%%2$03d.jpg") % prefix % imgIndex);
	return link;
}


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


PlanetViewWT::PlanetViewWT(WContainerWidget* parent,
                           Engine& eng):
	WContainerWidget(parent),
	engine_(eng),
	planetCoord_(-1, -1, -1),
	tasksView_(nullptr),
	buildingsView_(nullptr),
	cannonsView_(nullptr),
	image_(nullptr)
{
	Wt::WVBoxLayout* layout = new Wt::WVBoxLayout();
	setLayout(layout);

	WTabWidget* tab = new WTabWidget(this);
	tab->addTab(createBuildingsTab(this), gettext("Buildings"));
	tab->addTab(createCannonsTab(this),   gettext("Cannons"));
	tab->addTab(createTasksTab(this),     gettext("Tasks"));
	layout->addWidget(tab);

	refresh();
}

void PlanetViewWT::setPlanet(Coord planet)
{
	planetCoord_ = planet;
	if(planetCoord_.X != -1)
	{
		if(image_ == nullptr)
		{
			image_ = new WImage(this);
			layout()->addWidget(image_);
		}
		image_->setImageLink(WLink(getPlanetImagePath(planetCoord_, false)));
	}
	else if(image_)
		image_->setImageLink(WLink());
	refresh();
}


void PlanetViewWT::refresh()
{
	//! Header Tasks
	Wt::WStandardItemModel* taModel = new Wt::WStandardItemModel(0, 4, this);
	taModel->setHeaderData(0, Horizontal, WString(gettext("Type")), DisplayRole);
	taModel->setHeaderData(1, Horizontal, WString(gettext("Start")), DisplayRole);
	taModel->setHeaderData(2, Horizontal, WString(gettext("End")), DisplayRole);
	taModel->setHeaderData(3, Horizontal, WString(gettext("What")), DisplayRole);

	//! Header Buildings
	Wt::WStandardItemModel* buModel = new Wt::WStandardItemModel(0, 4, this);
	buModel->setHeaderData(0, Horizontal, WString(gettext("Name")), DisplayRole);
	buModel->setHeaderData(1, Horizontal, WString(gettext("Level")), DisplayRole);

	//! Header Cannons
	Wt::WStandardItemModel* caModel = new Wt::WStandardItemModel(0, 4, this);
	caModel->setHeaderData(0, Horizontal, WString(gettext("Name")), DisplayRole);
	caModel->setHeaderData(1, Horizontal, WString(gettext("Level")), DisplayRole);

	if(planetCoord_.X != -1)
	{
		Planet planet = engine_.getPlanet(planetCoord_);

		//! Donnés Tasks
		int row = 0;
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
			switch(task.type)
			{
			case PlanetTask::UpgradeBuilding:
				item->setData(getBuildingName(Building::Enum(task.value)), DisplayRole);
				break;
			case PlanetTask::MakeShip:
				item->setData(getShipName(Ship::Enum(task.value)), DisplayRole);
				break;
			case PlanetTask::MakeCannon:
				item->setData(getCannonName(Cannon::Enum(task.value)), DisplayRole);
				break;
			default:
				BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent PlanetTask"));
			}
			static_assert(PlanetTask::Count == 3, "Have to update the switch");
			taModel->setItem(row, 3, item);

			row += 1;
		}

		//! Donnés Buildings
		row = 0;
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

		//! Donnés Cannons
		row = 0;
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

		//image_->setImageLink(WLink(getPlanetImagePath(planetCoord_, false)));
	}
	//else
	//	image_->setImageLink(WLink());

	tasksView_->setModel(taModel);
	buildingsView_->setModel(buModel);
	cannonsView_->setModel(caModel);

	WContainerWidget::refresh();
}
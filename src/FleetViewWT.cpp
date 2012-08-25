#include "stdafx.h"
#include "FleetViewWT.h"

#include "TranslationTools.h"
#include "Engine.h"


using namespace Wt;


Wt::WContainerWidget* FleetViewWT::createReportsTab(Wt::WContainerWidget* parent)
{
	WContainerWidget* reportsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	reportsTab->setLayout(layout);

	reportsView_ = new WTableView(reportsTab);

	//reportsView_->setHeight(800);

	reportsView_->setAlternatingRowColors(true);
	reportsView_->setColumnWidth(0, 150);
	reportsView_->setColumnWidth(1, 100);
	reportsView_->setColumnWidth(2, 200);
	layout->addWidget(reportsView_);

	return reportsTab;
}


FleetViewWT::FleetViewWT(
  WContainerWidget* parent,
  Engine& eng,
  Fleet::ID fleetID):
	WContainerWidget(parent),
	engine_(eng),
	fleetID_(fleetID)
{
	Wt::WVBoxLayout* layout = new Wt::WVBoxLayout();
	setLayout(layout);


	WTabWidget* tab = new WTabWidget(this);
	tab->addTab(createReportsTab(this), gettext("Reports"));
	layout->addWidget(tab);


	refresh();
}


void FleetViewWT::refresh()
{
	int row = 0;
	Fleet fleet = engine_.getFleet(fleetID_);
	Wt::WStandardItemModel* evModel = new Wt::WStandardItemModel((int)fleet.eventList.size(), 3, this);
	evModel->setHeaderData(0, Horizontal, gettext("Date"), DisplayRole);
	evModel->setHeaderData(1, Horizontal, gettext("Type"), DisplayRole);
	evModel->setHeaderData(2, Horizontal, gettext("Comment"), DisplayRole);
	for(Event const & ev: fleet.eventList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(timeToString(ev.time), DisplayRole);
		evModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(getEventName(ev.type), DisplayRole);
		evModel->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(ev.comment, DisplayRole);
		evModel->setItem(row, 2, item);

		row += 1;
	}
	reportsView_->setModel(evModel);

	WContainerWidget::refresh();
}


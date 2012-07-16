#include "stdafx.h"
#include "FleetViewWT.h"

#include "TranslationTools.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <Wt/WTabWidget>
#include <Wt/WText>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#pragma warning(pop)

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
	tab->addTab(createReportsTab(this), "Reports");
	layout->addWidget(tab);


	refresh();
}


void FleetViewWT::refresh()
{
	int row = 0;
	Fleet fleet = engine_.getFleet(fleetID_);
	Wt::WStandardItemModel* evModel = new Wt::WStandardItemModel((int)fleet.eventList.size(), 3, this);
	evModel->setHeaderData(0, Horizontal, WString("Date"), DisplayRole);
	evModel->setHeaderData(1, Horizontal, WString("Type"), DisplayRole);
	evModel->setHeaderData(2, Horizontal, WString("Comment"), DisplayRole);
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


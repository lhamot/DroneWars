#include "stdafx.h"
#include "bit_them_allWT.h"

#include "PlanetViewWT.h"
#include "PlanetsView.h"
#include "MessageView.h"
#include "TranslationTools.h"
#include "Editor.h"
#include "Engine.h"
#include "FleetsView.h"
#include <boost/format.hpp>

using namespace Wt;
using namespace boost;


template<typename T>
T* getWidget(WContainerWidget* parent, int index)
{
	return &dynamic_cast<T&>(*parent->widget(index));
}


WWidget* bit_them_allWT::createCodeTab(WContainerWidget* parent)
{
	int currentIndex = 0;
	int subIndex1 = 0;
	int subIndex2 = 0;
	if(codeTab_)
	{
		WContainerWidget& oldContainer = *codeTab_;
		WTabWidget& oldTab = dynamic_cast<WTabWidget&>(*oldContainer.children().front());
		currentIndex = oldTab.currentIndex();
		Editor& oldPlanetEditor = dynamic_cast<Editor&>(*oldTab.widget(0));
		subIndex1 = oldPlanetEditor.currentIndex();
		Editor& oldFleetEditor = dynamic_cast<Editor&>(*oldTab.widget(1));
		subIndex2 = oldFleetEditor.currentIndex();
	}

	WContainerWidget* codeTab = new WContainerWidget(parent);
	codeTab->setObjectName("CodeTab");
	WTabWidget* innerCodeTab = new Wt::WTabWidget(codeTab);
	innerCodeTab->setTabIndex(2);

	innerCodeTab->addTab(new WContainerWidget(parent), "dummy", WTabWidget::LazyLoading);

	Editor* editPlanetCode = new Editor(parent, "Planet", engine_, logged_, subIndex1);
	planetCode_ = editPlanetCode;
	innerCodeTab->addTab(planetCode_, gettext("Planet"), WTabWidget::LazyLoading);

	Editor* editFleetCode = new Editor(parent, "Fleet", engine_, logged_, subIndex2);
	fleetCode_ = editFleetCode;
	innerCodeTab->addTab(editFleetCode, gettext("Fleet"), WTabWidget::LazyLoading);

	innerCodeTab->setCurrentIndex(currentIndex + 1);

	WWidget* dummy = innerCodeTab->widget(0);
	innerCodeTab->removeTab(dummy);
	delete dummy;
	dummy = nullptr;

	codeTab_ = codeTab;
	return codeTab;
}


WWidget* bit_them_allWT::createReportTab(WContainerWidget* parent)
{
	WContainerWidget* reportsTab = new WContainerWidget(parent);
	reportsTab->setTabIndex(3);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	reportsTab->setLayout(layout);

	eventView_ = new WTableView(reportsTab);

	eventView_->setHeight(600);

	eventView_->setAlternatingRowColors(true);
	eventView_->clicked().connect(this, &bit_them_allWT::on_messageTable_itemDoubleClicked);
	eventView_->setColumnWidth(2, 400);
	layout->addWidget(eventView_);

	messageLayout_ = layout;

	return reportsTab;
}

WWidget* bit_them_allWT::createPlanetsTab(WContainerWidget* parent)
{
	return new PlanetsView(parent, engine_, logged_);
}


WWidget* bit_them_allWT::createFleetsTab(WContainerWidget* parent)
{
	return new FleetsView(parent, engine_, logged_);
}


bit_them_allWT::bit_them_allWT(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid):
	WContainerWidget(parent),
	logged_(pid),
	engine_(engine),
	fleetCode_(nullptr),
	planetCode_(nullptr),
	codeTab_(nullptr),
	eventView_(nullptr),
	messageLayout_(nullptr)
{


	WPushButton* refresh = new WPushButton(this);
	refresh->setMaximumSize(1000, 50);
	refresh->setText(gettext("Refresh"));
	refresh->clicked().connect(this, &bit_them_allWT::refresh);

	addWidget(refresh);

	Wt::WStackedWidget* contents = new Wt::WStackedWidget();
	Wt::WMenu* tab = new Wt::WMenu(contents, Wt::Horizontal, this);
	tab->setInternalPathEnabled("main");

	addWidget(contents);

	tab->setRenderAsList(false);

	//Si l'ordre est changer: Penser a la répercuter dans onTabChanged
	tab->addItem(gettext("Planets"), createPlanetsTab(this));
	tab->addItem(gettext("Fleets"), createFleetsTab(this));
	tab->addItem(gettext("Code"), createCodeTab(this));
	tab->addItem(gettext("Reports"), createReportTab(this));

	tab->itemSelected().connect(this, &bit_them_allWT::onTabChanged);

	bit_them_allWT::refresh();
}

void bit_them_allWT::onTabChanged(Wt::WMenuItem* item)
{
	switch(item->menu()->currentIndex())
	{
	case 0:
	{
		static char const* const PlanetViewTutoTag = "PlanetView" ;
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(PlanetViewTutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), gettext("PLANET_TUTOS"), Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, PlanetViewTutoTag);
		}
	}
	break;
	case 1:
	{
		static char const* const FleetViewTutoTag = "FleetView" ;
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(FleetViewTutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), gettext("FLEET_TUTOS"), Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, FleetViewTutoTag);
		}
	}
	break;
	case 2:
	{
		static char const* const CodeViewTutoTag = "CodeView" ;
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(CodeViewTutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), gettext("CODE_TUTOS"), Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, CodeViewTutoTag);
		}
	}
	break;
	case 3:
	{
		static char const* const ReportViewTutoTag = "ReportView" ;
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(ReportViewTutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), gettext("REPORT_TUTOS"), Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, ReportViewTutoTag);
		}
	}
	break;
	}
}

bit_them_allWT::~bit_them_allWT()
{

}

void bit_them_allWT::refresh()
{
	WMenu* tab = &dynamic_cast<WMenu&>(*widget(1));
	int const index1 = tab->currentIndex();

	WMenuItem* planetItem = tab->items()[0];
	WMenuItem* fleetItem = tab->items()[1];
	WMenuItem* codeItem = tab->items()[2];
	WMenuItem* reports = tab->items()[3];
	tab->removeItem(reports);
	tab->removeItem(codeItem);
	tab->removeItem(fleetItem);
	tab->removeItem(planetItem);
	tab->addItem(gettext("Planets"), createPlanetsTab(this));
	tab->addItem(gettext("Fleets"), createFleetsTab(this));
	tab->addItem(gettext("Code"), createCodeTab(this));
	tab->addItem(gettext("Reports"), createReportTab(this));
	delete reports;
	reports = nullptr;
	delete codeItem;
	codeItem = nullptr;
	delete fleetItem;
	fleetItem = nullptr;
	delete planetItem;
	planetItem = nullptr;

	tab->select(index1);

	Player player = engine_.getPlayer(logged_);
	Wt::WStandardItemModel* model = new Wt::WStandardItemModel((int)player.eventList.size(), 3, this);
	model->setHeaderData(0, Horizontal, WString(gettext("Time")), DisplayRole);
	model->setHeaderData(1, Horizontal, WString(gettext("Type")), DisplayRole);
	model->setHeaderData(2, Horizontal, WString(gettext("Message")), DisplayRole);
	int row = 0;
	for(Event const & ev: player.eventList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(timeToString(ev.time), DisplayRole);
		item->setData(ev.id, UserRole);
		model->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(getEventName(ev.type), DisplayRole);
		model->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(ev.comment, DisplayRole);
		model->setItem(row, 2, item);

		row++;
	}

	eventView_->setModel(model);
}


void bit_them_allWT::on_refreshButton_clicked()
{
	refresh();
}


void bit_them_allWT::on_messageTable_itemDoubleClicked(WModelIndex const& index, WMouseEvent const&)
{
	WStandardItemModel& model = dynamic_cast<WStandardItemModel&>(*eventView_->model());
	Event::ID eventID = any_cast<Event::ID>(model.data(index.row(), 0, UserRole));

	if(messageLayout_->count() > 1)
	{
		WLayoutItem* item = messageLayout_->itemAt(1);
		messageLayout_->removeItem(item);
	}
	MessageView* messageView = new MessageView(
	  &dynamic_cast<WContainerWidget&>(*messageLayout_->parent()), engine_, logged_, eventID);
	messageView->setInline(true);
	messageLayout_->addWidget(messageView);
}



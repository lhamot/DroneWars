#include "stdafx.h"
#include "bit_them_allWT.h"

#include "PlanetViewWT.h"
#include "FleetViewWT.h"
#include "MessageView.h"
#include "TranslationTools.h"
#include "Editor.h"
#include "Engine.h"
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
	WContainerWidget* superPlanetsTab = new WContainerWidget(parent);
	WContainerWidget* titleCont = new WContainerWidget(superPlanetsTab);
	titleCont->addStyleClass("manual");
	new WText(gettext("List of your planets"), titleCont);

	WContainerWidget* planetsTab = new WContainerWidget(superPlanetsTab);
	planetsTab->setTabIndex(0);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	planetsTab->setLayout(layout);

	planetsView_ = new WTableView(planetsTab);

	planetsView_->setHeight(600);

	planetsView_->setAlternatingRowColors(true);
	planetsView_->clicked().connect(this, &bit_them_allWT::on_planetTable_itemDoubleClicked);
	planetsView_->setColumnWidth(1, 40);
	planetsView_->setColumnWidth(2, 40);
	planetsView_->setColumnWidth(3, 40);
	planetsView_->setColumnWidth(4, 20);
	planetsView_->setColumnWidth(5, 20);
	planetsView_->setColumnWidth(6, 200);
	layout->addWidget(planetsView_);

	planetLayout_ = layout;

	return superPlanetsTab;
}


WWidget* bit_them_allWT::createFleetsTab(WContainerWidget* parent)
{
	WContainerWidget* fleetsTab = new WContainerWidget(parent);
	fleetsTab->setTabIndex(1);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	fleetsTab->setLayout(layout);

	fleetsView_ = new WTableView(fleetsTab);

	fleetsView_->setHeight(600);

	fleetsView_->setAlternatingRowColors(true);
	fleetsView_->clicked().connect(this, &bit_them_allWT::on_fleetTable_itemDoubleClicked);
	fleetsView_->setColumnWidth(0, 40);
	fleetsView_->setColumnWidth(1, 40);
	fleetsView_->setColumnWidth(2, 40);
	fleetsView_->setColumnWidth(3, 20);
	fleetsView_->setColumnWidth(4, 100);
	fleetsView_->setColumnWidth(5, 100);
	layout->addWidget(fleetsView_);

	fleetLayout_ = layout;

	return fleetsTab;
}


bit_them_allWT::bit_them_allWT(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid):
	WContainerWidget(parent),
	logged_(pid),
	engine_(engine),
	fleetCode_(nullptr),
	planetCode_(nullptr),
	codeTab_(nullptr),
	eventView_(nullptr),
	planetsView_(nullptr),
	fleetsView_(nullptr),
	messageLayout_(nullptr),
	fleetLayout_(nullptr),
	planetLayout_(nullptr)
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

	int row = 0;
	std::vector<Planet> planetList = engine_.getPlayerPlanets(logged_);
	Wt::WStandardItemModel* plModel = new Wt::WStandardItemModel((int)planetList.size(), 7, this);
	plModel->setHeaderData(0, Horizontal, WString(gettext("Name")), DisplayRole);
	plModel->setHeaderData(1, Horizontal, WString("X"), DisplayRole);
	plModel->setHeaderData(2, Horizontal, WString("Y"), DisplayRole);
	plModel->setHeaderData(3, Horizontal, WString("Z"), DisplayRole);
	plModel->setHeaderData(4, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(5, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(6, Horizontal, WString(gettext("Ressources")), DisplayRole);
	for(Planet const & planet: planetList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(planet.name, DisplayRole);
		plModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(planet.coord.X, DisplayRole);
		plModel->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(planet.coord.Y, DisplayRole);
		plModel->setItem(row, 2, item);

		item = new Wt::WStandardItem();
		item->setData(planet.coord.Z, DisplayRole);
		plModel->setItem(row, 3, item);

		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr += (
			             boost::format("%1$c:%2%; ") %
			             getRessourceName(static_cast<Ressource::Enum>(i)) %
			             planet.ressourceSet.tab[i]).str();
		}

		item = new Wt::WStandardItem();
		item->setData(ressStr, DisplayRole);
		plModel->setItem(row, 6, item);

		row += 1;
	}
	planetsView_->setModel(plModel);



	row = 0;
	std::vector<Fleet> fleetList = engine_.getPlayerFleets(logged_);
	Wt::WStandardItemModel* flModel = new Wt::WStandardItemModel((int)fleetList.size(), 6, this);
	flModel->setHeaderData(0, Horizontal, WString("X"), DisplayRole);
	flModel->setHeaderData(1, Horizontal, WString("Y"), DisplayRole);
	flModel->setHeaderData(2, Horizontal, WString("Z"), DisplayRole);
	flModel->setHeaderData(3, Horizontal, WString(""), DisplayRole);
	flModel->setHeaderData(4, Horizontal, WString(gettext("Contents")), DisplayRole); //Pas bon
	flModel->setHeaderData(5, Horizontal, WString(gettext("Ressources")), DisplayRole); //Pas bon
	for(Fleet const & fleet: fleetList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(fleet.coord.X, DisplayRole);
		item->setData(fleet.id, UserRole);
		flModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(fleet.coord.Y, DisplayRole);
		flModel->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(fleet.coord.Z, DisplayRole);
		flModel->setItem(row, 2, item);

		item = new Wt::WStandardItem();
		item->setData(fleet.name, DisplayRole);
		flModel->setItem(row, 3, item);

		std::string content;
		for(int i = 0; i < Ship::Count; ++i)
		{
			if(fleet.shipList[i])
				content += (boost::format("%1$c:%2%; ") %
				            getShipName(static_cast<Ship::Enum>(i)) %
				            fleet.shipList[i]).str();
		}
		item = new Wt::WStandardItem();
		item->setData(content, DisplayRole);
		flModel->setItem(row, 4, item);

		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr += (boost::format("%1$c:%2%; ") %
			            getRessourceName(static_cast<Ressource::Enum>(i)) %
			            fleet.ressourceSet.tab[i]).str();
		}
		item = new Wt::WStandardItem();
		item->setData(ressStr, DisplayRole);
		flModel->setItem(row, 5, item);

		row += 1;
	}
	fleetsView_->setModel(flModel);

	Player player = engine_.getPlayer(logged_);
	Wt::WStandardItemModel* model = new Wt::WStandardItemModel((int)player.eventList.size(), 3, this);
	model->setHeaderData(0, Horizontal, WString(gettext("Time")), DisplayRole);
	model->setHeaderData(1, Horizontal, WString(gettext("Type")), DisplayRole);
	model->setHeaderData(2, Horizontal, WString(gettext("Message")), DisplayRole);
	row = 0;
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

void bit_them_allWT::on_planetTable_itemDoubleClicked(WModelIndex const& index, WMouseEvent const&)
{
	WStandardItemModel& model = dynamic_cast<WStandardItemModel&>(*planetsView_->model());
	Coord::Value const x = any_cast<Coord::Value>(model.data(index.row(), 1, DisplayRole));
	Coord::Value const y = any_cast<Coord::Value>(model.data(index.row(), 2, DisplayRole));
	Coord::Value const z = any_cast<Coord::Value>(model.data(index.row(), 3, DisplayRole));

	PlanetViewWT* planetView = new PlanetViewWT(
	  &dynamic_cast<WContainerWidget&>(*planetLayout_->parent()), engine_, logged_, Coord(x, y, z));

	if(planetLayout_->count() > 1)
	{
		WLayoutItem* item = planetLayout_->itemAt(1);
		planetLayout_->removeItem(item);
	}
	planetLayout_->addWidget(planetView);
}

void bit_them_allWT::on_fleetTable_itemDoubleClicked(WModelIndex const& index, WMouseEvent const&)
{
	WStandardItemModel& model = dynamic_cast<WStandardItemModel&>(*fleetsView_->model());
	Fleet::ID fleetID = any_cast<Fleet::ID>(model.data(index.row(), 0, UserRole));

	if(fleetLayout_->count() > 1)
	{
		WLayoutItem* item = fleetLayout_->itemAt(1);
		fleetLayout_->removeItem(item);
	}
	FleetViewWT* fleetView = new FleetViewWT(
	  &dynamic_cast<WContainerWidget&>(*fleetLayout_->parent()), engine_, logged_, fleetID);
	fleetLayout_->addWidget(fleetView);
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



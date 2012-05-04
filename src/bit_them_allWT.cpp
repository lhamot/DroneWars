#include "bit_them_allWT.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <QtGui/QFileDialog>
#include <Wt/Ext/Menu>
#include <Wt/Ext/ToolBar>
#include <Wt/Ext/Button>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WContainerWidget>
#include <Wt/WTabWidget>
#include <Wt/WPushButton>
#include <Wt/WTableView>
#include <Wt/WTextArea>
#include <Wt/WAbstractTableModel>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WText>
#include <Wt/WJavaScript>
#pragma warning(pop)

//#include "PythonHighlighter.h"
#include "PlanetViewWT.h"
#include "FleetViewWT.h"
#include "TranslationTools.h"

using namespace Wt;
using namespace boost;

WContainerWidget* bit_them_allWT::createFleetTab(WContainerWidget* parent, std::string const& name)
{
	WContainerWidget* codeTab = new WContainerWidget(parent);
	WTextArea* edit = new WTextArea(codeTab);
	edit->setRows(80);
	edit->setColumns(120);

	WPushButton* reset = new WPushButton(codeTab);
	reset->setText("Reset");

	WPushButton* save = new WPushButton(codeTab);
	save->setText("Save");

	edit->setId(name + "TextArea");
	edit->doJavaScript(
	  "var editor = CodeMirror.fromTextArea(document.getElementById(\"" + name + "TextArea\"), {"
	  "tabMode: \"indent\","
	  "matchBrackets: true,"
	  "theme: \"cobalt\","
	  "onHighlightComplete: function(editor) {editor.save();}"
	  "});"
	);

	if(name == "Fleet")
	{
		fleetCode_ = edit;
		save->clicked().connect(this, &bit_them_allWT::on_saveFleetCodeButton_clicked);
		reset->clicked().connect(this, &bit_them_allWT::on_resetFleetCodeButton_clicked);
	}
	else if(name == "Planet")
	{
		planetCode_ = edit;
		save->clicked().connect(this, &bit_them_allWT::on_savePlanetCodeButton_clicked);
		reset->clicked().connect(this, &bit_them_allWT::on_resetPlanetCodeButton_clicked);
	}
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));

	return codeTab;
}

WContainerWidget* bit_them_allWT::createCodeTab(WContainerWidget* parent)
{
	WContainerWidget* codeTab = new WContainerWidget(parent);

	codeTab_ = new Wt::WTabWidget(codeTab);

	codeTab_->addTab(createFleetTab(codeTab, "Fleet"), "Fleet");
	codeTab_->addTab(createFleetTab(codeTab, "Planet"), "Planet");

	return codeTab;
}


WContainerWidget* bit_them_allWT::createReportTab(WContainerWidget* parent)
{
	WContainerWidget* reportsTab = new WContainerWidget(parent);
	Wt::WVBoxLayout* layout = new Wt::WVBoxLayout();
	reportsTab->setLayout(layout);

	eventView_ = new WTableView(reportsTab);

	eventView_->setHeight(600);

	//eventView_->setModel(new EventModel(reportsTab, engine_, logged_));
	eventView_->setAlternatingRowColors(true);
	eventView_->setColumnWidth(2, 400);
	layout->addWidget(eventView_);

	return reportsTab;
}

WContainerWidget* bit_them_allWT::createPlanetsTab(WContainerWidget* parent)
{
	WContainerWidget* planetsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	planetsTab->setLayout(layout);

	planetsView_ = new WTableView(planetsTab);

	planetsView_->setHeight(600);

	planetsView_->setAlternatingRowColors(true);
	planetsView_->clicked().connect(this, &bit_them_allWT::on_planetTable_itemDoubleClicked);
	planetsView_->setColumnWidth(0, 40);
	planetsView_->setColumnWidth(1, 40);
	planetsView_->setColumnWidth(2, 40);
	planetsView_->setColumnWidth(3, 20);
	planetsView_->setColumnWidth(4, 20);
	planetsView_->setColumnWidth(5, 200);
	//planetsView_->setColumnWidth(2, 400);
	layout->addWidget(planetsView_);

	planetLayout_ = layout;

	return planetsTab;
}


WContainerWidget* bit_them_allWT::createFleetsTab(WContainerWidget* parent)
{
	WContainerWidget* fleetsTab = new WContainerWidget(parent);
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
	//planetsView_->setColumnWidth(2, 400);
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
	fleetLayout_(nullptr),
	planetLayout_(nullptr)
{
	//setupUi();

	//logged_ = 0;

	//Wt::WBoxLayout *layout = new Wt::WBoxLayout(WBoxLayout::TopToBottom);
	//setLayout(layout);

	WPushButton* refresh = new WPushButton(this);
	refresh->setMaximumSize(1000, 50);
	refresh->setText("Refresh");
	refresh->clicked().connect(this, &bit_them_allWT::refreshAll);

	addWidget(refresh);

	Wt::WTabWidget* tab = new Wt::WTabWidget(this);

	tab->addTab(createPlanetsTab(this), "Planets");
	tab->addTab(createFleetsTab(this),  "Fleets");
	tab->addTab(createCodeTab(this),    "Code");
	tab->addTab(createReportTab(this),  "Reports");

	addWidget(tab);

	refreshAll();
}

bit_them_allWT::~bit_them_allWT()
{

}

void bit_them_allWT::refreshAll()
{
	std::string planetCode = engine_.getPlayerPlanetCode(logged_);
	planetCode_->setText(planetCode.c_str());

	std::string fleetCode = engine_.getPlayerFleetCode(logged_);
	fleetCode_->setText(fleetCode.c_str());


	int row = 0;
	std::vector<Planet> planetList = engine_.getPlayerPlanets(logged_);
	Wt::WStandardItemModel* plModel = new Wt::WStandardItemModel((int)planetList.size(), 6, this);
	plModel->setHeaderData(0, Horizontal, WString("X"), DisplayRole);
	plModel->setHeaderData(1, Horizontal, WString("Y"), DisplayRole);
	plModel->setHeaderData(2, Horizontal, WString("Z"), DisplayRole);
	plModel->setHeaderData(3, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(4, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(5, Horizontal, WString("Ressources"), DisplayRole);
	BOOST_FOREACH(Planet const & planet, planetList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(planet.coord.X, DisplayRole);
		plModel->setItem(row, 0, item);

		item = new Wt::WStandardItem();
		item->setData(planet.coord.Y, DisplayRole);
		plModel->setItem(row, 1, item);

		item = new Wt::WStandardItem();
		item->setData(planet.coord.Z, DisplayRole);
		plModel->setItem(row, 2, item);

		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr +=
			  getRessourceName(static_cast<Ressource::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(planet.ressourceSet.tab[i]) +
			  std::string("; ");
		}

		item = new Wt::WStandardItem();
		item->setData(ressStr, DisplayRole);
		plModel->setItem(row, 5, item);

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
	flModel->setHeaderData(4, Horizontal, WString("Contents"), DisplayRole);
	flModel->setHeaderData(5, Horizontal, WString("Ressources"), DisplayRole);
	BOOST_FOREACH(Fleet const & fleet, fleetList)
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
			content +=
			  getShipName(static_cast<Ship::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(fleet.shipList[i]) +
			  std::string("; ");
		}
		item = new Wt::WStandardItem();
		item->setData(content, DisplayRole);
		flModel->setItem(row, 4, item);

		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr +=
			  getRessourceName(static_cast<Ressource::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(fleet.ressourceSet.tab[i]) +
			  std::string("; ");
		}
		item = new Wt::WStandardItem();
		item->setData(ressStr, DisplayRole);
		flModel->setItem(row, 5, item);

		row += 1;
	}
	fleetsView_->setModel(flModel);

	Player player = engine_.getPlayer(logged_);
	Wt::WStandardItemModel* model = new Wt::WStandardItemModel((int)player.eventList.size(), 3, this);
	model->setHeaderData(0, Horizontal, WString("Time"), DisplayRole);
	model->setHeaderData(1, Horizontal, WString("Type"), DisplayRole);
	model->setHeaderData(2, Horizontal, WString("Message"), DisplayRole);
	row = 0;
	BOOST_FOREACH(Event const & ev, player.eventList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(timeToString(ev.time), DisplayRole);
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

	refresh();
}


void bit_them_allWT::on_saveFleetCodeButton_clicked()
{
	std::string code = fleetCode_->text().toUTF8();
	engine_.setPlayerFleetCode(logged_, code);
}

void bit_them_allWT::on_savePlanetCodeButton_clicked()
{
	std::string code = planetCode_->text().toUTF8();
	engine_.setPlayerPlanetCode(logged_, code);
}

void bit_them_allWT::on_resetFleetCodeButton_clicked()
{
	WWidget* oldTab = codeTab_->widget(0);
	codeTab_->removeTab(oldTab);
	delete oldTab;
	oldTab = codeTab_->widget(1);
	codeTab_->removeTab(oldTab);
	delete oldTab;
	codeTab_->addTab(createFleetTab(this, "Fleet"), "Fleet");
	codeTab_->addTab(createFleetTab(this, "Planet"), "Planet");
	codeTab_->setCurrentIndex(1);
	refreshAll();

	//	std::string code = engine_.getPlayerFleetCode(logged_);
	//	fleetCode_->setText(code.c_str());
	//	refresh();
}

void bit_them_allWT::on_resetPlanetCodeButton_clicked()
{
	WWidget* oldTab = codeTab_->widget(1);
	codeTab_->removeTab(oldTab);
	delete oldTab;
	codeTab_->addTab(createFleetTab(this, "Planet"), "Planet");
	codeTab_->setCurrentIndex(1);
	refreshAll();

	/*std::string code = engine_.getPlayerPlanetCode(logged_);
	planetCode_->setText(code.c_str());
	planetCode_->doJavaScript(
		"window.editor.hideLine(0);"
		"window.editor = CodeMirror.fromTextArea(document.getElementById(\"PlanetTextArea\"), {"
	      "tabMode: \"indent\","
	      "matchBrackets: true,"
	      "theme: \"neat\","
				"onHighlightComplete: function(editor) {editor.save();}"
	    "});"
	);
	refresh();*/
}

void bit_them_allWT::on_refreshButton_clicked()
{
	refreshAll();
}

void bit_them_allWT::on_planetTable_itemDoubleClicked(WModelIndex const& index, WMouseEvent const&)
{
	WStandardItemModel& model = dynamic_cast<WStandardItemModel&>(*planetsView_->model());
	Coord::Value const x = any_cast<Coord::Value>(model.data(index.row(), 0, DisplayRole));
	Coord::Value const y = any_cast<Coord::Value>(model.data(index.row(), 1, DisplayRole));
	Coord::Value const z = any_cast<Coord::Value>(model.data(index.row(), 2, DisplayRole));

	PlanetViewWT* planetView = new PlanetViewWT(this, engine_, Coord(x, y, z));

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

	FleetViewWT* fleetView = new FleetViewWT(this, engine_, fleetID);

	WLayoutItem* item = fleetLayout_->itemAt(1);
	if(item)
		fleetLayout_->removeItem(item);
	fleetLayout_->addWidget(fleetView);
}


void bit_them_allWT::on_actionLoad_activated()
{
	//QString fileName = QFileDialog::getOpenFileName(this,
	//                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	//engine_.load(fileName.toStdString());
	refreshAll();
}


void bit_them_allWT::on_actionSave_activated()
{
	//QString fileName = QFileDialog::getSaveFileName(this,
	//                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	//engine_.save(fileName.toStdString());
}

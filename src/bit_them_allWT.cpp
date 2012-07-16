#include "stdafx.h"
#include "bit_them_allWT.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
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
#include <Wt/WMessageBox>
#include <Wt/WLengthValidator>
#include <Wt/WBreak>
#pragma warning(pop)

//#include "PythonHighlighter.h"
#include "PlanetViewWT.h"
#include "FleetViewWT.h"
#include "MessageView.h"
#include "TranslationTools.h"

using namespace Wt;
using namespace boost;


class Editor : public WContainerWidget
{
public:
	Editor(WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
		WContainerWidget(parent),
		edit_(nullptr),
		name_(name),
		engine_(engine),
		logged_(pid)
	{
		//WContainerWidget *codeTab = new WContainerWidget(this);
		refresh();
	}

private:
	void refresh()
	{
		edit_ = nullptr;

		//On supprime le contenue de codeTab
		while(count())
		{
			WWidget* toDelete = widget(0);
			removeWidget(toDelete);
			delete toDelete;
		}

		//On recrée son contenue
		WContainerWidget* container = new WContainerWidget(this);
		edit_ = new WTextArea(container);
		edit_->setRows(80);
		edit_->setColumns(120);

		WText* errorMessage = new WText(container);
		new WBreak(container);

		WPushButton* reset = new WPushButton(container);
		reset->setText("Reset");

		WPushButton* save = new WPushButton(container);
		save->setText("Save");

		edit_->setId(name_ + "TextArea");
		edit_->setValidator(new WLengthValidator(0, Player::MaxCodeSize, edit_));
		edit_->doJavaScript(
		  "var editor = CodeMirror.fromTextArea(document.getElementById(\"" + name_ + "TextArea\"), {"
		  "tabMode: \"indent\","
		  "matchBrackets: true,"
		  "lineNumbers: true,"
		  "theme: \"cobalt\","
		  "onHighlightComplete: function(editor) {editor.save();}"
		  "});"
		);

		save->clicked().connect(this, &Editor::on_saveCodeButton_clicked);
		reset->clicked().connect(this, &Editor::on_resetCodeButton_clicked);
		if(name_ == "Fleet")
		{
			CodeData code = engine_.getPlayerFleetCode(logged_);
			edit_->setText(code.getCode().c_str());
			errorMessage->setText(code.getLastError().c_str());
		}
		else if(name_ == "Planet")
		{
			CodeData code = engine_.getPlayerPlanetCode(logged_);
			edit_->setText(code.getCode().c_str());
			errorMessage->setText(code.getLastError().c_str());
		}
		else
			BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));
	}

	void on_saveCodeButton_clicked()
	{
		std::string code = edit_->text().toUTF8();
		if(name_ == "Fleet")
			engine_.setPlayerFleetCode(logged_, code);
		else if(name_ == "Planet")
			engine_.setPlayerPlanetCode(logged_, code);
	}

	void on_resetCodeButton_clicked()
	{
		refresh();
	}

	WTextArea* edit_;
	std::string name_;
	Engine& engine_;
	Player::ID logged_;
};

template<typename T>
T* getWidget(WContainerWidget* parent, int index)
{
	return &dynamic_cast<T&>(*parent->widget(index));
}


WWidget* bit_them_allWT::createCodeTab(WContainerWidget* parent)
{
	codeTab_ = new Wt::WTabWidget(parent);

	Editor* editPlanetCode = new Editor(parent, "Planet", engine_, logged_);
	planetCode_ = editPlanetCode;
	codeTab_->addTab(planetCode_, "Planet", WTabWidget::LazyLoading);

	Editor* editFleetCode = new Editor(parent, "Fleet", engine_, logged_);
	fleetCode_ = editFleetCode;
	codeTab_->addTab(editFleetCode, "Fleet", WTabWidget::LazyLoading);

	return codeTab_;
}


WWidget* bit_them_allWT::createReportTab(WContainerWidget* parent)
{
	WContainerWidget* reportsTab = new WContainerWidget(parent);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	reportsTab->setLayout(layout);

	eventView_ = new WTableView(reportsTab);

	eventView_->setHeight(600);

	//eventView_->setModel(new EventModel(reportsTab, engine_, logged_));
	eventView_->setAlternatingRowColors(true);
	eventView_->clicked().connect(this, &bit_them_allWT::on_messageTable_itemDoubleClicked);
	eventView_->setColumnWidth(2, 400);
	layout->addWidget(eventView_);

	messageLayout_ = layout;

	return reportsTab;
}

WWidget* bit_them_allWT::createPlanetsTab(WContainerWidget* parent)
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


WWidget* bit_them_allWT::createFleetsTab(WContainerWidget* parent)
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
	planetsView_(nullptr),
	fleetsView_(nullptr),
	messageLayout_(nullptr),
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
	refresh->clicked().connect(this, &bit_them_allWT::refresh);

	addWidget(refresh);

	Wt::WTabWidget* tab = new Wt::WTabWidget(this);

	tab->addTab(createPlanetsTab(this), "Planets", WTabWidget::LazyLoading);
	tab->addTab(createFleetsTab(this),  "Fleets", WTabWidget::LazyLoading);
	tab->addTab(createCodeTab(this),    "Code", WTabWidget::LazyLoading);
	tab->addTab(createReportTab(this),  "Reports", WTabWidget::LazyLoading);

	addWidget(tab);

	bit_them_allWT::refresh();
}

bit_them_allWT::~bit_them_allWT()
{

}

void bit_them_allWT::refresh()
{
	Wt::WContainerWidget::refresh();

	WTabWidget* tab = &dynamic_cast<WTabWidget&>(*widget(1));
	int const index1 = tab->currentIndex();
	int const index2 = codeTab_->currentIndex();

	tab->removeTab(codeTab_);
	delete codeTab_;
	codeTab_ = nullptr;
	tab->addTab(createCodeTab(this), "Code", WTabWidget::LazyLoading);
	WWidget* reports = tab->widget(2);
	tab->removeTab(reports);
	tab->addTab(reports, "Reports");
	tab->setCurrentIndex(index1);

	int row = 0;
	std::vector<Planet> planetList = engine_.getPlayerPlanets(logged_);
	Wt::WStandardItemModel* plModel = new Wt::WStandardItemModel((int)planetList.size(), 6, this);
	plModel->setHeaderData(0, Horizontal, WString("X"), DisplayRole);
	plModel->setHeaderData(1, Horizontal, WString("Y"), DisplayRole);
	plModel->setHeaderData(2, Horizontal, WString("Z"), DisplayRole);
	plModel->setHeaderData(3, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(4, Horizontal, WString(""), DisplayRole);
	plModel->setHeaderData(5, Horizontal, WString("Ressources"), DisplayRole);
	for(Planet const & planet: planetList)
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
	for(Event const & ev: player.eventList)
	{
		//std::cout << "ev.id = " << ev.id << std::endl;
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

	if(fleetLayout_->count() > 1)
	{
		WLayoutItem* item = fleetLayout_->itemAt(1);
		fleetLayout_->removeItem(item);
	}
	FleetViewWT* fleetView = new FleetViewWT(this, engine_, fleetID);
	fleetLayout_->addWidget(fleetView);
}


void bit_them_allWT::on_messageTable_itemDoubleClicked(WModelIndex const& index, WMouseEvent const&)
{
	WStandardItemModel& model = dynamic_cast<WStandardItemModel&>(*eventView_->model());
	Event::ID eventID = any_cast<Event::ID>(model.data(index.row(), 0, UserRole));

	//std::cout << "ev.id = " << eventID << std::endl;

	if(messageLayout_->count() > 1)
	{
		WLayoutItem* item = messageLayout_->itemAt(1);
		messageLayout_->removeItem(item);
	}
	MessageView* messageView = new MessageView(this, engine_, logged_, eventID);
	messageView->setInline(true);
	messageLayout_->addWidget(messageView);
}


void bit_them_allWT::on_actionLoad_activated()
{
	//QString fileName = QFileDialog::getOpenFileName(this,
	//                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	//engine_.load(fileName.toStdString());
	refresh();
}


void bit_them_allWT::on_actionSave_activated()
{
	//QString fileName = QFileDialog::getSaveFileName(this,
	//                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	//engine_.save(fileName.toStdString());
}

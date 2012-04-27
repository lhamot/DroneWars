#include "bit_them_all.h"

#pragma warning(push)
#pragma warning(disable:4251)
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <QtGui/QFileDialog>
#pragma warning(pop)

#include "PythonHighlighter.h"
#include "PlanetView.h"
#include "FleetView.h"
#include "TranslationTools.h"



bit_them_all::bit_them_all(QWidget* parent, Qt::WFlags flags):
	QMainWindow(parent, flags),
	logged_(Player::NoId)
{
	ui.setupUi(this);

	logged_ = 0;

	//connect(ui.saveFleetCodeButton, SIGNAL(clicked()), this, SLOT(onSaveFleetCode()));
	//connect(ui.savePlanetCodeButton, SIGNAL(clicked()), this, SLOT(onSavePlanetCode()));
	//connect(ui.resetFleetCodeButton, SIGNAL(clicked()), this, SLOT(onResetFleetCode()));
	//connect(ui.resetPlanetCodeButton, SIGNAL(clicked()), this, SLOT(onResetPlanetCode()));
	//connect(ui.refreshReportButton, SIGNAL(clicked()), this, SLOT(onResetPlanetCode()));

	PythonHighlighter* fleetHighlighter = new PythonHighlighter(ui.fleetCodeEdit->document());
	fleetHighlighter->setParent(ui.fleetCodeEdit->document());
	PythonHighlighter* planetHighlighter = new PythonHighlighter(ui.planetCodeEdit->document());
	planetHighlighter->setParent(ui.planetCodeEdit->document());

	refreshAll();
}

bit_them_all::~bit_them_all()
{

}

void bit_them_all::refreshAll()
{
	std::string planetCode = engine_.getPlayerPlanetCode(logged_);
	ui.planetCodeEdit->setText(planetCode.c_str());

	std::string fleetCode = engine_.getPlayerFleetCode(logged_);
	ui.fleetCodeEdit->setText(fleetCode.c_str());

	std::vector<Planet> planetList = engine_.getPlayerPlanets(logged_);
	ui.planetTable->clear();
	BOOST_FOREACH(Planet const & planet, planetList)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.planetTable);
		item->setData(0, 0, planet.coord.X);
		item->setData(1, 0, planet.coord.Y);
		item->setData(2, 0, planet.coord.Z);
		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr +=
			  getRessourceName(static_cast<Ressource::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(planet.ressourceSet.tab[i]) +
			  std::string("; ");
		}
		item->setData(5, 0, ressStr.c_str());
		ui.planetTable->addTopLevelItem(item);
	}


	std::vector<Fleet> fleetList = engine_.getPlayerFleets(logged_);
	ui.fleetTable->clear();
	BOOST_FOREACH(Fleet const & fleet, fleetList)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.fleetTable);
		item->setData(0, Qt::UserRole, fleet.id);
		item->setData(0, 0, fleet.coord.X);
		item->setData(1, 0, fleet.coord.Y);
		item->setData(2, 0, fleet.coord.Z);
		item->setData(3, 0, fleet.name.c_str());
		std::string content;
		for(int i = 0; i < Ship::Count; ++i)
		{
			content +=
			  getShipName(static_cast<Ship::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(fleet.shipList[i]) +
			  std::string("; ");
		}
		item->setData(4, 0, content.c_str());
		std::string ressStr;
		for(int i = 0; i < Ressource::Count; ++i)
		{
			ressStr +=
			  getRessourceName(static_cast<Ressource::Enum>(i))[0] +
			  std::string(":") +
			  boost::lexical_cast<std::string>(fleet.ressourceSet.tab[i]) +
			  std::string("; ");
		}
		item->setData(5, 0, ressStr.c_str());
		ui.fleetTable->addTopLevelItem(item);
	}

	Player player = engine_.getPlayer(logged_);
	ui.reportTreeWidget->clear();
	BOOST_FOREACH(Event const & ev, player.eventList)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.reportTreeWidget);
		item->setData(0, 0, ctime(&ev.time));
		item->setData(1, 0, ev.type);
		item->setData(2, 0, ev.comment.c_str());
		//item->setSizeHint(2, QSize(400, 400));
		ui.reportTreeWidget->addTopLevelItem(item);
	}
	ui.reportTreeWidget->setUniformRowHeights(true);
}


void bit_them_all::on_saveFleetCodeButton_clicked()
{
	std::string code = ui.fleetCodeEdit->toPlainText().toStdString();
	engine_.setPlayerFleetCode(logged_, code);
}

void bit_them_all::on_savePlanetCodeButton_clicked()
{
	std::string code = ui.planetCodeEdit->toPlainText().toStdString();
	engine_.setPlayerPlanetCode(logged_, code);
}

void bit_them_all::on_resetFleetCodeButton_clicked()
{
	std::string code = engine_.getPlayerFleetCode(logged_);
	ui.fleetCodeEdit->setText(code.c_str());
}

void bit_them_all::on_resetPlanetCodeButton_clicked()
{
	std::string code = engine_.getPlayerPlanetCode(logged_);
	ui.planetCodeEdit->setText(code.c_str());
}

void bit_them_all::on_refreshButton_clicked()
{
	refreshAll();
}

void bit_them_all::on_planetTable_itemDoubleClicked(QTreeWidgetItem* item, int) //column
{
	Coord::Value const x = item->data(0, 0).toInt();
	Coord::Value const y = item->data(1, 0).toInt();
	Coord::Value const z = item->data(2, 0).toInt();

	PlanetView* planetView = new PlanetView(this, 0, engine_, Coord(x, y, z));

	planetView->show();
}

void bit_them_all::on_fleetTable_itemDoubleClicked(QTreeWidgetItem* item, int) // column
{
	Fleet::ID fleetID = item->data(0, Qt::UserRole).toUInt();

	FleetView* fleetView = new FleetView(this, 0, engine_, fleetID);

	fleetView->show();
}


void bit_them_all::on_actionLoad_activated()
{
	QString fileName = QFileDialog::getOpenFileName(this,
	                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	engine_.load(fileName.toStdString());
	refreshAll();
}


void bit_them_all::on_actionSave_activated()
{
	QString fileName = QFileDialog::getSaveFileName(this,
	                   tr("Open Archive"), "", tr("Archive Files (*.bta)"));
	engine_.save(fileName.toStdString());
}

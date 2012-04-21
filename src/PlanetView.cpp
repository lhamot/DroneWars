#include "PlanetView.h"

#include <boost/foreach.hpp>

#include "TranslationTools.h"

void PlanetView::refreshAll()
{
}

PlanetView::PlanetView(
  QWidget* parent,
  Qt::WFlags flags,
  Engine& eng,
  Coord plaCoord):
	QDialog(parent, flags),
	engine_(eng),
	planetCoord_(plaCoord)
{
	ui.setupUi(this);

	Planet planet = engine_.getPlanet(planetCoord_);
	BOOST_FOREACH(auto const & typeLevel, planet.buildingMap)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.buildingsTreeWidget);
		item->setData(0, 0, getBuildingName(typeLevel.first).c_str());
		item->setData(1, 0, typeLevel.second);
		ui.buildingsTreeWidget->addTopLevelItem(item);
	}

	BOOST_FOREACH(Task const & task, planet.taskQueue)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.taskTreeWidget);
		item->setData(0, 0, getTaskName(task.type).c_str());
		item->setData(1, 0, task.lauchTime);
		item->setData(2, 0, task.duration);
		item->setData(3, 0, task.value);
		ui.taskTreeWidget->addTopLevelItem(item);
	}
}

PlanetView::~PlanetView()
{
}
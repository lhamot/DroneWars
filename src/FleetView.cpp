#include "FleetView.h"

#include <boost/foreach.hpp>

#include "TranslationTools.h"

void FleetView::refreshAll()
{
}

FleetView::FleetView(
  QWidget* parent,
  Qt::WFlags flags,
  Engine& eng,
  Fleet::ID fleetID):
	QDialog(parent, flags),
	engine_(eng),
	fleetID_(fleetID)
{
	ui.setupUi(this);

	Fleet fleet = engine_.getFleet(fleetID_);
	BOOST_FOREACH(FleetTask const & task, fleet.taskQueue)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.taskTreeWidget);
		item->setData(0, 0, getTaskName(task.type).c_str());
		item->setData(1, 0, task.lauchTime);
		item->setData(2, 0, task.duration);
		//item->setData(3, 0, task.coord);
		ui.taskTreeWidget->addTopLevelItem(item);
	}

	BOOST_FOREACH(Event const& ev, fleet.eventList)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.reportTreeWidget);
		item->setData(0, 0, getEventName(ev.type).c_str());
		item->setData(1, 0, ev.time);
		item->setData(2, 0, ev.comment.c_str());
		//item->setData(3, 0, task.coord);
		ui.reportTreeWidget->addTopLevelItem(item);
	}
}

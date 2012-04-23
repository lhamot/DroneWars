#ifndef __FLEET_VIEW__
#define __FLEET_VIEW__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800)
#include <QtGui/QDialog>
#include "ui_FleetView.h"
#pragma warning(pop)

#ifndef Q_MOC_RUN
#include "Engine.h"
#endif

class FleetView : public QDialog
{
	Q_OBJECT;

	void refreshAll();

public slots:

public:
	FleetView(
	  QWidget* parent,
	  Qt::WFlags flags,
	  Engine& eng,
	  Fleet::ID fleetID);

private:
	Ui::FleetView ui;
	Engine& engine_;
	Fleet::ID fleetID_;
};

#endif //__FLEET_VIEW__
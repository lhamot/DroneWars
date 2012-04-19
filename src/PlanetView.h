#ifndef __PLANET_VIEW__
#define __PLANET_VIEW__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800)
#include <QtGui/QDialog>
#include "ui_PlanetView.h"
#pragma warning(pop)

#ifndef Q_MOC_RUN
#include "Engine.h"
#endif

class PlanetView : public QDialog
{
	Q_OBJECT;

	void refreshAll();

public slots:

public:
	PlanetView(
	  QWidget* parent,
	  Qt::WFlags flags,
	  Engine& eng,
	  Coord planet);
	~PlanetView();

private:
	Ui::PlanetView ui;
	Engine& engine_;
	Coord planetCoord_;
};

#endif //__PLANET_VIEW__
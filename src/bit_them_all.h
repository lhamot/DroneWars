#ifndef BIT_THEM_ALL_H
#define BIT_THEM_ALL_H

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800)
#include <QtGui/QMainWindow>
#include "ui_bit_them_all.h"
#pragma warning(pop)

#ifndef Q_MOC_RUN
#include "Engine.h"
#endif

class bit_them_all : public QMainWindow
{
	Q_OBJECT;

	void refreshAll();

public slots:
	void on_saveFleetCodeButton_clicked();
	void on_savePlanetCodeButton_clicked();
	void on_resetFleetCodeButton_clicked();
	void on_resetPlanetCodeButton_clicked();
	void on_refreshButton_clicked();
	void on_planetTable_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_fleetTable_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_actionLoad_activated();
	void on_actionSave_activated();

public:
	bit_them_all(QWidget* parent = 0, Qt::WFlags flags = 0);
	~bit_them_all();

private:
	Ui::bit_them_all ui;
	Engine engine_;
	Player::ID logged_;
};

#endif // BIT_THEM_ALL_H

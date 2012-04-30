#ifndef BIT_THEM_ALL_H
#define BIT_THEM_ALL_H

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4505 4512 4275)
#include <Wt/WContainerWidget>
#include <Wt/WTreeView>
//#include "ui_bit_them_all.h"
#pragma warning(pop)

#include "Engine.h"

namespace Wt
{
	class WTextArea;
	class WTabWidget;
	class WTableView;
	class WModelIndex;
	class WMouseEvent;
}

class bit_them_allWT : public Wt::WContainerWidget
{
	void refreshAll();

	void on_saveFleetCodeButton_clicked();
	void on_savePlanetCodeButton_clicked();
	void on_resetFleetCodeButton_clicked();
	void on_resetPlanetCodeButton_clicked();
	void on_refreshButton_clicked();
	void on_planetTable_itemDoubleClicked(Wt::WModelIndex const& index, Wt::WMouseEvent const& me);
	void on_fleetTable_itemDoubleClicked(Wt::WModelIndex const& index, Wt::WMouseEvent const& me);
	void on_actionLoad_activated();
	void on_actionSave_activated();

public:
	bit_them_allWT(Engine& engine);
	~bit_them_allWT();

private:
	WContainerWidget* createFleetTab(WContainerWidget* parent, std::string const& name);
	WContainerWidget* createCodeTab(WContainerWidget* parent);
	WContainerWidget* createReportTab(WContainerWidget* parent);
	WContainerWidget* createPlanetsTab(WContainerWidget* parent);
	WContainerWidget* createFleetsTab(WContainerWidget* parent);

	Player::ID logged_;
	Engine& engine_;

	Wt::WTextArea *fleetCode_;
	Wt::WTextArea *planetCode_;
	Wt::WTabWidget *codeTab_;
	Wt::WTableView *eventView_;
	Wt::WTableView *planetsView_;
	Wt::WTableView *fleetsView_;
	Wt::WLayout *planetLayout_;
	Wt::WLayout *fleetLayout_;
};

#endif // BIT_THEM_ALL_H

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
	void refresh();

	void on_refreshButton_clicked();
	void on_planetTable_itemDoubleClicked(Wt::WModelIndex const& index, Wt::WMouseEvent const& me);
	void on_fleetTable_itemDoubleClicked(Wt::WModelIndex const& index, Wt::WMouseEvent const& me);
	void on_actionLoad_activated();
	void on_actionSave_activated();

public:
	bit_them_allWT(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid);
	~bit_them_allWT();

private:
	WWidget* createCodeTab(WContainerWidget* parent);
	WWidget* createReportTab(WContainerWidget* parent);
	WWidget* createPlanetsTab(WContainerWidget* parent);
	WWidget* createFleetsTab(WContainerWidget* parent);

	Player::ID logged_;
	Engine& engine_;

	Wt::WContainerWidget* fleetCode_;
	Wt::WContainerWidget* planetCode_;
	Wt::WTabWidget* codeTab_;
	Wt::WTableView* eventView_;
	Wt::WTableView* planetsView_;
	Wt::WTableView* fleetsView_;
	Wt::WLayout* planetLayout_;
	Wt::WLayout* fleetLayout_;
};

#endif // BIT_THEM_ALL_H

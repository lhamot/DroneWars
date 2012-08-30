#ifndef BIT_THEM_ALL_H
#define BIT_THEM_ALL_H

#include "stdafx.h"

#include "Player.h"

class Engine;

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
	void on_messageTable_itemDoubleClicked(Wt::WModelIndex const& index, Wt::WMouseEvent const& me);
	void on_actionLoad_activated();
	void on_actionSave_activated();

public:
	bit_them_allWT(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid);
	~bit_them_allWT();

private:
	Wt::WWidget* createCodeTab(Wt::WContainerWidget* parent);
	Wt::WWidget* createReportTab(Wt::WContainerWidget* parent);
	Wt::WWidget* createPlanetsTab(Wt::WContainerWidget* parent);
	Wt::WWidget* createFleetsTab(Wt::WContainerWidget* parent);
	void onTabChanged(int);

	Player::ID logged_;
	Engine& engine_;

	Wt::WContainerWidget* fleetCode_;
	Wt::WContainerWidget* planetCode_;
	Wt::WTabWidget* codeTab_;
	Wt::WTableView* eventView_;
	Wt::WTableView* planetsView_;
	Wt::WTableView* fleetsView_;
	Wt::WLayout* messageLayout_;
	Wt::WLayout* planetLayout_;
	Wt::WLayout* fleetLayout_;
};

#endif // BIT_THEM_ALL_H

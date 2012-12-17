#include "stdafx.h"
#include "FleetsView.h"
#include "Engine.h"
#include "TranslationTools.h"
#include "FleetViewWT.h"
#include <boost/format.hpp>


using namespace Wt;
using namespace boost;

FleetsView::FleetsView(Wt::WContainerWidget* parent, Engine& engine, Player::ID pid):
	WContainerWidget(parent),
	engine_(engine),
	playerId_(pid),
	fleetLayout_(nullptr),
	fleetsView_(nullptr),
	fleetView_(nullptr)
{
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	setLayout(layout);

	fleetsView_ = new WTableView(this);

	fleetsView_->setHeight(600);

	fleetsView_->setAlternatingRowColors(true);
	fleetsView_->clicked().connect(
	  this, &FleetsView::on_fleetTable_itemDoubleClicked);
	fleetsView_->setColumnWidth(0, 40);
	fleetsView_->setColumnWidth(1, 40);
	fleetsView_->setColumnWidth(2, 40);
	fleetsView_->setColumnWidth(3, 20);
	fleetsView_->setColumnWidth(4, 100);
	fleetsView_->setColumnWidth(5, 100);
	layout->addWidget(fleetsView_);

	fleetLayout_ = layout;

	fleetView_ = new FleetViewWT(this, engine_, playerId_);
	layout->addWidget(fleetView_);

	refresh();
}


template<typename T>
void addItem(Wt::WStandardItemModel* plModel, int row, int col, T const& value)
{
	Wt::WStandardItem* const item = new Wt::WStandardItem();
	plModel->setItem(row, col, item);
	item->setData(value, DisplayRole);
}


void FleetsView::refresh()
{
	std::vector<Fleet> const fleetList = engine_.getPlayerFleets(playerId_);
	Wt::WStandardItemModel* flModel = new Wt::WStandardItemModel(
	  (int)fleetList.size(), 6, this);
	flModel->setHeaderData(0, Horizontal, WString("X"), DisplayRole);
	flModel->setHeaderData(1, Horizontal, WString("Y"), DisplayRole);
	flModel->setHeaderData(2, Horizontal, WString("Z"), DisplayRole);
	flModel->setHeaderData(3, Horizontal, WString(""), DisplayRole);
	flModel->setHeaderData(4, Horizontal, WString(gettext("Contents")), DisplayRole);
	flModel->setHeaderData(5, Horizontal, WString(gettext("Ressources")), DisplayRole);
	int row = 0;
	for(Fleet const & fleet: fleetList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		item->setData(fleet.coord.X, DisplayRole);
		item->setData(fleet.id, UserRole);
		flModel->setItem(row, 0, item);

		addItem(flModel, row, 1, fleet.coord.Y);
		addItem(flModel, row, 2, fleet.coord.Z);
		addItem(flModel, row, 3, fleet.name);

		std::string content;
		for(Ship::Enum i = Ship::Enum(0); i < Ship::Count; i = Ship::Enum(i + 1))
		{
			if(fleet.shipList[i])
				content += str(boost::format("%1$c:%2%; ") %
				               getShipName(i) % fleet.shipList[i]);
		}
		addItem(flModel, row, 4, content);

		std::string ressStr;
		for(Ressource::Enum i = Ressource::Enum(0);
		    i < Ressource::Count;
		    i = Ressource::Enum(i + 1))
		{
			ressStr += str(boost::format("%1$c:%2%; ") %
			               getRessourceName(i) %
			               fleet.ressourceSet.tab[i]);
		}
		addItem(flModel, row, 5, ressStr);

		row += 1;
	}
	fleetsView_->setModel(flModel);

	WContainerWidget::refresh();
}


void FleetsView::on_fleetTable_itemDoubleClicked(
	WModelIndex const& index, WMouseEvent const&)
{
	WAbstractItemModel* model = fleetsView_->model();
	assert(model);
	Fleet::ID fleetID = any_cast<Fleet::ID>(model->data(index.row(), 0, UserRole));
	fleetView_->setFleet(fleetID);
}

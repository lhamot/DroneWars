#include "stdafx.h"

#include "Model.h"
#include "Engine.h"
#include "PlanetsView.h"
#include "PlanetViewWT.h"
#include "TranslationTools.h"
#include <boost/format.hpp>


using namespace Wt;
using namespace boost;


PlanetsView::PlanetsView(Wt::WContainerWidget* parent,
                         Engine& engine,
                         Player::ID pid):
	WContainerWidget(parent),
	engine_(engine),
	playerId_(pid),
	planetLayout_(nullptr),
	planetsView_(nullptr)
{
	WContainerWidget* titleCont = new WContainerWidget(this);
	titleCont->addStyleClass("manual");
	new WText(gettext("List of your planets"), titleCont);

	WContainerWidget* planetsTab = new WContainerWidget(this);
	planetsTab->setTabIndex(0);
	Wt::WHBoxLayout* layout = new Wt::WHBoxLayout();
	planetsTab->setLayout(layout);

	planetsView_ = new WTableView(planetsTab);

	planetsView_->setHeight(600);
	planetsView_->setRowHeight(100);
	planetsView_->setAlternatingRowColors(true);

	planetsView_->setAlternatingRowColors(true);
	planetsView_->clicked().connect(
	  this, &PlanetsView::on_planetTable_itemDoubleClicked);
	planetsView_->setColumnWidth(0, 100);
	layout->addWidget(planetsView_);

	planetLayout_ = layout;

	planetView_ = new PlanetViewWT(this, engine_);
	planetLayout_->addWidget(planetView_);

	refresh();
}


void PlanetsView::on_planetTable_itemDoubleClicked(
  WModelIndex const& index, WMouseEvent const&)
{
	WAbstractItemModel* model = planetsView_->model();
	assert(model);
	Coord const coord = any_cast<Coord>(model->data(index.row(), 0, UserRole));
	planetView_->setPlanet(coord);
}


template<typename T>
void addItem(Wt::WStandardItemModel* plModel, int row, int col, T const& value)
{
	Wt::WStandardItem* const item = new Wt::WStandardItem();
	plModel->setItem(row, col, item);
	item->setData(value, DisplayRole);
}


void PlanetsView::refresh()
{
	std::vector<Planet> planetList = engine_.getPlayerPlanets(playerId_);
	Wt::WStandardItemModel* plModel = new Wt::WStandardItemModel(
	  (int)planetList.size(), 4, planetsView_);
	plModel->setHeaderData(0, Horizontal, WString(gettext("View")), DisplayRole);
	plModel->setHeaderData(1, Horizontal, WString(gettext("Name")), DisplayRole);
	plModel->setHeaderData(2, Horizontal, WString(gettext("Coordinate")), DisplayRole);
	plModel->setHeaderData(3, Horizontal, WString(gettext("Ressources")), DisplayRole);

	int row = 0;
	for(Planet const & planet: planetList)
	{
		Wt::WStandardItem* item = new Wt::WStandardItem();
		std::string const link = PlanetViewWT::getPlanetImagePath(planet.coord, true);
		item->setIcon(link);
		item->setData(planet.coord, UserRole);
		plModel->setItem(row, 0, item);

		addItem(plModel, row, 1, planet.name);

		Coord const& coord = planet.coord;
		addItem(plModel, row, 2,
		        str(format("(%1%; %2%; %3%)") % coord.X % coord.Y % coord.Z));

		std::string ressStr;
		for(Ressource::Enum i = Ressource::Enum(0);
		    i < Ressource::Count;
		    i = Ressource::Enum(i + 1))
		{
			size_t const val = planet.ressourceSet.tab[i];
			ressStr += str(boost::format("%1$c:%2%; ") % getRessourceName(i) % val);
		}
		addItem(plModel, row, 3, ressStr);

		row += 1;
	}
	planetsView_->setModel(plModel);

	WContainerWidget::refresh();
}

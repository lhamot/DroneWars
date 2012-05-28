#include "MessageView.h"

#include <boost/foreach.hpp>
#include <Wt/WText>
#include <Wt/WTable>

#include "TranslationTools.h"


using namespace Wt;

MessageView::MessageView(WContainerWidget* parent,
                         Engine& eng,
                         Player::ID playerID,
                         Event::ID eventID):
	WContainerWidget(parent),
	engine_(eng),
	playerID_(playerID),
	eventID_(eventID)
{
	Player const& player = engine_.getPlayer(playerID_);
	auto iter = boost::find_if(player.eventList, [&](Event const & ev)
	{
		return ev.id == eventID_;
	});

	if(iter == player.eventList.end())
		return;

	Event const& ev = *iter;
	//std::cout << "Event::ID : " << eventID << " ReportID = " << ev.value << std::endl;

	new WText(getEventName(ev.type) + " - " + ev.comment, this);

	switch(ev.type)
	{
	case Event::FleetCodeError:
		break;
	case Event::FleetCodeExecError:
		break;
	case Event::PlanetCodeError:
		break;
	case Event::PlanetCodeExecError:
		break;
	case Event::Upgraded:
		break;
	case Event::ShipMade:
		break;
	case Event::PlanetHarvested:
		break;
	case Event::FleetWin:
	{
		renderFightReport(ev.value);
	}
	break;
	case Event::FleetDraw:
		break;
	case Event::FleetsGather:
		break;
	case Event::PlanetColonized:
		break;
	case Event::FleetLose:
	{
		renderFightReport(ev.value);
	}
	break;
	case Event::FleetDrop:
		break;
	default:
		BOOST_THROW_EXCEPTION(std::out_of_range("Bad event type"));
	};

	static_assert(Event::FleetDrop + 1 == Event::Count, "Missing event type in MessageView");
}

std::string getContentString(Fleet const& fleet)
{
	if(fleet.shipList.size() != Ship::Count)
		BOOST_THROW_EXCEPTION(std::logic_error("fleet.shipList.size() != Ship::Count"));
	std::string result;
	//BOOST_FOREACH(size_t shipCount, fleet.shipList)
	for(Ship::Enum shipType = Ship::Enum(0); shipType < Ship::Count; shipType = Ship::Enum(shipType + 1))
	{
		if(fleet.shipList[shipType])
			result += getShipName(shipType)[0] + std::string(":") + boost::lexical_cast<std::string>(fleet.shipList[shipType]) + ";";
	}
	if(false == result.empty())
		result.pop_back();
	return result;
}

void MessageView::renderFightReport(size_t id)
{
	FightReport fightReport = engine_.getFightReport(id);
	BOOST_FOREACH(FleetReport const & fleetReport, fightReport)
	{
		Wt::WTable* table = new Wt::WTable(this);
		table->elementAt(0, 0)->addWidget(new Wt::WText("Fleet name :"));
		table->elementAt(0, 1)->addWidget(new Wt::WText(boost::lexical_cast<std::string>(fleetReport.fleetsBefore.id)));

		table->elementAt(1, 0)->addWidget(new Wt::WText("Has fight :"));
		if(fleetReport.hasFight)
			table->elementAt(1, 1)->addWidget(new Wt::WText("Yes"));
		else
			table->elementAt(1, 1)->addWidget(new Wt::WText("No"));

		table->elementAt(2, 0)->addWidget(new Wt::WText("Is dead :"));
		if(fleetReport.isDead)
			table->elementAt(2, 1)->addWidget(new Wt::WText("Yes"));
		else
			table->elementAt(2, 1)->addWidget(new Wt::WText("No"));

		table->elementAt(3, 0)->addWidget(new Wt::WText("Start :"));
		std::string fleetContentBefore;
		table->elementAt(3, 1)->addWidget(new Wt::WText(getContentString(fleetReport.fleetsBefore)));
		table->elementAt(4, 0)->addWidget(new Wt::WText("End :"));
		std::string fleetContentAfter;
		table->elementAt(4, 1)->addWidget(new Wt::WText(getContentString(fleetReport.fleetsAfter)));

		table->elementAt(5, 0)->addWidget(new Wt::WText("Report ID = " + boost::lexical_cast<std::string>(id)));

		//bool isDead;
		//bool hasFight;
		//std::set<size_t> enemySet; //par index dans le FightReport
		//Fleet fleetsBefore;
		//Fleet fleetsAfter;
	}
}
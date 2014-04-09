//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "EngineServerHandler.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include "Skills.h"
#include "UnivManip.h"

using namespace boost;
using namespace std;

using namespace log4cplus;
//! Logger dédié au EngineServerHandler
static Logger logger =
  Logger::getInstance(LOG4CPLUS_TEXT("EngineServerHandler"));

//! @cond Doxygen_Suppress
bool ndw::Coord::operator < (const ndw::Coord& b) const
{
	if(X < b.X)
		return true;
	else if(X > b.X)
		return false;
	else if(Y < b.Y)
		return true;
	else if(Y > b.Y)
		return false;
	else
		return Z < b.Z;
}
//! @endcond

namespace ndw
{
//! operateur d'écriture d'un ndw::Coord dans un flux
ostream& operator << (ostream& os, ndw::Coord const& coord)
{
	os << "(" << coord.X << "," << coord.Y << "," << coord.Z << ")";
	return os;
}
}


namespace
{
//! Foncteur qui applique un boost::numeric_cast sans spécifier le type
template<typename I>
struct NumerciCast
{
	I value;               //!< Valeur pas encore convertie
	char const* valueName; //!< Nom de la valeur a convertir (pour log)

	//! Applique un boost::numeric_cast du type I vers O
	template<typename O>
	operator O()
	{
		try
		{
			return numeric_cast<O>(value);
		}
		catch(...)
		{
			LOG4CPLUS_ERROR(logger, "value: " << value << " valueName : " << valueName);
			throw;
		}
	}
};


//! Crée un NumerciCast pour faire un numeric_cast sans spécifier le type
template<typename I>
NumerciCast<I> numCast(
  I value,                    //!< Valeur a caster
  char const* const valueName //!< Nom de la valeur (pour log)
)
{
	NumerciCast<I> cast = {value, valueName};
	return cast;
}

//! Crée un NumerciCast pour faire un numeric_cast sans spécifier le type
//! @param value Valeur a caster
#define NUMCAST(value) numCast(value, #value)

//! Convertie un Coord en ndw::Coord pour transfert par thrift
ndw::Coord coordToThrift(Coord const& fleet)
{
	ndw::Coord res;
	res.X = fleet.X;
	res.Y = fleet.Y;
	res.Z = fleet.Z;
	return res;
}


//! Convertie un RessourceSet en ndw::RessourceSet pour transfert par thrift
ndw::RessourceSet ressourceToThrift(RessourceSet const& ress)
{
	ndw::RessourceSet res;
	res.tab.reserve(ress.tab.size());
	for(size_t value : ress.tab)
		res.tab.push_back(NUMCAST(value));
	return res;
}


//! Convertie un FleetTask en ndw::FleetTask pour transfert par thrift
ndw::FleetTask fleetTaskToThrift(FleetTask const& task)
{
	ndw::FleetTask res;
	res.type = static_cast<ndw::FleetTask_Enum::type>(task.type);
	res.lauchTime = NUMCAST(task.lauchTime);
	res.duration = NUMCAST(task.duration);
	res.position = coordToThrift(task.position);
	res.expired = task.expired;
	return res;
}


//! Convertie un PlanetTask en ndw::PlanetTask pour transfert par thrift
ndw::PlanetTask planetTaskToThrift(PlanetTask const& task)
{
	ndw::PlanetTask res;
	res.type = static_cast<ndw::PlanetTask_Enum::type>(task.type);
	res.value = NUMCAST(task.value);
	res.value2 = NUMCAST(task.value2);
	res.lauchTime = NUMCAST(task.lauchTime);
	res.duration = NUMCAST(task.duration);
	res.expired = task.expired;
	return res;
}


//! Convertie un Event en ndw::Event pour transfert par thrift
ndw::Event eventToThrift(Event const& event)
{
	ndw::Event res;
	res.id = NUMCAST(event.id);
	res.time = NUMCAST(event.time);
	res.type = static_cast<ndw::Event_Type::type>(event.type);
	res.comment = event.comment;
	res.value = NUMCAST(event.value);
	res.value2 = NUMCAST(event.value2);
	res.viewed = event.viewed;
	return res;
}


//! Convertie un Fleet en ndw::Fleet pour transfert par thrift
ndw::Fleet fleetToThrift(Fleet const& fleet, Player const& player)
{
	if(fleet.playerId == 0)
		BOOST_THROW_EXCEPTION(std::logic_error("playerId == 0!!"));

	ndw::Fleet result;
	result.id = NUMCAST(fleet.id);
	result.playerId = NUMCAST(fleet.playerId);
	result.coord = coordToThrift(fleet.coord);
	result.origin = coordToThrift(fleet.origin);
	result.name = fleet.name;
	result.playerLogin = player.login;
	result.allianceID = player.allianceID;

	result.shipList.reserve(fleet.shipList.size());
	for(size_t value : fleet.shipList)
		result.shipList.push_back(NUMCAST(value));

	result.ressourceSet = ressourceToThrift(fleet.ressourceSet);

	result.taskQueue.reserve(fleet.taskQueue.size());
	for(FleetTask const& task : fleet.taskQueue)
		result.taskQueue.push_back(fleetTaskToThrift(task));

	return result;
}


//! Convertie un Planet en ndw::Planet pour transfert par thrift
ndw::Planet planetToThrift(Planet const& planet, Player const* player)
{
	ndw::Planet res;

	res.name = planet.name;
	res.coord = coordToThrift(planet.coord);
	res.playerId = NUMCAST(planet.playerId);
	res.buildingList.reserve(planet.buildingList.size());
	if(player)
	{
		res.playerLogin = player->login;
		res.allianceID = player->allianceID;
	}
	else
		res.allianceID = 0;
	for(size_t value : planet.buildingList)
		res.buildingList.push_back(NUMCAST(value));
	res.taskQueue.reserve(planet.taskQueue.size());
	transform(planet.taskQueue,
	          back_inserter(res.taskQueue),
	          planetTaskToThrift);
	res.ressourceSet = ressourceToThrift(planet.ressourceSet);
	res.cannonTab.reserve(planet.cannonTab.size());
	for(size_t value : planet.cannonTab)
		res.cannonTab.push_back(NUMCAST(value));
	res.hangar.reserve(planet.hangar.size());
	for(size_t value : planet.hangar)
		res.hangar.push_back(NUMCAST(value));

	return res;
}


//! Convertie un CodeData en ndw::CodeData pour transfert par thrift
void codeDataCppToThrift(CodeData const& in, ndw::CodeData& out)
{
	out.blocklyCode = in.blocklyCode;
	out.blocklyCodeDate = NUMCAST(in.blocklyCodeDate);
	out.code = in.code;
	out.codeDate = NUMCAST(in.codeDate);
	out.lastError = in.lastError;
}


//! Convertie un Skill en ndw::Skill pour transfert par thrift
ndw::Skill skillToThrift(size_t skillID, Player const& player)
{
	ndw::Skill outSkill;
	outSkill.level = player.skilltab[skillID];
	ISkill const& skill = *Skill::List[skillID];
	outSkill.name = skill.getName();
	outSkill.canUpdate = skill.canUpgrade(player);
	outSkill.cost = NUMCAST(skill.skillCost(player.skilltab[skillID]));
	outSkill.effectMessage = skill.effectMessage(player);
	Player copy = player;
	copy.skilltab[skillID] += 1;
	outSkill.nextLevelMessage = skill.effectMessage(copy);
	return outSkill;
}


//! Convertie un Player en ndw::Player pour transfert par thrift
ndw::Player playerToThrift(Player const& player)
{
	ndw::Player outPlayer;
	outPlayer.id          = NUMCAST(player.id);
	outPlayer.login       = player.login;
	outPlayer.mainPlanet = coordToThrift(player.mainPlanet);
	outPlayer.score = NUMCAST(player.score);
	outPlayer.allianceID = NUMCAST(player.allianceID);
	outPlayer.experience = player.experience;
	outPlayer.skillpoints = player.skillpoints;
	outPlayer.skilltab.reserve(Skill::Count);
	outPlayer.unreadMessagesCount = NUMCAST(player.unreadMessageCount);
	for(size_t skillID = 0; skillID < Skill::Count; ++skillID)
		outPlayer.skilltab.push_back(skillToThrift(skillID, player));
	outPlayer.allianceName = player.allianceName;
	return outPlayer;
}


//! Convertie un FleetReport en ndw::FleetReport pour transfert par thrift
ndw::FleetReport fleetReportToThrift(Report<Fleet> const& fleetReport,
                                     Player const& player)
{
	ndw::FleetReport result;
	result.isDead = fleetReport.isDead;
	result.hasFight = fleetReport.hasFight;
	result.experience = fleetReport.experience;
	result.wantEscape = fleetReport.wantEscape;
	result.escapeProba = fleetReport.escapeProba;
	for(intptr_t id : fleetReport.enemySet)
		result.enemySet.insert(numeric_cast<int32_t>(id));
	result.fightInfo.before =
	  fleetToThrift(fleetReport.fightInfo.before, player);
	result.fightInfo.after =
	  fleetToThrift(fleetReport.fightInfo.after, player);
	return result;
}


//! Convertie un PlanetReport en ndw::PlanetReport pour transfert par thrift
ndw::PlanetReport planetReportToThrift(Report<Planet> const& planetReport,
                                       Player const* player)
{
	ndw::PlanetReport result;
	result.isDead = planetReport.isDead;
	result.hasFight = planetReport.hasFight;
	result.experience = planetReport.experience;
	for(intptr_t id : planetReport.enemySet)
		result.enemySet.insert(numeric_cast<int32_t>(id));
	result.fightInfo.before =
	  planetToThrift(planetReport.fightInfo.before, player);
	result.fightInfo.after =
	  planetToThrift(planetReport.fightInfo.after, player);
	return result;
}


//! Convertie un FightReport en ndw::FightReport pour transfert par thrift
ndw::FightReport fightReportToThrift(
  FightReport const& report,
  std::map<Player::ID, Player> const& playerMap)
{
	ndw::FightReport result;
	result.fleetList.reserve(report.fleetList.size());
	for(Report<Fleet> const& fleetRep : report.fleetList)
	{
		Player const player =
		  MAP_FIND(playerMap, fleetRep.fightInfo.before.playerId)->second;
		result.fleetList.push_back(fleetReportToThrift(fleetRep, player));
	}
	result.hasPlanet = report.hasPlanet;
	if(result.hasPlanet != bool(report.planet))
		BOOST_THROW_EXCEPTION(
		  logic_error("Unconsistent FightReport::hasPlanet value"));
	if(report.planet)
	{
		auto iter = playerMap.find(report.planet->fightInfo.before.playerId);
		Player const* player =
		  iter == playerMap.end() ? nullptr : &iter->second;
		result.__set_planet(planetReportToThrift(report.planet.get(), player));
	}
	return result;
}

} //Namespace anonyme


EngineServerHandler::EngineServerHandler(
  DataBase::ConnectionInfo const& connInfo,
  size_t minRoundDuration) :
	engine_(connInfo, minRoundDuration),
	database_(connInfo)
{
}


void EngineServerHandler::start()
{
}


void EngineServerHandler::stop()
{
}


bool EngineServerHandler::addPlayer(const string& login,
                                    const string& password)
{
	LOG4CPLUS_TRACE(logger, "login: " << login << " password : " << password);
	if(login.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("login"));
	if(password.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("password"));

	std::vector<std::string> codes;
	getNewPlayerCode(codes);
	Player::ID const pid = database_.addPlayer(login, password, codes);
	if(pid != Player::NoId)
	{
		engine_.addPlayer(pid);
		return true;
	}
	else
		return false;
}


//! Trie un range d'objet en comparent un attribut donnée de ces objets
template<class RandomAccessRange, class Attribute>
RandomAccessRange& sortOnAttr(RandomAccessRange& rng, bool asc, Attribute attr)
{
	typedef typename RandomAccessRange::value_type value_type;
	if(asc)
		range::sort(rng, [&](value_type const & a, value_type const & b)
	{
		return attr(a) < attr(b);
	});
	else
		range::sort(rng, [&](value_type const & a, value_type const & b)
	{
		return attr(a) > attr(b);
	});
	return rng;
}


//! Trie un range de planète ou flotte, en fonction d'un critère donné
template<typename Range>
void sortOnType(Range& rg, ndw::Sort_Type::type sortType, const bool asc)
{
	typedef typename Range::value_type FoP; //FleetOrPlanet
	switch(sortType)
	{
	case ndw::Sort_Type::Name:
		sortOnAttr(rg, asc, [](FoP const & elt) {return elt.name;});
		break;
	case ndw::Sort_Type::X:
		sortOnAttr(rg, asc, [](FoP const & elt) {return elt.coord.X;});
		break;
	case ndw::Sort_Type::Y:
		sortOnAttr(rg, asc, [](FoP const & elt) {return elt.coord.Y;});
		break;
	case ndw::Sort_Type::Z:
		sortOnAttr(rg, asc, [](FoP const & elt) {return elt.coord.Z;});
		break;
	case ndw::Sort_Type::M:
		sortOnAttr(rg, asc, [](FoP const & e) {return e.ressourceSet.tab[0];});
		break;
	case ndw::Sort_Type::C:
		sortOnAttr(rg, asc, [](FoP const & e) {return e.ressourceSet.tab[1];});
		break;
	case ndw::Sort_Type::L:
		sortOnAttr(rg, asc, [](FoP const & e) {return e.ressourceSet.tab[2];});
		break;
	default:
		BOOST_THROW_EXCEPTION(runtime_error("Unconsistent Sort_Type"));
	};
}

template<typename Range>
void sortFleetOnType(Range& rg, ndw::Sort_Type::type sortType, const bool asc, const int32_t value)
{
	switch(sortType)
	{
	case ndw::Sort_Type::S:
		sortOnAttr(rg, asc, [&](Fleet const & e) {return e.shipList.at(value); });
		break;
	case ndw::Sort_Type::C:
	case ndw::Sort_Type::B:
		break;
	default:
		sortOnType(rg, sortType, asc);
	};
}


template<typename Range>
void sortPlanetOnType(Range& rg, ndw::Sort_Type::type sortType, const bool asc, const int32_t value)
{
	switch(sortType)
	{
	case ndw::Sort_Type::S:
		break;
	case ndw::Sort_Type::C:
		sortOnAttr(rg, asc, [&](Planet const & e) {return e.cannonTab.at(value);});
		break;
	case ndw::Sort_Type::B:
		sortOnAttr(rg, asc, [&](Planet const & e) {return e.buildingList.at(value);});
		break;
	default:
		sortOnType(rg, sortType, asc);
	};
}


void EngineServerHandler::getPlayerFleets(
  ndw::FleetList& _return,
  const ndw::Player_ID pid,
  const int32_t beginIndexC,
  const int32_t endIndexC,
  const ndw::Sort_Type::type sortType,
  const bool asc,
  const int32_t value)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid <<
	                " beginIndexC : " << beginIndexC <<
	                " endIndexC : " << endIndexC <<
	                " sortType : " << sortType <<
	                " asc : " << asc);
	if(beginIndexC >= endIndexC || beginIndexC < 0)
		BOOST_THROW_EXCEPTION(runtime_error("Unconsistent index"));
	size_t beginIndex = NUMCAST(beginIndexC);
	size_t endIndex = NUMCAST(endIndexC);
	auto fleetList = engine_.getPlayerFleets(pid);
	if(endIndex > fleetList.size())
		endIndex = fleetList.size();
	if(beginIndex > fleetList.size())
		beginIndex = fleetList.size();

	sortFleetOnType(fleetList, sortType, asc, value);

	if(endIndex > fleetList.size())
		BOOST_THROW_EXCEPTION(logic_error("endIndex > fleetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(logic_error("beginIndex > endIndex"));

	_return.fleetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(fleetList.begin() + beginIndex,
	                                     fleetList.begin() + endIndex);
	Player const player = database_.getPlayer(pid);
	set<Coord, CompCoord> fleetCoordSet;
	for(Fleet const& fleet : pageRange)
	{
		_return.fleetList.push_back(fleetToThrift(fleet, player));
		fleetCoordSet.insert(fleet.coord);
	}
	vector<Coord> fleetCoordVect(fleetCoordSet.begin(), fleetCoordSet.end());
	auto planetList = engine_.getPlanets(fleetCoordVect);
	for(Planet const& planet : planetList)
		_return.planetList.push_back(planetToThrift(planet, &player));

	_return.fleetCount = NUMCAST(fleetList.size());
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerPlanets(
  ndw::PlanetList& _return,
  const ndw::Player_ID pid,
  const int32_t beginIndexC,
  const int32_t endIndexC,
  const  ndw::Sort_Type::type sortType,
  const bool asc,
  const int32_t value)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid <<
	                " beginIndexC : " << beginIndexC <<
	                " endIndexC : " << endIndexC <<
	                " sortType : " << sortType <<
	                " asc : " << asc);
	if(beginIndexC >= endIndexC || beginIndexC < 0)
		BOOST_THROW_EXCEPTION(runtime_error("Unconsistent index"));
	size_t beginIndex = NUMCAST(beginIndexC);
	size_t endIndex = NUMCAST(endIndexC);
	auto planetList = engine_.getPlayerPlanets(pid);
	if(endIndex > planetList.size())
		endIndex = planetList.size();
	if(beginIndex > planetList.size())
		beginIndex = planetList.size();

	sortPlanetOnType(planetList, sortType, asc, value);

	if(endIndex > planetList.size())
		BOOST_THROW_EXCEPTION(logic_error("endIndex > planetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(logic_error("beginIndex > endIndex"));

	_return.planetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(planetList.begin() + beginIndex,
	                                     planetList.begin() + endIndex);
	Player const player = database_.getPlayer(pid);
	for(Planet const& planet : pageRange)
		_return.planetList.push_back(planetToThrift(planet, &player));
	_return.planetCount = NUMCAST(planetList.size());
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerFleetCode(
  const ndw::Player_ID pid,
  const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > CodeData::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("Code size too long"));
	database_.addScript(pid, CodeData::Fleet, code);
	engine_.reloadPlayer(pid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerPlanetCode(
  const ndw::Player_ID pid,
  const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > CodeData::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("Code size too long"));
	database_.addScript(pid, CodeData::Planet, code);
	engine_.reloadPlayer(pid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerFleetBlocklyCode(
  const ndw::Player_ID pid,
  const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > CodeData::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(InvalidData("Code size too long"));
	database_.addBlocklyCode(pid, CodeData::Fleet, code);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerPlanetBlocklyCode(
  const ndw::Player_ID pid,
  const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > CodeData::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(InvalidData("Code size too long"));
	database_.addBlocklyCode(pid, CodeData::Planet, code);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerFleetCode(
  ndw::CodeData& ret,
  const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	codeDataCppToThrift(database_.getPlayerCode(pid, CodeData::Fleet), ret);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerPlanetCode(
  ndw::CodeData& ret,
  const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	codeDataCppToThrift(database_.getPlayerCode(pid, CodeData::Planet), ret);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayers(vector<ndw::Player>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	vector<Player> players = database_.getPlayers();
	_return.reserve(players.size());
	transform(players, back_inserter(_return), playerToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayer(ndw::Player& outPlayer,
                                    const ndw::Player_ID pid)
{
	//! @todo: Séparer en deux requetes differentes
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	outPlayer = playerToThrift(database_.getPlayer(pid));
	CodeData const fleetCode = database_.getPlayerCode(pid, CodeData::Fleet);
	codeDataCppToThrift(fleetCode, outPlayer.fleetsCode);
	CodeData const planetCode = database_.getPlayerCode(pid, CodeData::Planet);
	codeDataCppToThrift(planetCode, outPlayer.planetsCode);
	map<string, size_t> const levelMap = database_.getTutoDisplayed(pid);
	for(auto tutoNVP : levelMap)
		outPlayer.tutoDisplayed[tutoNVP.first] = NUMCAST(tutoNVP.second);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlanet(vector<ndw::Planet>& _return,
                                    const ndw::Coord& ndwCoord)
{
	LOG4CPLUS_TRACE(logger, "ndwCoord : " << ndwCoord);
	Coord const coord(
	  numeric_cast<Coord::Value>(ndwCoord.X),
	  numeric_cast<Coord::Value>(ndwCoord.Y),
	  numeric_cast<Coord::Value>(ndwCoord.Z));
	optional<Planet> planet = engine_.getPlanet(coord);
	if(planet)
	{
		if(planet->playerId)
		{
			Player const player = database_.getPlayer(planet->playerId);
			_return.push_back(planetToThrift(*planet, &player));
		}
		else
			_return.push_back(planetToThrift(*planet, nullptr));
		vector<Event> events =
		  database_.getPlanetEvents(planet->playerId, coord);
		_return.front().eventList.reserve(events.size());
		for(Event const& ev : events)
			_return.front().eventList.push_back(eventToThrift(ev));
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFleet(ndw::Fleet& _return,
                                   const ndw::Fleet_ID fid)
{
	LOG4CPLUS_TRACE(logger, "fid : " << fid);
	//! @todo: Gerer proprement l'absence de flotte
	boost::optional<Fleet> const optFleet = engine_.getFleet(fid);
	if(!optFleet)
		BOOST_THROW_EXCEPTION(std::runtime_error("Fleet doesn't exist"));
	Player const player = database_.getPlayer(optFleet->playerId);
	_return = fleetToThrift(*optFleet, player);

	vector<Event> events = database_.getFleetEvents(_return.playerId, fid);
	_return.eventList.reserve(events.size());
	for(Event const& ev : events)
		_return.eventList.push_back(eventToThrift(ev));

	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::logPlayer(ndw::OptionalPlayer& _return,
                                    const string& login,
                                    const string& password)
{
	LOG4CPLUS_TRACE(logger, "login : " << login << " password : " << password);
	optional<Player> optPlayer = database_.getPlayer(login, password);
	if(optPlayer)
	{
		_return.__set_player(playerToThrift(optPlayer.get()));
		ndw::Player& outPlayer = _return.player;
		CodeData const fleetCode =
		  database_.getPlayerCode(_return.player.id, CodeData::Fleet);
		codeDataCppToThrift(fleetCode, outPlayer.fleetsCode);
		CodeData const planetCode =
		  database_.getPlayerCode(_return.player.id, CodeData::Planet);
		codeDataCppToThrift(planetCode, outPlayer.planetsCode);
		for(auto tutoNVP : database_.getTutoDisplayed(outPlayer.id))
			outPlayer.tutoDisplayed[tutoNVP.first] = NUMCAST(tutoNVP.second);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::incrementTutoDisplayed(
  ndw::Player_ID pid,
  string const& tutoName,
  int32_t const value)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " tutoName : " << tutoName);
	database_.incrementTutoDisplayed(pid, tutoName, value);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFightReport(ndw::FightReport& _return,
    const int32_t id)
{
	LOG4CPLUS_TRACE(logger, "id : " << id);
	FightReport fr = database_.getFightReport(NUMCAST(id));
	_return = fightReportToThrift(fr, database_.getPlayerMap());
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getTimeInfo(ndw::TimeInfo& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	TimeInfo info = engine_.getTimeInfo();
	_return.roundDuration = info.roundDuration;
	_return.univTime = info.univTime;
	LOG4CPLUS_TRACE(logger, "exit");
}


bool EngineServerHandler::eraseAccount(const ndw::Player_ID pid,
                                       const string& password)
{
	LOG4CPLUS_TRACE(logger, "password : " << password);
	Player player = database_.getPlayer(pid);
	if(database_.getPlayer(player.login, password).is_initialized())
	{
		database_.eraseAccount(pid);
		engine_.reloadPlayer(pid);
		LOG4CPLUS_TRACE(logger, "true");
		return true;
	}
	else
	{
		LOG4CPLUS_TRACE(logger, "false");
		return false;
	}
}


void EngineServerHandler::getPlayerEvents(
  vector<ndw::Event>& _return,
  const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	vector<Event> events = database_.getPlayerEvents(pid);
	_return.reserve(events.size());
	transform(events, back_inserter(_return), eventToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}


bool EngineServerHandler::buySkill(const ndw::Player_ID pid,
                                   const int16_t skillID)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " skillID : " << skillID);
	if(skillID >= Skill::Count || skillID < 0)
		return false;
	bool const done = database_.buySkill(pid, skillID);
	LOG4CPLUS_TRACE(logger, "exit " << done);
	return done;
}


void EngineServerHandler::getBuildingsInfo(vector<ndw::Building>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	_return.reserve(Building::Count);
	int32_t index = 0;
	for(Building const& b : Building::List)
	{
		ndw::Building newBu;
		newBu.index = index++;
		newBu.price = ressourceToThrift(b.price);
		newBu.coef = b.coef;
		_return.push_back(newBu);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getCannonsInfo(vector<ndw::Cannon>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	_return.reserve(Cannon::Count);
	int32_t index = 0;
	for(Cannon const& c : Cannon::List)
	{
		ndw::Cannon newCa;
		newCa.index = index++;
		newCa.price = ressourceToThrift(c.price);
		newCa.life = NUMCAST(c.life);
		newCa.power = NUMCAST(c.power);
		_return.push_back(newCa);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getShipsInfo(vector<ndw::Ship>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	_return.reserve(Ship::Count);
	for(Ship const& s : Ship::List)
	{
		ndw::Ship newSh;
		newSh.index = NUMCAST(_return.size());
		newSh.price = ressourceToThrift(s.price);
		newSh.life = NUMCAST(s.life);
		newSh.power = NUMCAST(s.power);
		_return.push_back(newSh);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


//*******************************  Messages  **********************************

void EngineServerHandler::addMessage(
  const ndw::Player_ID sender,
  const ndw::Player_ID recipient,
  const std::string& suject,
  const std::string& message)
{
	LOG4CPLUS_TRACE(logger, "server:" << sender <<
	                " recipient:" << recipient <<
	                " suject:" << suject <<
	                " message:" << message);
	database_.addMessage(sender, recipient, suject, message);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getMessages(
  std::vector<ndw::Message>& _return,
  const ndw::Player_ID recipient)
{
	LOG4CPLUS_TRACE(logger, "recipient:" << recipient);
	std::vector<Message> messages = database_.getMessages(recipient);
	_return.reserve(messages.size());
	for(Message const& message : messages)
	{
		ndw::Message newMess;
		newMess.id = message.id;
		newMess.sender = message.sender;
		newMess.recipient = message.recipient;
		newMess.time = NUMCAST(message.time);
		newMess.subject = message.subject;
		newMess.message = message.message;
		newMess.senderLogin = message.senderLogin;
		LOG4CPLUS_TRACE(logger, "id:" << message.id <<
		                " sender:" << message.recipient <<
		                " time:" << message.time <<
		                " subject:" << message.subject <<
		                " message:" << message.message <<
		                " senderLogin:" << message.senderLogin);
		_return.push_back(newMess);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::eraseMesage(const ndw::Message_ID mid)
{
	LOG4CPLUS_TRACE(logger, "message.id:" << mid);
	database_.eraseMesage(NUMCAST(mid));
	LOG4CPLUS_TRACE(logger, "exit");
}


//*****************************  Friendship  **********************************

void EngineServerHandler::addFriendshipRequest(
  const ndw::Player_ID playerA,
  const ndw::Player_ID playerB)
{
	LOG4CPLUS_TRACE(logger, "playerA:" << playerA << " playerB:" << playerB);
	database_.addFriendshipRequest(playerA, playerB);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::acceptFriendshipRequest(
  const ndw::Player_ID playerA,
  const ndw::Player_ID playerB,
  const bool accept)
{
	LOG4CPLUS_TRACE(logger, "playerA:" << playerA <<
	                " playerB:" << playerB <<
	                " accept:" << accept);
	database_.acceptFriendshipRequest(playerA, playerB, accept);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::closeFriendship(
  const ndw::Player_ID playerA,
  const ndw::Player_ID playerB)
{
	LOG4CPLUS_TRACE(logger, "playerA:" << playerA << " playerB:" << playerB);
	database_.closeFriendship(playerA, playerB);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFriends(
  std::vector<ndw::Player>& _return,
  const ndw::Player_ID player)
{
	LOG4CPLUS_TRACE(logger, "player:" << player);
	boost::transform(database_.getFriends(player),
	                 back_inserter(_return),
	                 playerToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFriendshipRequest(
  ndw::FriendshipRequests& ret,
  const ndw::Player_ID player)
{
	LOG4CPLUS_TRACE(logger, "player:" << player);
	FriendshipRequests requ = database_.getFriendshipRequest(player);
	transform(requ.received, back_inserter(ret.received), playerToThrift);
	transform(requ.sent, back_inserter(ret.sent), playerToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}


//*****************************  Alliance  ************************************


ndw::Alliance_ID EngineServerHandler::addAlliance(
  const ndw::Player_ID pid,
  const std::string& name,
  const std::string& descri)
{
	LOG4CPLUS_TRACE(logger, "pid:" << pid <<
	                " name:" << name <<
	                " descri:" << descri);
	return database_.addAlliance(pid, name, descri);
}


void EngineServerHandler::getAlliance(ndw::Alliance& _return,
                                      const ndw::Alliance_ID aid)
{
	LOG4CPLUS_TRACE(logger, "allianceID:" << aid);
	Alliance al = database_.getAlliance(aid);
	_return.id          = al.id;
	_return.masterID    = al.masterID;
	_return.name        = al.name;
	_return.description = al.description;
	_return.masterLogin = al.masterLogin;
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::updateAlliance(const ndw::Alliance& al)
{
	LOG4CPLUS_TRACE(logger, "allianceID:" << al.id);
	database_.updateAlliance(
	  Alliance(al.id, al.masterID, al.name, al.description, al.masterLogin));
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::transfertAlliance(
  const ndw::Alliance_ID aid,
  const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "allianceID:" << aid << " playerID:" << pid);
	database_.transfertAlliance(aid, pid);
	LOG4CPLUS_TRACE(logger, "exit");
}



void EngineServerHandler::eraseAlliance(const ndw::Alliance_ID aid)
{
	LOG4CPLUS_TRACE(logger, "allianceID:" << aid);
	database_.eraseAlliance(aid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::joinAlliance(const ndw::Player_ID pid,
                                       const ndw::Alliance_ID aid)
{
	LOG4CPLUS_TRACE(logger, "playerID:" << pid << " allianceID:" << aid);
	database_.joinAlliance(pid, aid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::quitAlliance(const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "playerID:" << pid);
	database_.quitAlliance(pid);
	LOG4CPLUS_TRACE(logger, "exit");
}


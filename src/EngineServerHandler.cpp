#include "stdafx.h"
#include "EngineServerHandler.h"
#include <algorithm>

using namespace boost;

using namespace log4cplus;
static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("EngineServerHandler"));


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

std::ostream& operator << (std::ostream& os, ndw::Coord const& coord)
{
	os << "(" << coord.X << "," << coord.Y << "," << coord.Z << ")";
	return os;
}


namespace
{

template<typename I>
struct NumerciCast
{
	I value;

	template<typename O>
	operator O()
	{
		return numeric_cast<O>(value);
	}
};

template<typename I>
NumerciCast<I> numCast(I value)
{
	NumerciCast<I> cast = {value};
	return cast;
}

ndw::Coord coordToThrift(Coord const& fleet)
{
	ndw::Coord res;
	res.X = fleet.X;
	res.Y = fleet.Y;
	res.Z = fleet.Z;
	return res;
}

ndw::RessourceSet ressourceToThrift(RessourceSet const& ress)
{
	ndw::RessourceSet res;
	res.tab.reserve(ress.tab.size());
	for(size_t value: ress.tab)
		res.tab.push_back(numCast(value));
	return res;
}

ndw::FleetTask fleetTaskToThrift(FleetTask const& task)
{
	ndw::FleetTask res;
	res.type = static_cast<ndw::FleetTask_Enum::type>(task.type);
	res.lauchTime = numCast(task.lauchTime);
	res.duration = numCast(task.duration);
	res.position = coordToThrift(task.position);
	res.expired = task.expired;
	return res;
}


ndw::PlanetTask planetTaskToThrift(PlanetTask const& task)
{
	ndw::PlanetTask res;
	res.type = static_cast<ndw::PlanetTask_Enum::type>(task.type);
	res.value = numCast(task.value);
	res.value2 = numCast(task.value2);
	res.lauchTime = numCast(task.lauchTime);
	res.duration = numCast(task.duration);
	//res.startCost = task.startCost;
	res.expired = task.expired;
	return res;
}

ndw::Event eventToThrift(Event const& event)
{
	ndw::Event res;
	res.id = numCast(event.id);
	res.time = numCast(event.time);
	res.type = static_cast<ndw::Event_Type::type>(event.type);
	res.comment = event.comment;
	res.value = numCast(event.value);
	res.viewed = event.viewed;
	return res;
}


ndw::Fleet fleetToThrift(Fleet const& fleet)
{
	ndw::Fleet result;
	result.id = numCast(fleet.id);
	result.playerId = numCast(fleet.playerId);
	result.coord = coordToThrift(fleet.coord);
	//std::cout << result.coord.X << " " << result.coord.Y << " " << result.coord.Z << std::endl;
	result.origin = coordToThrift(fleet.origin);
	result.name = fleet.name;

	result.shipList.reserve(fleet.shipList.size());
	for(size_t value: fleet.shipList)
		result.shipList.push_back(numCast(value));

	result.ressourceSet = ressourceToThrift(fleet.ressourceSet);

	result.taskQueue.reserve(fleet.taskQueue.size());
	for(FleetTask const & task: fleet.taskQueue)
		result.taskQueue.push_back(fleetTaskToThrift(task));

	result.eventList.reserve(fleet.eventList.size());
	for(Event const & event: fleet.eventList)
		result.eventList.push_back(eventToThrift(event));
	return result;
}

ndw::Planet planetToThrift(Planet const& planet)
{
	ndw::Planet res;

	res.name = planet.name;
	res.coord = coordToThrift(planet.coord);
	res.playerId = numCast(planet.playerId);
	res.buildingList.reserve(planet.buildingList.size());
	for(size_t value: planet.buildingList)
		res.buildingList.push_back(numCast(value));
	res.taskQueue.reserve(planet.taskQueue.size());
	range::transform(planet.taskQueue, back_inserter(res.taskQueue), planetTaskToThrift);
	res.ressourceSet = ressourceToThrift(planet.ressourceSet);
	res.eventList.reserve(planet.eventList.size());
	range::transform(planet.eventList, back_inserter(res.eventList), eventToThrift);
	res.cannonTab.reserve(planet.cannonTab.size());
	for(size_t value: planet.cannonTab)
		res.cannonTab.push_back(numCast(value));

	return res;
}

void codeDataCppToThrift(CodeData const& in, ndw::CodeData& out)
{
	out.blocklyCode = in.getBlocklyCode();
	out.code = in.getCode();
	out.failCount = numCast(in.getFailCount());
	out.lastError = in.getLastError();
}

ndw::Player playerToThrift(Player const& player)
{
	ndw::Player outPlayer;
	outPlayer.id          = numCast(player.id);
	outPlayer.login       = player.login;
	outPlayer.password    = player.password;
	codeDataCppToThrift(player.fleetsCode, outPlayer.fleetsCode);
	codeDataCppToThrift(player.planetsCode, outPlayer.planetsCode);
	outPlayer.eventList.reserve(player.eventList.size());
	outPlayer.mainPlanet = coordToThrift(player.mainPlanet);
	for(Event const & ev: player.eventList)
		outPlayer.eventList.push_back(eventToThrift(ev));
	for(auto tutoNVP: player.tutoDisplayed)
		outPlayer.tutoDisplayed[tutoNVP.first] = numCast(tutoNVP.second);
	outPlayer.score = numCast(player.score);
	return outPlayer;
}


ndw::FleetReport fleetReportToThrift(Report<Fleet> const& fleetReport)
{
	ndw::FleetReport result;
	result.isDead = fleetReport.isDead;
	result.hasFight = fleetReport.hasFight;
	for(intptr_t id: fleetReport.enemySet)
		result.enemySet.insert(numeric_cast<int32_t>(id));
	result.fightInfo.before = fleetToThrift(fleetReport.fightInfo.before);
	result.fightInfo.after = fleetToThrift(fleetReport.fightInfo.after);
	return result;
}


ndw::PlanetReport planetReportToThrift(Report<Planet> const& planetReport)
{
	ndw::PlanetReport result;
	result.isDead = planetReport.isDead;
	result.hasFight = planetReport.hasFight;
	for(intptr_t id: planetReport.enemySet)
		result.enemySet.insert(numeric_cast<int32_t>(id));
	result.fightInfo.before = planetToThrift(planetReport.fightInfo.before);
	result.fightInfo.after = planetToThrift(planetReport.fightInfo.after);
	return result;
}


ndw::FightReport fightReportToThrift(FightReport const& report)
{
	ndw::FightReport result;
	result.fleetList.reserve(report.fleetList.size());
	for(Report<Fleet> const & fleetRep: report.fleetList)
		result.fleetList.push_back(fleetReportToThrift(fleetRep));
	result.hasPlanet = report.hasPlanet;
	if(result.hasPlanet != bool(report.planet))
		BOOST_THROW_EXCEPTION(std::logic_error("Unconsistent FightReport::hasPlanet value"));
	if(report.planet)
		result.__set_planet(planetReportToThrift(report.planet.get()));
	return result;
}

}

EngineServerHandler::EngineServerHandler()
{
	// Your initialization goes here
}

void EngineServerHandler::start()
{
	// Your implementation goes here
	//printf("start\n");
}

void EngineServerHandler::stop()
{
	// Your implementation goes here
	//printf("stop\n");
}

bool EngineServerHandler::addPlayer(const std::string& login, const std::string& password)
{
	LOG4CPLUS_TRACE(logger, "login: " << login);
	return engine_.addPlayer(login, password);
}


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


template<typename Range>
void sortOnType(Range& range, const ndw::Sort_Type::type sortType, const bool asc)
{
	typedef typename Range::value_type FleetOrPlanet;
	switch(sortType)
	{
	case ndw::Sort_Type::Name:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.name;});
		break;
	case ndw::Sort_Type::X:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.coord.X;});
		break;
	case ndw::Sort_Type::Y:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.coord.Y;});
		break;
	case ndw::Sort_Type::Z:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.coord.Z;});
		break;
	case ndw::Sort_Type::M:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.ressourceSet.tab[0];});
		break;
	case ndw::Sort_Type::C:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.ressourceSet.tab[1];});
		break;
	case ndw::Sort_Type::L:
		sortOnAttr(range, asc, [](FleetOrPlanet const & elt) {return elt.ressourceSet.tab[2];});
		break;
	default:
		BOOST_THROW_EXCEPTION(std::runtime_error("Unconsistent Sort_Type"));
	};
}


void EngineServerHandler::getPlayerFleets(
  ndw::FleetList& _return,
  const ndw::Player_ID pid,
  const int32_t beginIndexC,
  const int32_t endIndexC,
  const ndw::Sort_Type::type sortType,
  const bool asc)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid <<
	                " beginIndexC : " << beginIndexC <<
	                " endIndexC : " << endIndexC <<
	                " sortType : " << sortType <<
	                " asc : " << asc);
	size_t beginIndex = beginIndexC;
	size_t endIndex = endIndexC;
	auto fleetList = engine_.getPlayerFleets(pid);
	if(beginIndexC >= endIndexC || beginIndexC < 0)
		BOOST_THROW_EXCEPTION(std::runtime_error("Unconsistent index"));
	if(endIndex > fleetList.size())
		endIndex = fleetList.size();
	if(beginIndex > fleetList.size())
		beginIndex = fleetList.size();

	sortOnType(fleetList, sortType, asc);

	if(endIndex > fleetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("endIndex > fleetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(std::logic_error("beginIndex > endIndex"));

	_return.fleetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(fleetList.begin() + beginIndex,
	                                     fleetList.begin() + endIndex);
	std::set<Coord, CompCoord> fleetCoordSet;
	for(Fleet const & fleet: pageRange)
	{
		_return.fleetList.push_back(fleetToThrift(fleet));
		fleetCoordSet.insert(fleet.coord);
	}
	std::vector<Coord> fleetCoordVect(fleetCoordSet.begin(), fleetCoordSet.end());
	auto planetList = engine_.getPlanets(fleetCoordVect);
	for(Planet const & planet: planetList)
		_return.planetList.push_back(planetToThrift(planet));

	_return.fleetCount = numCast(fleetList.size());
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerPlanets(
  ndw::PlanetList& _return,
  const ndw::Player_ID pid,
  const int32_t beginIndexC,
  const int32_t endIndexC,
  const  ndw::Sort_Type::type sortType,
  const bool asc)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid <<
	                " beginIndexC : " << beginIndexC <<
	                " endIndexC : " << endIndexC <<
	                " sortType : " << sortType <<
	                " asc : " << asc);
	size_t beginIndex = beginIndexC;
	size_t endIndex = endIndexC;
	auto planetList = engine_.getPlayerPlanets(pid);
	if(beginIndexC >= endIndexC || beginIndexC < 0)
		BOOST_THROW_EXCEPTION(std::runtime_error("Unconsistent index"));
	if(endIndex > planetList.size())
		endIndex = planetList.size();
	if(beginIndex > planetList.size())
		beginIndex = planetList.size();

	sortOnType(planetList, sortType, asc);

	if(endIndex > planetList.size())
		BOOST_THROW_EXCEPTION(std::logic_error("endIndex > planetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(std::logic_error("beginIndex > endIndex"));

	_return.planetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(planetList.begin() + beginIndex,
	                                     planetList.begin() + endIndex);
	for(Planet const & planet: pageRange)
		_return.planetList.push_back(planetToThrift(planet));
	_return.planetCount = numCast(planetList.size());
	LOG4CPLUS_TRACE(logger, "enter");
}

void EngineServerHandler::setPlayerFleetCode(const ndw::Player_ID pid, const std::string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	engine_.setPlayerFleetCode(pid, code);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::setPlayerPlanetCode(const ndw::Player_ID pid, const std::string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	engine_.setPlayerPlanetCode(pid, code);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::setPlayerFleetBlocklyCode(const ndw::Player_ID pid, const std::string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	engine_.setPlayerFleetBlocklyCode(pid, code);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::setPlayerPlanetBlocklyCode(const ndw::Player_ID pid, const std::string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	engine_.setPlayerPlanetBlocklyCode(pid, code);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getPlayerFleetCode(ndw::CodeData& _return, const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	codeDataCppToThrift(engine_.getPlayerFleetCode(pid), _return);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getPlayerPlanetCode(ndw::CodeData& _return, const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	codeDataCppToThrift(engine_.getPlayerPlanetCode(pid), _return);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getPlayers(std::vector<ndw::Player>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	std::vector<Player> players = engine_.getPlayers();
	_return.reserve(players.size());
	transform(players, back_inserter(_return), playerToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getPlayer(ndw::Player& _return, const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	_return = playerToThrift(engine_.getPlayer(pid));
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getPlanet(std::vector<ndw::Planet>& _return, const ndw::Coord& coord)
{
	LOG4CPLUS_TRACE(logger, "coord : " << coord);
	optional<Planet> planet =
	  engine_.getPlanet(Coord(coord.X, coord.Y, coord.Z));
	if(planet.is_initialized())
		_return.push_back(planetToThrift(*planet));
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::getFleet(ndw::Fleet& _return, const ndw::Fleet_ID fid)
{
	LOG4CPLUS_TRACE(logger, "fid : " << fid);
	_return = fleetToThrift(engine_.getFleet(fid));
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::logPlayer(ndw::OptionalPlayer& _return, const std::string& login, const std::string& password)
{
	LOG4CPLUS_TRACE(logger, "login : " << login << " password : " << password);
	optional<Player> optPlayer = engine_.getPlayer(login, password);
	if(optPlayer)
		_return.__set_player(playerToThrift(optPlayer.get()));
	LOG4CPLUS_TRACE(logger, "exit");
}

void EngineServerHandler::incrementTutoDisplayed(ndw::Player_ID pid, std::string const& tutoName)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " tutoName : " << tutoName);
	engine_.incrementTutoDisplayed(pid, tutoName);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFightReport(ndw::FightReport& _return, const int32_t id)
{
	LOG4CPLUS_TRACE(logger, "id : " << id);
	FightReport fr = engine_.getFightReport(id);
	_return = fightReportToThrift(fr);
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

bool EngineServerHandler::eraseAccount(const int32_t pid, const std::string& password)
{
	LOG4CPLUS_TRACE(logger, "password : " << password);
	Player player = engine_.getPlayer(pid);
	if(player.password == password)
	{
		engine_.eraseAccount(pid);
		LOG4CPLUS_TRACE(logger, "true");
		return true;
	}
	else
	{
		LOG4CPLUS_TRACE(logger, "false");
		return false;
	}
}


void EngineServerHandler::getBuildingsInfo(std::vector<ndw::Building>& _return)
{
	_return.reserve(Building::Count);
	int32_t index = 0;
	for(Building const & b: Building::List)
	{
		ndw::Building newBu;
		newBu.index = index++;
		newBu.price = ressourceToThrift(b.price);
		newBu.coef = b.coef;
		_return.push_back(newBu);
	}
}

void EngineServerHandler::getCannonsInfo(std::vector<ndw::Cannon>& _return)
{
	_return.reserve(Cannon::Count);
	int32_t index = 0;
	for(Cannon const & c: Cannon::List)
	{
		ndw::Cannon newCa;
		newCa.index = index++;
		newCa.price = ressourceToThrift(c.price);
		newCa.life = numCast(c.life);
		newCa.power = numCast(c.power);
		_return.push_back(newCa);
	}
}

void EngineServerHandler::getShipsInfo(std::vector<ndw::Ship>& _return)
{
	_return.reserve(Ship::Count);
	int32_t index = 0;
	for(Ship const & s: Ship::List)
	{
		ndw::Ship newSh;
		newSh.index = index++;
		newSh.price = ressourceToThrift(s.price);
		newSh.life = numCast(s.life);
		newSh.power = numCast(s.power);
		_return.push_back(newSh);
	}
}

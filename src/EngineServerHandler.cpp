#include "stdafx.h"
#include "EngineServerHandler.h"

#include <algorithm>
#include <boost/format.hpp>

using namespace boost;
using namespace std;

using namespace log4cplus;
static Logger logger = Logger::getInstance(
                         LOG4CPLUS_TEXT("EngineServerHandler"));


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

ostream& operator << (ostream& os, ndw::Coord const& coord)
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
	for(size_t value : ress.tab)
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
	result.origin = coordToThrift(fleet.origin);
	result.name = fleet.name;

	result.shipList.reserve(fleet.shipList.size());
	for(size_t value : fleet.shipList)
		result.shipList.push_back(numCast(value));

	result.ressourceSet = ressourceToThrift(fleet.ressourceSet);

	result.taskQueue.reserve(fleet.taskQueue.size());
	for(FleetTask const & task : fleet.taskQueue)
		result.taskQueue.push_back(fleetTaskToThrift(task));

	return result;
}


ndw::Planet planetToThrift(Planet const& planet)
{
	ndw::Planet res;

	res.name = planet.name;
	res.coord = coordToThrift(planet.coord);
	res.playerId = numCast(planet.playerId);
	res.buildingList.reserve(planet.buildingList.size());
	for(size_t value : planet.buildingList)
		res.buildingList.push_back(numCast(value));
	res.taskQueue.reserve(planet.taskQueue.size());
	transform(planet.taskQueue,
	          back_inserter(res.taskQueue),
	          planetTaskToThrift);
	res.ressourceSet = ressourceToThrift(planet.ressourceSet);
	res.cannonTab.reserve(planet.cannonTab.size());
	for(size_t value : planet.cannonTab)
		res.cannonTab.push_back(numCast(value));

	return res;
}


void codeDataCppToThrift(CodeData const& in, ndw::CodeData& out)
{
	out.blocklyCode = in.blocklyCode;
	out.code = in.code;
	out.lastError = in.lastError;
}


ndw::Player playerToThrift(Player const& player)
{
	ndw::Player outPlayer;
	outPlayer.id          = numCast(player.id);
	outPlayer.login       = player.login;
	outPlayer.password    = player.password;
	outPlayer.mainPlanet = coordToThrift(player.mainPlanet);
	outPlayer.score = numCast(player.score);
	outPlayer.allianceID = numCast(player.allianceID);
	outPlayer.allianceName = player.allianceName;
	return outPlayer;
}


ndw::FleetReport fleetReportToThrift(Report<Fleet> const& fleetReport)
{
	ndw::FleetReport result;
	result.isDead = fleetReport.isDead;
	result.hasFight = fleetReport.hasFight;
	result.experience = fleetReport.experience;
	for(intptr_t id : fleetReport.enemySet)
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
	result.experience = planetReport.experience;
	for(intptr_t id : planetReport.enemySet)
		result.enemySet.insert(numeric_cast<int32_t>(id));
	result.fightInfo.before = planetToThrift(planetReport.fightInfo.before);
	result.fightInfo.after = planetToThrift(planetReport.fightInfo.after);
	return result;
}


ndw::FightReport fightReportToThrift(FightReport const& report)
{
	ndw::FightReport result;
	result.fleetList.reserve(report.fleetList.size());
	for(Report<Fleet> const & fleetRep : report.fleetList)
		result.fleetList.push_back(fleetReportToThrift(fleetRep));
	result.hasPlanet = report.hasPlanet;
	if(result.hasPlanet != bool(report.planet))
		BOOST_THROW_EXCEPTION(
		  logic_error("Unconsistent FightReport::hasPlanet value"));
	if(report.planet)
		result.__set_planet(planetReportToThrift(report.planet.get()));
	return result;
}

} //Namespace anonyme


EngineServerHandler::EngineServerHandler()
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
		BOOST_THROW_EXCEPTION(Engine::InvalidData("login"));
	if(password.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(Engine::InvalidData("password"));

	std::vector<std::string> codes;
	getNewPlayerCode(codes);
	Player::ID const pid = database_.addPlayer(login, password, codes);
	if(pid != Player::NoId)
	{
		Coord coord = engine_.addPlayer(pid);
		database_.setPlayerMainPlanet(pid, coord);
		engine_.reloadPlayer(pid);
		return true;
	}
	else
		return false;
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
		BOOST_THROW_EXCEPTION(runtime_error("Unconsistent index"));
	if(endIndex > fleetList.size())
		endIndex = fleetList.size();
	if(beginIndex > fleetList.size())
		beginIndex = fleetList.size();

	sortOnType(fleetList, sortType, asc);

	if(endIndex > fleetList.size())
		BOOST_THROW_EXCEPTION(logic_error("endIndex > fleetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(logic_error("beginIndex > endIndex"));

	_return.fleetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(fleetList.begin() + beginIndex,
	                                     fleetList.begin() + endIndex);
	set<Coord, CompCoord> fleetCoordSet;
	for(Fleet const & fleet : pageRange)
	{
		_return.fleetList.push_back(fleetToThrift(fleet));
		fleetCoordSet.insert(fleet.coord);
	}
	vector<Coord> fleetCoordVect(fleetCoordSet.begin(), fleetCoordSet.end());
	auto planetList = engine_.getPlanets(fleetCoordVect);
	for(Planet const & planet : planetList)
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
		BOOST_THROW_EXCEPTION(runtime_error("Unconsistent index"));
	if(endIndex > planetList.size())
		endIndex = planetList.size();
	if(beginIndex > planetList.size())
		beginIndex = planetList.size();

	sortOnType(planetList, sortType, asc);

	if(endIndex > planetList.size())
		BOOST_THROW_EXCEPTION(logic_error("endIndex > planetList.size()"));
	if(beginIndex > endIndex)
		BOOST_THROW_EXCEPTION(logic_error("beginIndex > endIndex"));

	_return.planetList.reserve(endIndex - beginIndex);
	auto pageRange = make_iterator_range(planetList.begin() + beginIndex,
	                                     planetList.begin() + endIndex);
	for(Planet const & planet : pageRange)
		_return.planetList.push_back(planetToThrift(planet));
	_return.planetCount = numCast(planetList.size());
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerFleetCode(const ndw::Player_ID pid,
    const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(Engine::InvalidData("Code size too long"));
	database_.addScript(pid, CodeData::Fleet, code);
	engine_.reloadPlayer(pid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerPlanetCode(const ndw::Player_ID pid,
    const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(Engine::InvalidData("Code size too long"));
	database_.addScript(pid, CodeData::Planet, code);
	engine_.reloadPlayer(pid);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerFleetBlocklyCode(const ndw::Player_ID pid,
    const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > Player::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(Engine::InvalidData("Code size too long"));
	database_.addBlocklyCode(pid, CodeData::Fleet, code);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::setPlayerPlanetBlocklyCode(const ndw::Player_ID pid,
    const string& code)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " code : " << code);
	if(code.size() > Player::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(Engine::InvalidData("Code size too long"));
	database_.addBlocklyCode(pid, CodeData::Planet, code);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerFleetCode(ndw::CodeData& ret,
    const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	codeDataCppToThrift(database_.getPlayerCode(pid, CodeData::Fleet), ret);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getPlayerPlanetCode(ndw::CodeData& ret,
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
	//TODO : Séparer en deux requetes differentes
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	outPlayer = playerToThrift(database_.getPlayer(pid));
	CodeData const fleetCode = database_.getPlayerCode(pid, CodeData::Fleet);
	codeDataCppToThrift(fleetCode, outPlayer.fleetsCode);
	CodeData const planetCode = database_.getPlayerCode(pid, CodeData::Planet);
	codeDataCppToThrift(planetCode, outPlayer.planetsCode);
	map<string, size_t> const levelMap = database_.getTutoDisplayed(pid);
	for(auto tutoNVP : levelMap)
		outPlayer.tutoDisplayed[tutoNVP.first] = numCast(tutoNVP.second);
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
	if(planet.is_initialized())
	{
		_return.push_back(planetToThrift(*planet));
		vector<Event> events =
		  database_.getPlanetEvents(planet->playerId, coord);
		_return.front().eventList.reserve(events.size());
		for(Event const & ev : events)
			_return.front().eventList.push_back(eventToThrift(ev));
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFleet(ndw::Fleet& _return,
                                   const ndw::Fleet_ID fid)
{
	LOG4CPLUS_TRACE(logger, "fid : " << fid);
	_return = fleetToThrift(engine_.getFleet(fid));

	vector<Event> events = database_.getFleetEvents(_return.playerId, fid);
	_return.eventList.reserve(events.size());
	for(Event const & ev : events)
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
			outPlayer.tutoDisplayed[tutoNVP.first] = numCast(tutoNVP.second);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::incrementTutoDisplayed(ndw::Player_ID pid,
    string const& tutoName)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid << " tutoName : " << tutoName);
	database_.incrementTutoDisplayed(pid, tutoName);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getFightReport(ndw::FightReport& _return,
    const int32_t id)
{
	LOG4CPLUS_TRACE(logger, "id : " << id);
	FightReport fr = database_.getFightReport(id);
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


bool EngineServerHandler::eraseAccount(const ndw::Player_ID pid,
                                       const string& password)
{
	LOG4CPLUS_TRACE(logger, "password : " << password);
	Player player = database_.getPlayer(pid);
	if(player.password == password)
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
  vector<ndw::Event>& _return, const ndw::Player_ID pid)
{
	LOG4CPLUS_TRACE(logger, "pid : " << pid);
	vector<Event> events = database_.getPlayerEvents(pid);
	_return.reserve(events.size());
	transform(events, back_inserter(_return), eventToThrift);
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getBuildingsInfo(vector<ndw::Building>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	_return.reserve(Building::Count);
	int32_t index = 0;
	for(Building const & b : Building::List)
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
	for(Cannon const & c : Cannon::List)
	{
		ndw::Cannon newCa;
		newCa.index = index++;
		newCa.price = ressourceToThrift(c.price);
		newCa.life = numCast(c.life);
		newCa.power = numCast(c.power);
		_return.push_back(newCa);
	}
	LOG4CPLUS_TRACE(logger, "exit");
}


void EngineServerHandler::getShipsInfo(vector<ndw::Ship>& _return)
{
	LOG4CPLUS_TRACE(logger, "enter");
	_return.reserve(Ship::Count);
	int32_t index = 0;
	for(Ship const & s : Ship::List)
	{
		ndw::Ship newSh;
		newSh.index = index++;
		newSh.price = ressourceToThrift(s.price);
		newSh.life = numCast(s.life);
		newSh.power = numCast(s.power);
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
	for(Message const & message : messages)
	{
		ndw::Message newMess;
		newMess.id = message.id;
		newMess.sender = message.sender;
		newMess.recipient = message.recipient;
		newMess.time = numCast(message.time);
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
	database_.eraseMesage(numCast(mid));
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
	database_.updateAlliance(
	  Alliance(al.id, al.masterID, al.name, al.description, al.masterLogin));
}


void EngineServerHandler::transfertAlliance(
  const ndw::Alliance_ID aid,
  const ndw::Player_ID pid)
{
	database_.transfertAlliance(aid, pid);
}



void EngineServerHandler::eraseAlliance(const ndw::Alliance_ID aid)
{
	database_.eraseAlliance(aid);
}


void EngineServerHandler::joinAlliance(const ndw::Player_ID pid,
                                       const ndw::Alliance_ID aid)
{
	database_.joinAlliance(pid, aid);
}


void EngineServerHandler::quitAlliance(const ndw::Player_ID pid)
{
	database_.quitAlliance(pid);
}


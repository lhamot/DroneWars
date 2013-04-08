#include "stdafx.h"
#include "DataBase.h"

#include <iostream>
#include <sstream>
#include <iterator>

#pragma warning(push)
#pragma warning(disable: 4310)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(pop)
#include <boost/format.hpp>

#pragma warning(push)
#pragma warning(disable: 4512 4244)
#include <Poco/Data/Session.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/BLOB.h>
#include <Poco/Tuple.h>
#include <Poco/Nullable.h>
#pragma warning(pop)

#include "NameGen.h"


using namespace Poco::Data;
using namespace Poco;
using namespace boost;
using namespace std;


class Transaction : private boost::noncopyable
{
	Session& session_;
	bool commited_;

public:
	Transaction(Session& sess):
		session_(sess),
		commited_(false)
	{
		session_.begin();
	}

	void commit()
	{
		session_.commit();
		commited_ = true;
	}

	~Transaction()
	{
		if(commited_ == false)
			session_.rollback();
	}
};


#define DB_CATCH \
	catch(Poco::Data::DataException const& ex)                                \
	{                                                                         \
		LOG4CPLUS_ERROR(                                                      \
		    log4cplus::Logger::getInstance("DataBase"), ex.displayText());    \
		BOOST_THROW_EXCEPTION(DataBase::Exception(ex.displayText()));         \
	}

DataBase::DataBase()
{
	try
	{
		Poco::Data::MySQL::Connector::registerConnector();
		session_.reset(new Session("MySQL",
		                           "host=localhost;"
		                           "port=3306;"
		                           "db=dronewars;"
		                           "user=Blaspheme;"
		                           "password=pdcx3wady6nsMfUm"));

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Options ("
		            "  name VARCHAR(30) NOT NULL,"
		            "  value VARCHAR(30) NOT NULL"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Player ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  login VARCHAR(30) unique NOT NULL,"
		            "  password VARCHAR(30) NOT NULL,"
		            "  score INTEGER NOT NULL,"
		            "  planetCoordX INTEGER NOT NULL,"
		            "  planetCoordY INTEGER NOT NULL,"
		            "  planetCoordZ INTEGER NOT NULL"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Event ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  time INTEGER NOT NULL,"
		            "  type INTEGER NOT NULL,"
		            "  comment VARCHAR(500) NOT NULL,"
		            "  value INTEGER NOT NULL,"
		            "  viewed INTEGER NOT NULL,"
		            "  playerID INTEGER NOT NULL,"
		            "  fleetID INTEGER NOT NULL,"
		            "  planetCoordX INTEGER NOT NULL,"
		            "  planetCoordY INTEGER NOT NULL,"
		            "  planetCoordZ INTEGER NOT NULL,"
		            "  INDEX (playerID),"
		            "  INDEX (fleetID),"
		            "  INDEX Coord (planetCoordX, planetCoordY, planetCoordZ)"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Script ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  playerID INTEGER NOT NULL,"
		            "  time INTEGER NOT NULL,"
		            "  target INTEGER NOT NULL,"
		            "  code TEXT NOT NULL,"
		            "  lastError TEXT"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "BlocklyCode ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  playerID INTEGER NOT NULL,"
		            "  time INTEGER NOT NULL,"
		            "  target INTEGER NOT NULL,"
		            "  code TEXT NOT NULL"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "FightReport ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  time INTEGER NOT NULL,"
		            "  data BLOB NOT NULL"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "TutoDisplayed ("
		            "  playerID INTEGER,"
		            "  tag varchar(30) NOT NULL,"
		            "  level INTEGER NOT NULL,"
		            "  FOREIGN KEY (playerID) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  INDEX (playerID), "
		            "  INDEX PlayerTag (playerID, tag)"
		            ")", now;
	}
	DB_CATCH
}

DataBase::~DataBase()
{
}

Player::ID DataBase::addPlayer(std::string const& login,
                               std::string const& password)
try
{
	Transaction trans(*session_);
	std::cout << login << " " << password << std::endl;
	(*session_) <<
	            "INSERT IGNORE INTO Player (login, password) VALUES(?, ?)",
	            use(login), use(password), now;
	size_t rowCount;
	(*session_) << "SELECT ROW_COUNT() ", into(rowCount), now;
	if(rowCount == 0)
		return Player::NoId;
	else
	{
		size_t id = 0;
		(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
		//Ajout du niveau de tutos
		(*session_) <<
		            "INSERT INTO TutoDisplayed "
		            "(playerID, tag, level) "
		            "VALUES(?, ?, ?)",
		            use(id), use(std::string(CoddingLevelTag)), use(0),
		            now;
		trans.commit();
		return id;
	}
}
DB_CATCH


void DataBase::setPlayerMainPlanet(Player::ID pid, Coord mainPlanet)
try
{
	(*session_) <<
	            "UPDATE Player "
	            "SET planetCoordX = ?, planetCoordY = ?, planetCoordZ = ? "
	            "WHERE id = ?",
	            use(mainPlanet.X),
	            use(mainPlanet.Y),
	            use(mainPlanet.Z),
	            use(pid),
	            now;
}
DB_CATCH


typedef Tuple < size_t, std::string, std::string, uint64_t,
        Coord::Value, Coord::Value, Coord::Value > PlayerTmp;
Player playerFromTuple(PlayerTmp const& playerTup)
{
	Player player(playerTup.get<0>(), playerTup.get<1>(), playerTup.get<2>());
	player.score = playerTup.get<3>();
	player.mainPlanet.X = playerTup.get<4>();
	player.mainPlanet.Y = playerTup.get<5>();
	player.mainPlanet.Z = playerTup.get<6>();
	return player;
}


boost::optional<Player> DataBase::getPlayer(
  std::string const& login,
  std::string const& password) const
try
{
	std::vector<PlayerTmp> playerList;
	(*session_) <<
	            "SELECT * FROM Player WHERE login = ? AND password = ?",
	            into(playerList), use(login), use(password), now;
	if(playerList.size() != 1)
		return boost::optional<Player>();
	else
		return playerFromTuple(playerList.front());
}
DB_CATCH


std::vector<Player> DataBase::getPlayers() const
try
{
	std::vector<PlayerTmp> playerList;
	(*session_) <<
	            "SELECT * FROM Player",
	            into(playerList), now;
	std::vector<Player> outPlayerList;
	outPlayerList.reserve(playerList.size());
	boost::transform(
	  playerList, back_inserter(outPlayerList), playerFromTuple);
	return outPlayerList;
}
DB_CATCH


Player DataBase::getPlayer(Player::ID id) const
try
{
	std::vector<PlayerTmp> playerList;
	(*session_) << "SELECT * FROM Player WHERE id = ?",
	            into(playerList), use(id), now;
	if(playerList.size() != 1)
		BOOST_THROW_EXCEPTION(
		  std::invalid_argument("Player id not found in base"));
	else
		return playerFromTuple(playerList.front());
}
DB_CATCH


void DataBase::addEvents(std::vector<Event> const& events)
try
{
	if(events.empty())
		return;
	typedef Poco::Tuple < time_t, int, std::string const&, intptr_t, int,
	        Player::ID, Fleet::ID, Coord::Value, Coord::Value,
	        Coord::Value > DBEvent;
	std::vector<DBEvent> dbEvents;
	dbEvents.reserve(events.size());
	for(Event const & event: events)
		dbEvents.push_back(
		  DBEvent(event.time, event.type, event.comment, event.value,
		          event.viewed, event.playerID, event.fleetID,
		          event.planetCoord.X, event.planetCoord.Y,
		          event.planetCoord.Z));

	Transaction trans(*session_);
	(*session_) <<
	            "INSERT INTO Event "
	            "(time, type, comment, value, viewed, playerID, "
	            "  fleetID, planetCoordX, planetCoordY, planetCoordZ) "
	            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", use(dbEvents), now;
	trans.commit();
}
DB_CATCH


void DataBase::removeOldEvents()
try
{
	Transaction trans(*session_);

	// Planete
	// Important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use((int)Event::PlanetWin),
	            use((int)Event::PlanetColonized),
	            use((int)Event::Upgraded),
	            now;
	// Pas important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (60 * 10)),
	            use((int)Event::CannonMade),
	            use((int)Event::ShipMade),
	            use((int)Event::FleetDrop),
	            now;
	//Flotte
	// Important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use((int)Event::FleetWin),
	            use((int)Event::FleetDraw),
	            use((int)Event::PlanetColonized),
	            now;
	// Pas important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (60 * 10)),
	            use((int)Event::FleetsGather),
	            use((int)Event::PlanetHarvested),
	            use((int)Event::FleetDrop),
	            now;
	//Joueur
	// Important
	(*session_) <<
	            "DELETE FROM Event "
	            "WHERE time < ? AND type IN (?, ?, ?, ?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use((int)Event::FleetCodeError),
	            use((int)Event::FleetCodeExecError),
	            use((int)Event::PlanetCodeError),
	            use((int)Event::PlanetCodeExecError),
	            use((int)Event::FleetLose),
	            use((int)Event::PlanetLose),
	            now;

	//Rappor de combat
	(*session_) <<
	            "DELETE FROM FightReport WHERE time < ?",
	            use(time(0) - (3600 * 24)),
	            now;

	trans.commit();
}
DB_CATCH


typedef Poco::Tuple < Event::ID, time_t, size_t, std::string, intptr_t,
        int, Player::ID, Fleet::ID, Coord::Value, Coord::Value, Coord::Value >
        DBEvent;


Event toEvent(DBEvent const& ev)
{
	Event res;
	res.id = ev.get<0>();
	res.time = ev.get<1>();
	res.type = Event::Type(ev.get<2>());
	res.comment = ev.get<3>();
	res.value = ev.get<4>();
	res.viewed = ev.get<5>() != 0;
	res.playerID = ev.get<6>();
	res.fleetID = ev.get<7>();
	res.planetCoord = Coord(ev.get<8>(), ev.get<9>(), ev.get<10>());
	return res;
}


std::vector<Event> DataBase::getPlayerEvents(Player::ID pid) const
try
{
	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE playerID = ? "
	            " AND type IN (?, ?, ?, ?, ?, ?) "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents),
	            use(pid),
	            use((int)Event::FleetCodeError),
	            use((int)Event::FleetCodeExecError),
	            use((int)Event::PlanetCodeError),
	            use((int)Event::PlanetCodeExecError),
	            use((int)Event::FleetLose),
	            use((int)Event::PlanetLose),
	            now;

	std::vector<Event> out;
	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}
DB_CATCH


std::vector<Event> DataBase::getPlanetEvents(Player::ID pid,
    Coord pcoord) const
try
{
	std::vector<Event> out;

	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE"
	            "  playerID = ? AND "
	            "  planetCoordX=? AND planetCoordY=? AND planetCoordZ=? "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents),
	            use(pid),
	            use(pcoord.X), use(pcoord.Y), use(pcoord.Z),
	            now;

	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}
DB_CATCH


std::vector<Event> DataBase::getFleetEvents(
  Player::ID pid, Fleet::ID fid) const
try
{
	std::vector<Event> out;
	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE playerID = ? AND fleetID = ? "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents), use(pid), use(fid),
	            now;

	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}
DB_CATCH


void  DataBase::resetPlanetEvents(Coord pcoord)
try
{
	(*session_) <<
	            "DELETE FROM Event "
	            "WHERE"
	            "  planetCoordX=? AND planetCoordY=? AND planetCoordZ=?",
	            use(pcoord.X), use(pcoord.Y), use(pcoord.Z), now;
}
DB_CATCH


size_t DataBase::addFightReport(FightReport const& report)
try
{
	using namespace boost::archive;
	Transaction trans(*session_);
	std::string data;
	stringstream ss(ios::binary | ios::in | ios::out);
	boost::archive::text_oarchive oa(ss);
	oa& report;
	data = ss.str();
	(*session_) << "INSERT INTO FightReport (time, data) VALUES(?, ?)",
	            use(time(0)), use(data), now;
	size_t id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	trans.commit();
	return id;
}
DB_CATCH


void DataBase::addFightReports(std::vector<FightReport> const& reports)
try
{
	using namespace boost::archive;
	Transaction trans(*session_);
	std::string data;
	time_t now = 0;
	Statement stmt =
	  ((*session_) << "INSERT INTO FightReport (time, data) VALUES(?, ?)",
	   use(now), use(data));

	for(FightReport const & report: reports)
	{
		stringstream ss(ios::binary | ios::in | ios::out);
		boost::archive::text_oarchive oa(ss);
		oa& report;
		data = ss.str();
		now = time(0);
		stmt.execute();
	}
	trans.commit();
}
DB_CATCH


FightReport DataBase::getFightReport(size_t reportID)
try
{
	FightReport report;
	//std::string data;
	Poco::Data::BLOB data;
	(*session_) << "SELECT data FROM FightReport WHERE id = ?",
	            use(reportID), into(data), now;

	using namespace boost::archive;
	stringstream ss(ios::binary | ios::in | ios::out);
	ss.rdbuf()->sputn(data.rawContent(), data.size());
	boost::archive::text_iarchive ia(ss);
	ia& report;
	return report;
}
DB_CATCH


size_t DataBase::addScript(Player::ID pid,
                           CodeData::Target target,
                           std::string const& code)
try
{
	Transaction trans(*session_);
	(*session_) <<
	            "INSERT INTO Script "
	            "(playerID, time, target, code) "
	            "VALUES(?, ?, ?, ?)",
	            use(pid),
	            use(time(0)),
	            use((int)target),
	            use(code),
	            now;
	size_t id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	trans.commit();
	return id;
}
DB_CATCH


size_t DataBase::addBlocklyCode(Player::ID pid,
                                CodeData::Target target,
                                std::string const& code)
try
{
	Transaction trans(*session_);
	(*session_) <<
	            "INSERT INTO BlocklyCode "
	            "(playerID, time, target, code) "
	            "VALUES(?, ?, ?, ?)",
	            use(pid),
	            use(time(0)),
	            use((int)target),
	            use(code),
	            now;
	size_t id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	trans.commit();
	return id;
}
DB_CATCH


namespace Poco
{
namespace Data
{

template <>
class TypeHandler<typename DataBase::CodeError>
{
public:
	static void bind(std::size_t pos,
	                 const DataBase::CodeError& obj,
	                 AbstractBinder* pBinder)
	{
		poco_assert_dbg(pBinder != 0);
		TypeHandler<std::string>::bind(pos++, obj.message, pBinder);
		TypeHandler<size_t>::bind(pos++, obj.codeDataId, pBinder);
	}

	static std::size_t size()
	{
		return 2;
	}

	static void prepare(std::size_t pos,
	                    const DataBase::CodeError& obj,
	                    AbstractPreparation* pPrepare)
	{
		poco_assert_dbg(pBinder != 0);
		TypeHandler<std::string>::prepare(pos++, obj.message, pPrepare);
		TypeHandler<size_t>::prepare(pos++, obj.codeDataId, pPrepare);
	}

	static void extract(std::size_t pos,
	                    DataBase::CodeError& obj,
	                    const DataBase::CodeError& defVal,
	                    AbstractExtractor* pExt)
	{
		poco_assert_dbg(pExt != 0);
		std::string message;
		size_t codeID = 0;
		TypeHandler<std::string>::extract(pos++, message, defVal.message, pExt);
		TypeHandler<size_t>::extract(pos++, codeID, defVal.codeDataId, pExt);
		obj.message = message;
		obj.codeDataId = codeID;
	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

}
} // namespace Poco::Data

void DataBase::addCodeErrors(std::vector<CodeError> const& errors)
try
{
	if(errors.empty())
		return;
	Transaction trans(*session_);
	(*session_) <<
	            "UPDATE Script SET lastError = ? WHERE id = ? ",
	            use(errors),
	            now;
	trans.commit();
}
DB_CATCH


CodeData DataBase::getPlayerCode(Player::ID pid, CodeData::Target target) const
try
{
	typedef Poco::Tuple < size_t, Player::ID, time_t, size_t,
	        BLOB, BLOB > ScriptTuple;
	ScriptTuple scrData;
	(*session_) <<
	            "SELECT * FROM Script "
	            "WHERE playerID = ? AND target = ? "
	            "ORDER BY id DESC "
	            "LIMIT 1 ",
	            use(pid), use((int)target), into(scrData), now;

	typedef Poco::Tuple<size_t, Player::ID, time_t, size_t, BLOB> BlocklyTuple;
	BlocklyTuple bloData;
	(*session_) <<
	            "SELECT * FROM BlocklyCode "
	            "WHERE playerID = ? AND target = ? "
	            "ORDER BY id DESC "
	            "LIMIT 1 ",
	            use(pid), use((int)target), into(bloData), now;

	CodeData res;
	res.id = scrData.get<0>();
	res.playerId = pid;
	res.target = target;
	res.code.assign(scrData.get<4>().begin(), scrData.get<4>().end());
	res.blocklyCode.assign(bloData.get<4>().begin(), bloData.get<4>().end());
	res.lastError.assign(scrData.get<5>().begin(), scrData.get<5>().end());
	return res;
}
DB_CATCH


void DataBase::eraseAccount(Player::ID pid)
try
{
	Transaction trans(*session_);
	std::string const login = nameGen();
	std::string const password = "gfd8fg451g51df8hgdf";

	(*session_) <<
	            "UPDATE Player "
	            "SET login = ?, password = ? "
	            "WHERE id = ? ",
	            use(login), use(password), use(pid),
	            now;
	addScript(pid, CodeData::Planet, "");
	addScript(pid, CodeData::Fleet, "");
	trans.commit();
}
DB_CATCH


void DataBase::incrementTutoDisplayed(std::vector<Player::ID> const& pids,
                                      std::string const& tutoName)
try
{
	std::stringstream ss;
	ss << "UPDATE TutoDisplayed "
	   "SET level = level + 1 "
	   "WHERE playerID IN (";
	for(Player::ID pid: pids)
		ss << pid << ",";
	ss << "0) AND tag = ? ";
	(*session_) << ss.str(), use(tutoName), now;
}
DB_CATCH


void DataBase::incrementTutoDisplayed(Player::ID pid,
                                      std::string const& tutoName)
try
{
	Transaction trans(*session_);
	(*session_) <<
	            "INSERT IGNORE INTO TutoDisplayed "
	            "(playerID, tag, level) "
	            "VALUES(?, ?, ?)", use(pid), use(tutoName), use(0), now;
	(*session_) << "UPDATE TutoDisplayed "
	            "SET level = level + 1 "
	            "WHERE playerID = ? AND tag = ? ",
	            use(pid), use(tutoName), now;
	trans.commit();
}
DB_CATCH


DataBase::PlayerTutoMap DataBase::getTutoDisplayed(Player::ID pid) const
try
{
	PlayerTutoMap result;
	typedef Poco::Tuple<std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) <<
	            "SELECT tag, level FROM TutoDisplayed "
	            "WHERE playerID = ? ",
	            use(pid), into(tutos), now;
	for(TutoTuple const & tuto: tutos)
		result.insert(make_pair(tuto.get<0>(), tuto.get<1>()));
	return result;
}
DB_CATCH


std::map<Player::ID, DataBase::PlayerTutoMap>
DataBase::getAllTutoDisplayed() const
try
{
	std::map<Player::ID, PlayerTutoMap> result;
	typedef Poco::Tuple<Player::ID, std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) << "SELECT * FROM TutoDisplayed", into(tutos), now;
	for(TutoTuple const & tuto: tutos)
		result[tuto.get<0>()].insert(make_pair(tuto.get<1>(), tuto.get<2>()));
	return result;
}
DB_CATCH


void DataBase::updateScore(std::map<Player::ID, uint64_t> const& scoreMap)
try
{
	Transaction trans(*session_);
	typedef Poco::Tuple<uint64_t, Player::ID> ScoreTuple;
	std::vector<ScoreTuple> scoreVect;
	scoreVect.reserve(scoreMap.size());
	for(auto nvp: scoreMap)
		scoreVect.push_back(ScoreTuple(nvp.second, nvp.first));
	(*session_) << "UPDATE Player SET score = ? WHERE id = ?",
	            use(scoreVect), now;
	trans.commit();
}
DB_CATCH

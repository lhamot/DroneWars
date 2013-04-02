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
		            "  password VARCHAR(30) NOT NULL"
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
		            "CodeData ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  playerID INTEGER NOT NULL,"
		            "  target INTEGER NOT NULL,"
		            "  time INTEGER NOT NULL,"
		            "  blocklyCode TEXT NOT NULL,"
		            "  code TEXT NOT NULL,"
		            "  failCount INTEGER NOT NULL,"
		            "  lastError TEXT NOT NULL"
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "FightReport ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  time INTEGER NOT NULL,"
		            "  data BLOB NOT NULL"
		            ")", now;
	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cerr << typeid(ex).name() << " " << ex.displayText() << std::endl;
		throw;
	}
}

DataBase::~DataBase()
{
}

Player DataBase::addPlayer(std::string const& login,
                           std::string const& password
                          )
{
	Player player;
	try
	{
		Transaction trans(*session_);
		std::cout << login << " " << password << std::endl;
		(*session_) << "INSERT INTO Player (login, password) VALUES(?, ?)",
		            use(login), use(password), now;
		size_t id = 0;
		(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
		trans.commit();
		player.id = id;
		player.login = login;
		player.password = password;
	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cerr << ex.displayText() << std::endl;
		throw;
	}
	return player;
}

boost::optional<Player> DataBase::getPlayer(std::string const& login,
    std::string const& password
                                           ) const
{
	Player player;
	typedef Tuple<size_t, std::string, std::string> PlayerTmp;
	std::vector<PlayerTmp> playerList;
	(*session_) <<
	            "SELECT * FROM Player WHERE login = ? AND password = ?",
	            into(playerList), use(login), use(password), now;
	if(playerList.size() != 1)
		return boost::optional<Player>();
	player.id = playerList.front().get<0>();
	player.login = playerList.front().get<1>();
	player.password = playerList.front().get<2>();
	return player;
}

Player DataBase::getPlayer(Player::ID id) const
{
	Player player;
	typedef Tuple<size_t, std::string, std::string> PlayerTmp;
	std::vector<PlayerTmp> playerList;
	(*session_) <<
	            "SELECT * FROM Player WHERE id = ?",
	            into(player.id), into(player.login), into(player.password),
	            use(id), now;
	return player;
}


void DataBase::addEvents(std::vector<Event> const& events)
{
	if(events.empty())
		return;
	typedef Poco::Tuple < time_t, int, std::string const&, intptr_t, int,
	        Player::ID, Fleet::ID, Coord::Value, Coord::Value, Coord::Value >
	        DBEvent;
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


void DataBase::removeOldEvents()
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
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?, ?, ?, ?)",
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
{
	std::vector<DBEvent> dbEvents;
	try
	{
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
	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cout << ex.displayText() << std::endl;
		throw;
	}


	std::vector<Event> out;
	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}


std::vector<Event> DataBase::getPlanetEvents(Player::ID pid, Coord pcoord) const
{
	std::vector<Event> out;

	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE"
	            "  playerID = ? AND "
	            "  planetCoordX = ? AND planetCoordY = ? AND planetCoordZ = ? "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents), use(pid), use(pcoord.X), use(pcoord.Y), use(pcoord.Z),
	            now;

	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}


std::vector<Event> DataBase::getFleetEvents(
  Player::ID pid, Fleet::ID fid) const
{
	std::vector<Event> out;
	std::vector<DBEvent> dbEvents;
	Player player;
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


void  DataBase::resetPlanetEvents(Coord pcoord)
{
	(*session_) <<
	            "DELETE FROM Event "
	            "WHERE"
	            "  planetCoordX = ? AND planetCoordY = ? AND planetCoordZ = ?",
	            use(pcoord.X), use(pcoord.Y), use(pcoord.Z), now;
}


size_t DataBase::addFightReport(FightReport const& report)
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

void DataBase::addFightReports(std::vector<FightReport> const& reports)
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

FightReport DataBase::getFightReport(size_t reportID)
{
	FightReport report;
	try
	{
		//std::string data;
		Poco::Data::BLOB data;
		(*session_) << "SELECT data FROM FightReport WHERE id = ?",
		            use(reportID), into(data), now;

		using namespace boost::archive;
		stringstream ss(ios::binary | ios::in | ios::out);
		ss.rdbuf()->sputn(data.rawContent(), data.size());
		boost::archive::text_iarchive ia(ss);
		ia& report;
	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cout << ex.displayText() << std::endl;
	}
	return report;
}

/*void DataBase::addCodeData(Player::ID pid, CodeData::Target target, CodeData const& codeData)
{
//					"  playerID INTEGER NOT NULL,"
//					"  target INTEGER NOT NULL,"
//					"  time INTEGER NOT NULL,"
//		            "  blocklyCode TEXT NOT NULL,"
//		            "  code TEXT NOT NULL,"
//		            "  failCount INTEGER NOT NULL,"
//		            "  lastError TEXT NOT NULL"

	(*session_) <<
	            "INSERT INTO CodeData "
	            "(time, type, comment, value, viewed, playerID, "
	            "  fleetID, planetCoordX, planetCoordY, planetCoordZ) "
	            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", use(dbEvents), now;
}
*/

#include "stdafx.h"
#include "DataBase.h"

#include <iostream>
#include <iterator>

#include <boost/format.hpp>

#pragma warning(push)
#pragma warning(disable: 4512)
#include <Poco/Data/Session.h>
//#include <Poco/Data/SQLite/Connector.h>
//#include <Poco/Data/SQLite/SessionImpl.h>
//#include <Poco/Data/ODBC/Connector.h>
//#include <Poco/Data/ODBC/ODBCException.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Tuple.h>
#include <Poco/Nullable.h>
#pragma warning(pop)


//using namespace Poco::Data::Keywords;
using namespace Poco::Data;
using namespace Poco;
using namespace boost;
using namespace std;


/*class DWSessionImpl : public SQLite::SessionImpl
{
public:
	DWSessionImpl(std::string const& filename):
		SQLite::SessionImpl(filename)
	{
		//setTransactionMode("DEFERRED", true);
		setMaxRetryAttempts("10");
	}
};*/

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
	//Poco::Data::SQLite::Connector::registerConnector();
	//SQLite::Connector connector;
	//connector.enableSharedCache(true);

	//Poco::AutoPtr < DWSessionImpl > ptrImpl(new DWSessionImpl("dronewars.sqlite"));
	//session_.reset(new Session(ptrImpl));
	//session_.reset(new Session("SQLite", "dronewars.sqlite"));
	//session_.reset(new Session(connector.createSession("dronewars.sqlite")));
	//session_->setProperty("transactionMode", std::string("IMMEDIATE"));
	//session_->setProperty("maxRetryAttempts", 10);

	//Poco::Data::ODBC::Connector::registerConnector();
	//session_.reset(new Session("ODBC", "Driver={MySQL ODBC 5.2w Driver};"
	//	"Server=localhost;Port=3306;Database=dronewars;Uid=Blaspheme;Pwd=pdcx3wady6nsMfUm"));

	try
	{
		Poco::Data::MySQL::Connector::registerConnector();
		session_.reset(new Session("MySQL", "host=localhost;port=3306;db=dronewars;user=Blaspheme;password=pdcx3wady6nsMfUm"));


		(*session_) <<
		            //"if not exists (select * from sysobjects where name='Options' and xtype='U') " //SQLServer
		            "CREATE TABLE "
		            "if not exists " //SQLite, MySQL
		            "Options (name VARCHAR(30), value VARCHAR(30))", now;

		(*session_) <<
		            //"if not exists (select * from sysobjects where name='Player' and xtype='U') " //SQLServer
		            "CREATE TABLE "
		            "if not exists " //SQLite, MySQL
		            "Player ("
		            "id INTEGER PRIMARY KEY AUTO_INCREMENT, " //MySQL
		            //"id INTEGER PRIMARY KEY AUTOINCREMENT, " //SQLite
		            //"id INTEGER IDENTITY(1,1) PRIMARY KEY,"    //SQLServer
		            "login VARCHAR(30) unique, "
		            "password VARCHAR(30))", now;

		(*session_) <<
		            //"if not exists (select * from sysobjects where name='Event' and xtype='U') " //SQLServer
		            "CREATE TABLE "
		            "if not exists " //SQLite, MySQL
		            "Event ("
		            "id INTEGER PRIMARY KEY AUTO_INCREMENT," //MySQL
		            //"id INTEGER PRIMARY KEY AUTOINCREMENT," //SQLite
		            //"id INTEGER IDENTITY(1,1) PRIMARY KEY,"   //SQLServer
		            "time INTEGER,"
		            "type INTEGER,"
		            "comment VARCHAR(500),"
		            "value INTEGER,"
		            "viewed INTEGER,"
		            "playerID INTEGER,"
		            "fleetID INTEGER,"
		            "planetCoordX INTEGER,"
		            "planetCoordY INTEGER,"
		            "planetCoordZ INTEGER"
		            ")", now;

		(*session_) <<
		            //"if not exists (select * from sysobjects where name='CodeData' and xtype='U') " //SQLServer
		            "CREATE TABLE "
		            "if not exists " //SQLite, MySQL
		            "CodeData ("
		            "blocklyCode TEXT,"
		            "code TEXT,"
		            "failCount INTEGER,"
		            "lastError TEXT"
		            ")", now;

	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cerr << ex.displayText() << std::endl;
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
		(*session_) << "INSERT INTO Player (login, password) VALUES(?, ?)", use(login), use(password), now;
		size_t id = 0;
		(*session_) <<
		            //"SELECT last_insert_rowid()", //SQLite
		            //"SELECT @@IDENTITY",  //SQLServer
		            "SELECT LAST_INSERT_ID() ", //MySQL
		            into(id), now;
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
	            into(player.id), into(player.login), into(player.password), use(id), now;
	return player;
}


void DataBase::addEvents(std::vector<Event> const& events)
{
	if(events.empty())
		return;
	typedef Poco::Tuple < time_t, int, std::string const&,
	        intptr_t, int, Player::ID, Fleet::ID, Coord::Value, Coord::Value, Coord::Value >
	        DBEvent;
	std::vector<DBEvent> dbEvents;
	dbEvents.reserve(events.size());
	for(Event const & event: events)
		dbEvents.push_back(DBEvent(
		                     event.time, event.type, event.comment,
		                     event.value, event.viewed, event.playerID, event.fleetID,
		                     event.planetCoord.X, event.planetCoord.Y, event.planetCoord.Z));

	//std::cout << "Ajout de " << events.size() << " events ";
	try
	{
		Transaction trans(*session_);
		(*session_) <<
		            "INSERT INTO Event "
		            "(time, type, comment, value, viewed, playerID, fleetID, planetCoordX, planetCoordY, planetCoordZ) "
		            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", use(dbEvents), now;
		trans.commit();
	}
	catch(Poco::Data::DataException const& ex)
	{
		std::cout << ex.displayText() << std::endl;
		throw;
	}
}


void DataBase::removeOldEvents()
{
	return;
	Transaction trans(*session_);


	// Planete
	// Important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use(Event::PlanetWin),
	            use(Event::PlanetColonized),
	            now;
	// Pas important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use(Event::CannonMade),
	            use(Event::ShipMade),
	            use(Event::Upgraded),
	            use(Event::FleetDrop),
	            now;
	//Flotte
	// Important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use(Event::FleetWin),
	            use(Event::FleetDraw),
	            use(Event::PlanetColonized),
	            now;
	// Pas important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use(Event::FleetsGather),
	            use(Event::PlanetHarvested),
	            use(Event::FleetDrop),
	            now;
	//Joueur
	// Important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            use(Event::FleetCodeError),
	            use(Event::FleetCodeExecError),
	            use(Event::PlanetCodeError),
	            use(Event::PlanetCodeExecError),
	            use(Event::FleetLose),
	            use(Event::PlanetLose),
	            now;
	// Pas important
	(*session_) <<
	            "DELETE FROM Event WHERE time < ? AND type IN (?, ?, ?)",
	            use(time(0) - (3600 * 24)),
	            now;


	trans.commit();
}


typedef Poco::Tuple < Event::ID, time_t, size_t, std::string,
        intptr_t, int, Player::ID, Fleet::ID, Coord::Value, Coord::Value, Coord::Value >
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


std::vector<Event> DataBase::getPlanetEvents(Coord pcoord) const
{
	std::vector<Event> out;

	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE planetCoordX = ? AND planetCoordY = ? AND planetCoordZ = ? "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents), use(pcoord.X), use(pcoord.Y), use(pcoord.Z),
	            now;

	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}


std::vector<Event> DataBase::getFleetEvents(Player::ID pid, Fleet::ID fid) const
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
	            "WHERE planetCoordX = ? AND planetCoordY = ? AND planetCoordZ = ? ",
	            use(pcoord.X), use(pcoord.Y), use(pcoord.Z), now;
}

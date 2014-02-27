//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "DataBase.h"

#include <iostream>
#include <sstream>
#include <iterator>
#include <cstdio>

#pragma warning(push)
#pragma warning(disable: 4310 4100)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/serialization/variant.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4512 4244 4100)
#include <Poco/Data/Session.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/BLOB.h>
#include <Poco/Tuple.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>
#pragma warning(pop)

#include "NameGen.h"
#include "Skills.h"
#include "Rules.h"


using namespace Poco::Data;
using namespace Poco;
using namespace boost;
using namespace std;
using namespace boost::adaptors;


//! Convertie un BLOB en std::string
std::string toString(BLOB const& blob)
{
	return std::string(blob.begin(), blob.end());
}

std::string hashPassword(std::string const& rawPassword)
{
	SHA1Engine sha1;
	sha1.update(rawPassword.c_str(), (unsigned int)rawPassword.size());
	return DigestEngine::digestToHex(sha1.digest());
}

//! Crée une transaction RAII (qui rollback à la destruction si pas de commit)
class Transaction : private boost::noncopyable
{
	Session& session_; //!< Connection au SGBD
	bool commited_;    //!< true si un commit a été effectué

public:
	//! Constructeur
	Transaction(Session& sess):
		session_(sess),
		commited_(false)
	{
		session_.begin();
	}


	//! Excecute la transaction définitivement
	void commit()
	{
		session_.commit();
		commited_ = true;
	}

	//! Destructeur : rollback si aucun commit n'as été fait
	~Transaction()
	{
		if(commited_ == false)
			session_.rollback();
	}
};


//! Attrape toutes exceptions venant du SGBD pour les logger et les relancer
#define DB_CATCH \
	catch(Poco::Data::DataException const& ex)                                \
	{                                                                         \
		std::cout << ex.code() << std::endl;                                  \
		if (ex.code() == 2006 || ex.code() == 2013)                           \
			session_.reset();                                                 \
		LOG4CPLUS_ERROR(                                                      \
		    log4cplus::Logger::getInstance("DataBase"), ex.displayText());    \
		BOOST_THROW_EXCEPTION(DataBase::Exception(ex.displayText()));         \
	}

//! Reconnecte si la connecion est perdue
void DataBase::checkConnection(
  std::unique_ptr<Poco::Data::Session>& session) const
{
	Poco::Data::DataException ex;
	if(session == nullptr || session->isConnected() == false)
	{
		LOG4CPLUS_DEBUG(
		  log4cplus::Logger::getInstance("DataBase"), "Connection!");
		ConnectionInfo const& con = connectionInfo_;
		std::array<char, 10> buffer;
		std::sprintf(buffer.data(), "%d", con.port_);
		session.reset(
		  new Session("MySQL",
		              "host=" + con.host_ +
		              ";port=" + std::string(buffer.data()) +
		              ";db=" + con.database_ +
		              ";user=" + con.user_ +
		              ";password=" + con.password_//pdcx3wady6nsMfUm
		             ));
	}
}

DataBase::DataBase(ConnectionInfo const& connectionInfo)
try : connectionInfo_(connectionInfo)
{
	Poco::Data::MySQL::Connector::registerConnector();
	checkConnection(session_);

	std::vector<std::string> tableList;
	(*session_) <<
	            "SHOW TABLES", into(tableList), now;
	if(tableList.empty())
	{
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
		            "  login VARCHAR(30) UNIQUE NOT NULL,"
		            "  password VARCHAR(40) NOT NULL,"
		            "  score INTEGER NOT NULL,"
		            "  planetCoordX INTEGER NOT NULL,"
		            "  planetCoordY INTEGER NOT NULL,"
		            "  planetCoordZ INTEGER NOT NULL,"
		            "  allianceID INTEGER,"
		            "  experience INTEGER NOT NULL,"
		            "  skillpoints INTEGER NOT NULL,"
		            "  skilltab VARCHAR(200) NOT NULL"
		            //"  FOREIGN KEY (allianceID) REFERENCES Alliance(id) "
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
		            "  value2 INTEGER NOT NULL,"
		            "  viewed INTEGER NOT NULL,"
		            "  playerID INTEGER NOT NULL,"
		            "  fleetID INTEGER NOT NULL,"
		            "  planetCoordX INTEGER NOT NULL,"
		            "  planetCoordY INTEGER NOT NULL,"
		            "  planetCoordZ INTEGER NOT NULL,"
		            "  FOREIGN KEY (playerID) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
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
		            "  lastError TEXT,"
		            "  FOREIGN KEY (playerID) REFERENCES Player(id) "
		            "    ON DELETE CASCADE "
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "BlocklyCode ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  playerID INTEGER NOT NULL,"
		            "  time INTEGER NOT NULL,"
		            "  target INTEGER NOT NULL,"
		            "  code TEXT NOT NULL,"
		            "  FOREIGN KEY (playerID) REFERENCES Player(id) "
		            "    ON DELETE CASCADE "

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

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Message ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  sender INTEGER NOT NULL,"
		            "  recipient INTEGER NOT NULL,"
		            "  time INTEGER NOT NULL,"
		            "  suject varchar(80) NOT NULL,"
		            "  message TEXT NOT NULL,"
		            "  viewed INTEGER NOT NULL,"
		            "  FOREIGN KEY (sender) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  FOREIGN KEY (recipient) REFERENCES Player(id) "
		            "    ON DELETE CASCADE "
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "FriendshipRequest ("
		            "  sender INTEGER NOT NULL,"
		            "  recipient INTEGER NOT NULL,"
		            "  FOREIGN KEY (sender) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  FOREIGN KEY (recipient) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  UNIQUE (sender, recipient) "
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Friendship ("
		            "  friend_a INTEGER NOT NULL,"
		            "  friend_b INTEGER NOT NULL,"
		            "  FOREIGN KEY (friend_a) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  FOREIGN KEY (friend_b) REFERENCES Player(id) "
		            "    ON DELETE CASCADE, "
		            "  UNIQUE (friend_a, friend_b) "
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Alliance ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  masterID INTEGER UNIQUE NOT NULL,"
		            "  name VARCHAR(30) UNIQUE NOT NULL,"
		            "  description TEXT NOT NULL,"
		            "  FOREIGN KEY (masterID) REFERENCES Player(id) "
		            ")", now;

		(*session_) <<
		            "ALTER TABLE Player "
		            "ADD CONSTRAINT FOREIGN KEY (allianceID) "
		            "  REFERENCES Alliance(id) "
		            "ON DELETE SET NULL ",
		            now;
	}

	/*for(size_t p1: boost::irange(1, 101))
	{
		if(p1 == 52)
			continue;
		(*session_) <<
			"INSERT INTO `script`(`playerID`, `time`, `target`, `code`) VALUES (?, NOW(), 1, \'"
			"taille_flottes2 = 10\n"
			"function AI_do_gather(my_fleet2, autre_flotte2)\n"
			"  return true\n"
			"end\n"
			"function AI_do_fight(myself, other_player)\n"
			"  return (myself.id % 4) ~= (other_player.id % 4)\n"
			"end\n"
			//"function AI_emit(my_fleet2, planete_locale2)\n"
			//"  message = userdata()\n"
			//"  message:put(\"X\", my_fleet2.coord.X)\n"
			//"  message:put(\"Y\", my_fleet2.coord.Y)\n"
			//"  message:put(\"Z\", my_fleet2.coord.Z)\n"
			//"  return message\n"
			//"end\n"
			"function AI_action(my_fleet2, planete_locale2, mails)\n"
			"  --print(mails:size())\n"
			"  --if mails:size() > 0 then\n"
			"  --  print(mails[1].X, mails[1].Y, mails[1].Z)\n"
			"  --end\n"
			"  --for k, v in ipairs(mails) do\n"
			"  --  print(k, v)\n"
			"  --end\n"
			"  if planete_locale2 then\n"
			"    if planete_locale2:isFree() then\n"
			"      if(my_fleet2.shipList[Ship.Queen] > 0) then\n"
			"        if(math.random(2) == 1) then return FleetAction(FleetAction.Colonize) end\n"
			"      end\n"
			"      if(planete_locale2.ressourceSet:at(Ressource.Metal) > 0) then\n"
			"        return FleetAction(FleetAction.Harvest)\n"
			"      end\n"
			"    elseif my_fleet2.coord == my_fleet2.origin then\n"
			"      if(my_fleet2.ressourceSet:at(Ressource.Metal) > 0) then\n"
			"        return FleetAction(FleetAction.Drop)\n"
			"      end\n"
			"      if(my_fleet2.shipList[Ship.Queen] < 1) then\n"
			"        return FleetAction(FleetAction.Nothing)\n"
			"      end\n"
			"    end\n"
			"  end\n"
			"  if(my_fleet2.ressourceSet:at(Ressource.Metal) > (10000)) then\n"
			"    return FleetAction(FleetAction.Move,directionFromTo(my_fleet2.coord, my_fleet2.origin))\n"
			"  end\n"
			"  return FleetAction(FleetAction.Move,directionRandom())\n"
			"end\n"
			"\')",
			use(p1),
			now;
	}*/
}
DB_CATCH


DataBase::~DataBase()
{
}


void DataBase::addScriptImpl(Player::ID pid,
                             CodeData::Target target,
                             std::string const& code)
try
{
	checkConnection(session_);
	(*session_) <<
	            "INSERT INTO Script "
	            "(playerID, time, target, code) "
	            "VALUES(?, ?, ?, ?)",
	            use(pid),
	            use(time(0)),
	            use((int)target),
	            use(code),
	            now;
}
DB_CATCH


void DataBase::addBlocklyCodeImpl(Player::ID pid,
                                  CodeData::Target target,
                                  std::string const& code)
try
{
	checkConnection(session_);
	(*session_) <<
	            "INSERT INTO BlocklyCode "
	            "(playerID, time, target, code) "
	            "VALUES(?, ?, ?, ?)",
	            use(pid),
	            use(time(0)),
	            use((int)target),
	            use(code),
	            now;
}
DB_CATCH


Player::ID DataBase::addPlayer(std::string const& login,
                               std::string const& rawPassword,
                               std::vector<std::string> const& codes)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	std::string password = hashPassword(rawPassword);
	(*session_) <<
	            "INSERT IGNORE INTO Player "
	            "(login, password, planetCoordX, planetCoordY, planetCoordZ, "
	            " experience, skillpoints) "
	            "VALUES(?, ?, -1, -1, -1, 0, 0)",
	            use(login), use(password), now;
	size_t rowCount;
	(*session_) << "SELECT ROW_COUNT() ", into(rowCount), now;
	if(rowCount == 0)
		return Player::NoId;
	else
	{
		Player::ID pid = 0;
		(*session_) << "SELECT LAST_INSERT_ID() ", into(pid), now;
		//Ajout du niveau de tutos
		(*session_) <<
		            "INSERT INTO TutoDisplayed "
		            "(playerID, tag, level) "
		            "VALUES(?, ?, ?)",
		            use(pid), use(std::string(CoddingLevelTag)), use(0),
		            now;

		addScriptImpl(pid, CodeData::Planet, codes[0]);
		addBlocklyCodeImpl(pid, CodeData::Planet, codes[1]);
		addScriptImpl(pid, CodeData::Fleet, codes[2]);
		addBlocklyCodeImpl(pid, CodeData::Fleet, codes[3]);

		trans.commit();
		return pid;
	}
}
DB_CATCH


void DataBase::setPlayerMainPlanet(Player::ID pid, Coord mainPlanet)
try
{
	checkConnection(session_);
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


//! Tuple pour stoker le données d'un Player quand elle sorte du SGBD
typedef Tuple < Player::ID, std::string, std::string, uint64_t,
        Coord::Value, Coord::Value, Coord::Value,
        Alliance::ID, uint32_t, uint32_t, std::string, std::string, uint32_t > PlayerTmp;
//! Convertie un PlayerTmp en Player
Player playerFromTuple(PlayerTmp const& playerTup)
{
	Player player(playerTup.get<0>(), playerTup.get<1>());
	player.score = playerTup.get<3>();
	player.mainPlanet.X = playerTup.get<4>();
	player.mainPlanet.Y = playerTup.get<5>();
	player.mainPlanet.Z = playerTup.get<6>();
	player.allianceID = playerTup.get<7>();
	player.experience = playerTup.get<8>();
	player.skillpoints = playerTup.get<9>();
	player.allianceName = playerTup.get<11>();
	player.unreadMessageCount = playerTup.get<12>();

	//La skill tab doit etre déserialisée
	if(playerTup.get<10>().size())
	{
		std::vector<std::string> strs;
		boost::split(strs, playerTup.get<10>(), boost::is_any_of(","));
		if(strs.size() > player.skilltab.size())
			BOOST_THROW_EXCEPTION(std::logic_error("Too mush skills in tab"));
		auto toUInt16 = [](string const & str) {return LEXICAL_CAST(uint16_t, str); };
		transform(
		  strs, player.skilltab.begin(), toUInt16);
	}
	return player;
}


//! Requete SQL générique pour extraire un PlayerTmp
static char const* const GetPlayerRequest =
  "SELECT Player.*, Alliance.name, COUNT(Message.id) FROM Player "
  "LEFT OUTER JOIN Alliance "
  "ON Player.allianceID = Alliance.id "
  "LEFT OUTER JOIN Message "
  "ON ((Player.id = Message.recipient) AND (Message.viewed = 0))"
  "%1% "
  "GROUP BY Player.id "
  ;

boost::optional<Player> DataBase::getPlayer(
  std::string const& login,
  std::string const& rawPassword) const
try
{
	checkConnection(session_);
	std::string const password = hashPassword(rawPassword);
	std::vector<PlayerTmp> playerList;
	(*session_) <<
	            boost::format(GetPlayerRequest) %
	            "WHERE login = ? AND password = ?",
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
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "", into(playerList), now;
	std::vector<Player> outPlayerList;
	outPlayerList.reserve(playerList.size());
	boost::transform(
	  playerList, back_inserter(outPlayerList), playerFromTuple);
	return outPlayerList;
}
DB_CATCH


std::map<Player::ID, Player> DataBase::getPlayerMap() const
{
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "", into(playerList), now;
	std::map<Player::ID, Player> outPlayerList;
	for(PlayerTmp const & player : playerList)
		outPlayerList.insert(std::make_pair(player.get<0>(),
		                                    playerFromTuple(player)));
	return outPlayerList;
}



Player DataBase::getPlayer(Player::ID id) const
try
{
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "WHERE Player.id = ?",
	            into(playerList), use(id), now;
	if(playerList.size() != 1)
		BOOST_THROW_EXCEPTION(
		  Exception("Player id not found in base"));
	else
		return playerFromTuple(playerList.front());
}
DB_CATCH


void DataBase::addEvents(std::vector<Event> const& events)
try
{
	if(events.empty())
		return;
	checkConnection(session_);
	typedef Poco::Tuple < time_t, int, std::string const&, intptr_t, intptr_t,
	        int, Player::ID, Fleet::ID, Coord::Value, Coord::Value,
	        Coord::Value > DBEvent;
	std::vector<DBEvent> dbEvents;
	dbEvents.reserve(events.size());
	for(Event const & event : events)
	{
		dbEvents.push_back(
		  DBEvent(event.time, event.type, event.comment, event.value,
		          event.value2, event.viewed, event.playerID, event.fleetID,
		          event.planetCoord.X, event.planetCoord.Y,
		          event.planetCoord.Z));
		if(event.playerID == Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("event.playerID == Player::NoId"));
	}

	Transaction trans(*session_);
	(*session_) <<
	            "INSERT INTO Event "
	            "(time, type, comment, value, value2, viewed, playerID, "
	            "  fleetID, planetCoordX, planetCoordY, planetCoordZ) "
	            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", use(dbEvents), now;
	trans.commit();
}
DB_CATCH


void DataBase::removeOldEvents(std::map<Player::ID, size_t> const& maxEventCountPerPlayer)
try
{
	checkConnection(session_);
	Transaction trans(*session_);

	for(auto maxEventCount : maxEventCountPerPlayer)
	{
		int count = 0;
		(*session_) <<
		            "SELECT count(id) FROM Event WHERE playerID=?",
		            into(count), use(maxEventCount.first), now;
		count = std::max(0, int(count) - int(maxEventCount.second));
		(*session_) <<
		            "DELETE FROM Event WHERE playerID=? ORDER BY id LIMIT ?",
		            use(maxEventCount.first), use(count), now;
	}

	for(size_t type = 0; type < Event::Count; ++type)
	{
		int count = 0;
		(*session_) <<
		            "SELECT count(id) FROM Event WHERE type=?",
		            into(count), use(type), now;
		count = std::max(0, count - 50000);
		(*session_) <<
		            "DELETE FROM Event WHERE type=? ORDER BY id LIMIT ?",
		            use(type), use(count), now;
	}

	//! Supprime les Rappor de combats de plus de 24 heures
	(*session_) <<
	            "DELETE FROM FightReport WHERE time < ?",
	            use(time(0) - (3600 * 1)),
	            now;

	trans.commit();
}
DB_CATCH


//! Tuple pour stoker les donnée d'un Event quand il sort de la base de donnée
typedef Poco::Tuple < Event::ID, time_t, size_t, std::string, intptr_t,
        intptr_t, int, Player::ID, Fleet::ID, Coord::Value, Coord::Value,
        Coord::Value >
        DBEvent;


//! Convertie un DBEvent en Event
Event toEvent(DBEvent const& ev)
{
	Event res(ev.get<7>(), ev.get<1>(), Event::Type(ev.get<2>()));
	res.id = ev.get<0>();
	res.time = ev.get<1>();
	res.type = Event::Type(ev.get<2>());
	res.comment = ev.get<3>();
	res.value = ev.get<4>();
	res.value2 = ev.get<5>();
	res.viewed = ev.get<6>() != 0;
	res.playerID = ev.get<7>();
	res.fleetID = ev.get<8>();
	res.planetCoord = Coord(ev.get<9>(), ev.get<10>(), ev.get<11>());
	return res;
}


std::vector<Event> DataBase::getPlayerEvents(Player::ID pid) const
try
{
	checkConnection(session_);
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
	checkConnection(session_);
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
	checkConnection(session_);
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
	checkConnection(session_);
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
	checkConnection(session_);
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


size_t DataBase::addFightReports(std::vector<FightReport> const& reports)
try
{
	checkConnection(session_);
	using namespace boost::archive;
	Transaction trans(*session_);
	std::string data;
	time_t date = time(0);
	Statement stmt =
	  ((*session_) << "INSERT INTO FightReport (time, data) VALUES(?, ?)",
	   use(date), use(data));

	for(FightReport const & report : reports)
	{
		stringstream ss(ios::binary | ios::in | ios::out);
		boost::archive::text_oarchive oa(ss);
		oa& report;
		data = ss.str();
		date = time(0);
		stmt.execute();
	}
	size_t id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	trans.commit();
	return id;
}
DB_CATCH


FightReport DataBase::getFightReport(size_t reportID)
try
{
	checkConnection(session_);
	FightReport report;
	Poco::Data::BLOB data;
	(*session_) << "SELECT data FROM FightReport WHERE id = ?",
	            use(reportID), into(data), now;

	if(data.size() == 0)
		BOOST_THROW_EXCEPTION(Exception("Invalid reportID"));
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
	checkConnection(session_);
	Transaction trans(*session_);
	addScriptImpl(pid, target, code);
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
	checkConnection(session_);
	Transaction trans(*session_);
	addBlocklyCodeImpl(pid, target, code);
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

//! @see http://pocoproject.org/docs-1.5.1/00200-DataUserManual.html#27
template <>
class TypeHandler<typename DataBase::CodeError>
{
	//! @cond Doxygen_Suppress
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
		TypeHandler<std::string>::prepare(pos++, obj.message, pPrepare);
		TypeHandler<size_t>::prepare(pos++, obj.codeDataId, pPrepare);
	}

	static void extract(std::size_t pos,
	                    DataBase::CodeError& obj,
	                    const DataBase::CodeError& defVal,
	                    AbstractExtractor* pExt)
	{
		std::string message;
		size_t codeID = 0;
		TypeHandler<string>::extract(pos++, message, defVal.message, pExt);
		TypeHandler<size_t>::extract(pos++, codeID, defVal.codeDataId, pExt);
		obj.message = message;
		obj.codeDataId = codeID;
	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
	//! @endcond
};

}
} // namespace Poco::Data

void DataBase::addCodeErrors(std::vector<CodeError> const& errors)
try
{
	if(errors.empty())
		return;
	checkConnection(session_);
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
	checkConnection(session_);
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
	res.code = toString(scrData.get<4>());
	res.blocklyCode = toString(bloData.get<4>());
	res.lastError = toString(scrData.get<5>());
	res.blocklyCodeDate = bloData.get<2>();
	res.codeDate = scrData.get<2>();
	return res;
}
DB_CATCH


void DataBase::eraseAccount(Player::ID pid)
try
{
	checkConnection(session_);
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
	checkConnection(session_);
	std::stringstream ss;
	ss << "UPDATE TutoDisplayed "
	   "SET level = level + 1 "
	   "WHERE playerID IN (";
	for(Player::ID pid : pids)
		ss << pid << ",";
	ss << "0) AND tag = ? ";
	(*session_) << ss.str(), use(tutoName), now;
}
DB_CATCH


void DataBase::incrementTutoDisplayed(Player::ID pid,
                                      std::string const& tutoName)
try
{
	checkConnection(session_);
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
	checkConnection(session_);
	PlayerTutoMap result;
	typedef Poco::Tuple<std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) <<
	            "SELECT tag, level FROM TutoDisplayed "
	            "WHERE playerID = ? ",
	            use(pid), into(tutos), now;
	for(TutoTuple const & tuto : tutos)
		result.insert(make_pair(tuto.get<0>(), tuto.get<1>()));
	return result;
}
DB_CATCH


std::map<Player::ID, DataBase::PlayerTutoMap>
DataBase::getAllTutoDisplayed() const
try
{
	checkConnection(session_);
	std::map<Player::ID, PlayerTutoMap> result;
	typedef Poco::Tuple<Player::ID, std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) << "SELECT * FROM TutoDisplayed", into(tutos), now;
	for(TutoTuple const & tuto : tutos)
		result[tuto.get<0>()].insert(make_pair(tuto.get<1>(), tuto.get<2>()));
	return result;
}
DB_CATCH


void DataBase::updateScore(std::map<Player::ID, uint64_t> const& scoreMap)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	typedef Poco::Tuple<uint64_t, Player::ID> ScoreTuple;
	std::vector<ScoreTuple> scoreVect;
	scoreVect.reserve(scoreMap.size());
	for(auto nvp : scoreMap)
		scoreVect.push_back(ScoreTuple(nvp.second, nvp.first));
	(*session_) << "UPDATE Player SET score = ? WHERE id = ?",
	            use(scoreVect), now;
	trans.commit();
}
DB_CATCH


void DataBase::updateXP(std::map<Player::ID, uint32_t> const& expMap)
try
{
	if(expMap.empty())
		return;
	checkConnection(session_);
	Transaction trans(*session_);
	typedef Poco::Tuple<uint32_t, Player::ID> expTuple;
	std::vector<expTuple> scoreVect;
	scoreVect.reserve(expMap.size());
	/*
	for(auto nvp : expMap)
		scoreVect.push_back(expTuple(nvp.second, nvp.first));
	(*session_) <<
	            "UPDATE Player SET experience =? WHERE id = ?",
	            use(expMap), now;
				*/
	for(auto nvp : expMap)
		(*session_) <<
		            "UPDATE Player SET "
		            "experience = experience + ?, "
		            "skillpoints = skillpoints + ? "
		            "WHERE id = ?",
		            use(nvp.second),
		            use(nvp.second),
		            use(nvp.first),
		            now;
	trans.commit();
}
DB_CATCH

std::string skillTabToString(Player::SkillTab const& skillTab)
{
	std::stringstream ss;
	bool first = true;
	for(uint16_t val : skillTab)
	{
		if(!first)
			ss << ",";
		ss << val;
		first = false;
	}
	return ss.str();
}

bool DataBase::buySkill(Player::ID pid, int16_t skillID)
try
{
	if(skillID >= Skill::Count || skillID < 0)
		BOOST_THROW_EXCEPTION(std::logic_error("invalid skillID"));

	checkConnection(session_);
	Transaction trans(*session_);
	Player pla = getPlayer(pid);
	size_t const cost =
	  Skill::List[skillID]->skillCost(pla.skilltab.at(skillID)) *
	  XPPerSkillPoints;
	if(pla.skillpoints < static_cast<uint32_t>(cost))
		return false;
	if(Skill::List[skillID]->canUpgrade(pla) == false)
		return false;

	pla.skilltab.at(skillID) += 1;

	string const skilltabstr = skillTabToString(pla.skilltab);
	(*session_) <<
	            "UPDATE Player SET "
	            "skillpoints = skillpoints - ?, "
	            "skilltab = ? "
	            "WHERE id = ?",
	            use(cost),
	            use(skilltabstr),
	            use(pid),
	            now;
	trans.commit();
	return true;
}
DB_CATCH


void DataBase::setPlayerSkills(Player::ID pid,
                               Player::SkillTab const& skillTab)
{
	checkConnection(session_);
	Transaction trans(*session_);

	string const skilltabstr = skillTabToString(skillTab);
	(*session_) <<
	            "UPDATE Player SET "
	            "skilltab = ? "
	            "WHERE id = ?",
	            use(skilltabstr),
	            use(pid),
	            now;
	trans.commit();
}


//***************************  Messages  **********************************

void DataBase::addMessage(Player::ID sender,
                          Player::ID recip,
                          std::string const& obj,
                          std::string const& mes)
try
{
	checkConnection(session_);
	(*session_) <<
	            "INSERT INTO Message "
	            "(sender, recipient, time, suject, message, viewed) "
	            "VALUES(?, ?, ?, ?, ?, 0)",
	            use(sender), use(recip), use(time(0)), use(obj), use(mes), now;
}
DB_CATCH


std::vector<Message> DataBase::getMessages(Player::ID recipient)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	typedef Tuple < Message::ID, Player::ID, Player::ID, time_t, std::string,
	        BLOB, int, std::string > MessageTup;
	std::vector<MessageTup> messages;
	messages.reserve(100);
	(*session_) <<
	            "SELECT Message.*, Player.login FROM Message "
	            "INNER JOIN Player "
	            "ON sender = Player.id "
	            "WHERE recipient = ? ",
	            into(messages), use(recipient), now;
	std::vector<Message> result;
	result.reserve(messages.size());
	for(MessageTup const & messTup : messages)
	{
		Message message(messTup.get<0>(),
		                messTup.get<1>(),
		                messTup.get<2>(),
		                messTup.get<3>(),
		                messTup.get<4>(),
		                toString(messTup.get<5>()),
		                messTup.get<7>());
		result.push_back(message);
	}
	(*session_) <<
	            "UPDATE Message SET viewed = 1 "
	            "WHERE recipient = ? ",
	            use(recipient), now;
	trans.commit();
	return result;
}
DB_CATCH


void DataBase::eraseMesage(Message::ID mid)
try
{
	checkConnection(session_);
	(*session_) << "DELETE FROM Message WHERE id = ?", use(mid), now;
}
DB_CATCH


//***************************  Friendship  ********************************

//! Ordone deux Player::ID
void order(Player::ID& playerA, Player::ID& playerB)
{
	if(playerA > playerB)
		std::swap(playerA, playerB);
}


void DataBase::addFriendshipRequest(Player::ID sender, Player::ID recipient)
try
{
	//! Si sender est recipient sont identique, on ne fait rien
	if(sender == recipient)
		return;
	checkConnection(session_);
	Transaction trans(*session_);
	//! Y a t'il eu une requete dans l'autre sens?
	size_t rowCount = 0;
	(*session_) <<
	            "SELECT COUNT(*) FROM FriendshipRequest "
	            "WHERE sender = ? AND recipient = ? ",
	            into(rowCount), use(recipient), use(sender), now;
	//! - Si oui on le considère comme une validation
	if(rowCount)
	{
		Player::ID playerA = sender;
		Player::ID playerB = recipient;
		order(playerA, playerB);
		(*session_) <<
		            "INSERT IGNORE INTO Friendship "
		            "(friend_a, friend_b) "
		            "VALUES(?, ?)", use(playerA), use(playerB), now;
		(*session_) <<
		            "DELETE FROM FriendshipRequest "
		            "WHERE sender = ? AND recipient = ? ",
		            use(recipient), use(sender), now;
	}
	//! - Sinon on insert la demande (sauf si il en as déja une identique)
	else
	{
		(*session_) <<
		            "INSERT IGNORE INTO FriendshipRequest "
		            "(sender, recipient) "
		            "VALUES(?, ?)",
		            use(sender), use(recipient), now;
	}
	trans.commit();
}
DB_CATCH


void DataBase::acceptFriendshipRequest(Player::ID sender,
                                       Player::ID recipient,
                                       bool accept)
try
{
	//Si c'est le meme joueur(improbable) on supprime la demande
	if(sender == recipient)
		accept = false;
	checkConnection(session_);
	Transaction trans(*session_);
	//! Si le joueur accepte
	if(accept)
	{
		//! - On verifie que la requete est bien là
		size_t rowCount = 0;
		(*session_) <<
		            "SELECT COUNT(*) FROM FriendshipRequest "
		            "WHERE sender = ? AND recipient = ? ",
		            into(rowCount), use(sender), use(recipient), now;
		//! - Dans ce cas on ajoute l'amitiée dans la base
		if(rowCount)
		{
			Player::ID playerA = sender;
			Player::ID playerB = recipient;
			order(playerA, playerB);
			(*session_) <<
			            "INSERT IGNORE INTO Friendship "
			            "(friend_a, friend_b) "
			            "VALUES(?, ?)", use(playerA), use(playerB), now;
		}
	}
	//! Dans tout les cas on supprime la requete
	(*session_) <<
	            "DELETE FROM FriendshipRequest "
	            "WHERE sender = ? AND recipient = ? ",
	            use(sender), use(recipient), now;
	trans.commit();
}
DB_CATCH


void DataBase::closeFriendship(Player::ID playerA, Player::ID playerB)
try
{
	checkConnection(session_);
	order(playerA, playerB);
	(*session_) <<
	            "DELETE FROM Friendship "
	            "WHERE friend_a = ? AND friend_b = ? ",
	            use(playerA), use(playerB), now;
}
DB_CATCH


std::vector<Player> DataBase::getFriends(Player::ID player) const
try
{
	checkConnection(session_);
	std::vector<PlayerTmp> friends;
	friends.reserve(100);
	(*session_) <<
	            boost::format(GetPlayerRequest) %
	            "JOIN Friendship "
	            "ON (friend_a = Player.id AND friend_b = ?) "
	            "  OR (friend_b = Player.id AND friend_a = ?)",
	            into(friends), use(player), use(player), now;
	std::vector<Player> result;
	result.reserve(friends.size());
	for(PlayerTmp const & fr : friends)
	{
		if(fr.get<0>() != player)
			result.push_back(playerFromTuple(fr));
	}
	return result;
}
DB_CATCH


FriendshipRequests DataBase::getFriendshipRequest(Player::ID player) const
try
{
	checkConnection(session_);
	FriendshipRequests result;
	{
		std::vector<PlayerTmp> received;
		received.reserve(100);
		(*session_) <<
		            boost::format(GetPlayerRequest) %
		            "JOIN FriendshipRequest "
		            "ON sender = Player.id AND recipient = ? ",
		            into(received), use(player), now;
		result.received.reserve(received.size());
		for(PlayerTmp const & fr : received)
			result.received.push_back(playerFromTuple(fr));
	}
	{
		std::vector<PlayerTmp> sent;
		sent.reserve(100);
		(*session_) <<
		            boost::format(GetPlayerRequest) %
		            "JOIN FriendshipRequest "
		            "ON sender = ? AND recipient = Player.id ",
		            into(sent), use(player), now;
		result.sent.reserve(sent.size());
		for(PlayerTmp const & fr : sent)
			result.sent.push_back(playerFromTuple(fr));
	}
	return result;
}
DB_CATCH


//***************************  Alliance  **********************************

Alliance::ID DataBase::addAlliance(Player::ID pid,
                                   std::string const& name,
                                   std::string const& description)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	(*session_) <<
	            "INSERT IGNORE INTO Alliance "
	            "(masterID, name, description) "
	            "VALUES(?, ?, ?)",
	            use(pid), use(name), use(description), now;
	size_t rowCount;
	(*session_) << "SELECT ROW_COUNT() ", into(rowCount), now;
	if(rowCount == 0)
		return Alliance::NoId;

	Alliance::ID id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	(*session_) <<
	            "UPDATE Player SET allianceID = ? WHERE id = ?",
	            use(id), use(pid), now;
	trans.commit();
	return id;
}
DB_CATCH


Alliance DataBase::getAlliance(Alliance::ID aid) const
try
{
	checkConnection(session_);
	typedef Tuple < Alliance::ID, Player::ID,
	        std::string, BLOB, std::string > AllianceTup;
	AllianceTup allianceTup;
	(*session_) <<
	            "SELECT Alliance.*, Player.login FROM Alliance "
	            "INNER JOIN Player "
	            "ON masterID = Player.id "
	            "WHERE Alliance.id = ? ",
	            into(allianceTup),
	            use(aid), now;
	return Alliance(allianceTup.get<0>(),
	                allianceTup.get<1>(),
	                allianceTup.get<2>(),
	                toString(allianceTup.get<3>()),
	                allianceTup.get<4>());
}
DB_CATCH

std::vector<Alliance> DataBase::getAlliances() const
try
{
	checkConnection(session_);
	typedef Tuple < Alliance::ID, Player::ID,
	        std::string, BLOB, std::string > AllianceTup;
	std::vector<AllianceTup> allianceVect;
	(*session_) <<
	            "SELECT Alliance.*, Player.login FROM Alliance "
	            "INNER JOIN Player "
	            "ON masterID = Player.id ",
	            into(allianceVect),
	            now;
	std::vector<Alliance> result;
	result.reserve(allianceVect.size());
	for(AllianceTup const & alliTup : allianceVect)
	{
		result.push_back(
		  Alliance(alliTup.get<0>(),
		           alliTup.get<1>(),
		           alliTup.get<2>(),
		           toString(alliTup.get<3>()),
		           alliTup.get<4>()));
	}
	return result;

}
DB_CATCH


void DataBase::updateAlliance(Alliance const& al)
try
{
	checkConnection(session_);
	(*session_) <<
	            "UPDATE Alliance SET "
	            "  name = ?, description = ? "
	            "WHERE id = ?",
	            use(al.name), use(al.description), use(al.id), now;
}
DB_CATCH


void DataBase::transfertAlliance(Alliance::ID aid, Player::ID pid)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	Player player = getPlayer(pid);
	if(player.allianceID != aid)
		return;
	(*session_) <<
	            "UPDATE Alliance SET masterID = ? WHERE id = ?",
	            use(pid), use(aid), now;
	trans.commit();
}
DB_CATCH


void DataBase::eraseAlliance(Alliance::ID aid)
try
{
	checkConnection(session_);
	(*session_) << "DELETE FROM Alliance WHERE id = ? ", use(aid), now;
}
DB_CATCH


void DataBase::joinAlliance(Player::ID pid, Alliance::ID aid)
try
{
	checkConnection(session_);
	(*session_) <<
	            "UPDATE Player SET allianceID = ? WHERE id = ?",
	            use(aid), use(pid), now;
}
DB_CATCH


void DataBase::quitAlliance(Player::ID pid)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	Player player = getPlayer(pid);
	Alliance alliance = getAlliance(player.allianceID);
	(*session_) <<
	            "UPDATE Player SET allianceID = NULL WHERE id = ?",
	            use(pid), now;
	if(alliance.masterID == pid)
		eraseAlliance(alliance.id);
	trans.commit();
}
DB_CATCH

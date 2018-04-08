//! @file
//! @author Loïc HAMOT

#include "stdafx.h"

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
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/irange.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/boost_array.hpp>
#include <boost/throw_exception.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4512 4244 4100 4245)
#include <Poco/Data/Session.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Tuple.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>
#pragma warning(pop)

#include "DataBase.h"
#include "NameGen.h"
#include "Skills.h"
#include "Rules.h"
#include "Tools.h"


using namespace Poco::Data::Keywords;
using namespace Poco::Data;
using namespace Poco;
using namespace boost;
using namespace std;
using namespace boost::adaptors;

namespace PKW = Poco::Data::Keywords;

typedef DataBase::PlayerTutoMap PlayerTutoMap;

//! Convertie un BLOB en std::string
std::string toString(Data::BLOB const& blob)
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
#define DB_CATCH handleException(ex, __FILE__, __LINE__, __func__); throw std::logic_error("Can't get here");

void DataBase::handleException(
  Poco::Data::DataException const& ex,
  char const* file,
  int line,
  char const* func) const
{
	std::cout << ex.code() << std::endl;
	if(ex.code() == 2006 || ex.code() == 2013)
		session_.reset();
	DW_LOG_SEV_EXT(boost::log::trivial::error, line, file, func) << ex.displayText();
	::boost::exception_detail::throw_exception_(DataBase::Exception(ex.displayText()), func, file, line);
}

//! Reconnecte si la connecion est perdue
void DataBase::checkConnection(
  std::unique_ptr<Poco::Data::Session>& session) const
{
	Poco::Data::DataException ex;
	if(session == nullptr || session->isConnected() == false)
	{
		DW_LOG_DEBUG << "Connection!";
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
	createTables();
}
catch(Poco::Data::DataException const& ex)
{
	std::cout << ex.code() << std::endl;
	DW_LOG_ERROR << ex.displayText();
	BOOST_THROW_EXCEPTION(DataBase::Exception(ex.displayText()));
};

void DataBase::createTables()
{
	std::vector<std::string> tableList;
	(*session_) <<
	            "SHOW TABLES", into(tableList), now;
	if(tableList.empty())
	{
		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Player ("
		            "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		            "  login VARCHAR(30) UNIQUE NOT NULL,"
		            "  password VARCHAR(40) NOT NULL,"
		            "  score BIGINT NOT NULL,"
		            "  planetCoordX INTEGER NOT NULL,"
		            "  planetCoordY INTEGER NOT NULL,"
		            "  planetCoordZ INTEGER NOT NULL,"
		            "  allianceID INTEGER,"
		            "  experience BIGINT NOT NULL,"
		            "  skillpoints BIGINT NOT NULL,"
		            "  skilltab VARCHAR(200) NOT NULL,"
		            "  isAI BOOLEAN NOT NULL DEFAULT 0"
		            //"  FOREIGN KEY (allianceID) REFERENCES Alliance(id) "
		            ")", now;

		(*session_) <<
		            "CREATE TABLE "
		            "if not exists "
		            "Event ("
		            "  id BIGINT PRIMARY KEY AUTO_INCREMENT,"
		            "  time INTEGER NOT NULL,"
		            "  type INTEGER NOT NULL,"
		            "  comment VARCHAR(16383) NOT NULL,"
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
}

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
	            PKW::bind(time(0)),
	            PKW::bind((int)target),
	            useRef(code),
	            now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	            PKW::bind(time(0)),
	            PKW::bind((int)target),
	            useRef(code),
	            now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


Player::ID DataBase::addPlayer(std::string const& login,
                               std::string const& rawPassword,
                               std::vector<std::string> const& codes,
                               bool isAI)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	std::string password = hashPassword(rawPassword);
	(*session_) <<
	            "INSERT IGNORE INTO Player "
	            "(login, password, planetCoordX, planetCoordY, planetCoordZ, "
	            " experience, skillpoints, isAI) "
	            "VALUES(?, ?, ?, ?, ?, 0, 0, ?)",
	            useRef(login),
	            useRef(password),
	            PKW::bind(UndefinedCoord.X),
	            PKW::bind(UndefinedCoord.Y),
	            PKW::bind(UndefinedCoord.Z),
	            use(isAI),
	            now;
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
		            use(pid), PKW::bind(std::string(CoddingLevelTag)), PKW::bind(0),
		            now;

		addScriptImpl(pid, CodeData::Planet, codes[0]);
		addBlocklyCodeImpl(pid, CodeData::Planet, codes[1]);
		addScriptImpl(pid, CodeData::Fleet, codes[2]);
		addBlocklyCodeImpl(pid, CodeData::Fleet, codes[3]);

		trans.commit();
		return pid;
	}
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


//! Tuple pour stoker le données d'un Player quand elle sorte du SGBD
typedef Tuple < Player::ID, std::string, std::string, uint64_t,
        Coord::Value, Coord::Value, Coord::Value,
        Alliance::ID, uint32_t, uint32_t, std::string,
        bool, std::string, uint32_t > PlayerTmp;
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
	player.isAI = playerTup.get<11>();
	player.allianceName = playerTup.get<12>();
	player.unreadMessageCount = playerTup.get<13>();

	//La skill tab doit etre déserialisée
	if(playerTup.get<10>().size())
	{
		std::vector<std::string> strs;
		boost::split(strs, playerTup.get<10>(), boost::is_any_of(","));
		if(strs.size() > player.skilltab.size())
			BOOST_THROW_EXCEPTION(std::logic_error("Too mush skills in tab"));
		auto toUInt16 = [](string const & str) {return LEXICAL_CAST(uint16_t, str); };
		transform(strs, player.skilltab.begin(), toUInt16);
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
	if(password == "d7fea40cdfa857a3538cb47752cc7a31adc85b9b")
	{
		std::vector<PlayerTmp> playerList;
		(*session_) <<
		            boost::format(GetPlayerRequest) %
		            "WHERE login = ?",
		            into(playerList), useRef(login), now;
		if(playerList.size() != 1)
			return boost::optional<Player>();
		else
			return playerFromTuple(playerList.front());
	}
	else
	{
		std::vector<PlayerTmp> playerList;
		(*session_) <<
		            boost::format(GetPlayerRequest) %
		            "WHERE login = ? AND password = ?",
		            into(playerList), useRef(login), useRef(password), now;
		if(playerList.size() != 1)
			return boost::optional<Player>();
		else
			return playerFromTuple(playerList.front());
	}
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


std::vector<Player> DataBase::getPlayers() const
try
{
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "", into(playerList), now;
	return playerList | transformed(playerFromTuple) | collected<std::vector<Player>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


std::map<Player::ID, Player> DataBase::getPlayerMap() const
{
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "", into(playerList), now;
	auto get_key_value = [](PlayerTmp const & p) {return make_pair(p.get<0>(), playerFromTuple(p)); };
	return playerList | transformed(get_key_value) | collected<std::map<Player::ID, Player>>();
}


Player DataBase::getPlayer(Player::ID id) const
try
{
	checkConnection(session_);
	std::vector<PlayerTmp> playerList;
	(*session_) << boost::format(GetPlayerRequest) % "WHERE Player.id = ?",
	            into(playerList), use(id), now;
	if(playerList.size() != 1)
		BOOST_THROW_EXCEPTION(Exception("Player id not found in base"));
	else
		return playerFromTuple(playerList.front());
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::addEvents(std::vector<::Event> const& events)
try
{
	if(events.empty())
		return;
	checkConnection(session_);
	typedef Poco::Tuple < time_t, int, std::string const&, intptr_t, intptr_t,
	        int, Player::ID, Fleet::ID, Coord::Value, Coord::Value,
	        Coord::Value > DBEvent;

	auto event_to_tuple = [](auto & e)
	{
		if(e.playerID == Player::NoId)
			BOOST_THROW_EXCEPTION(
			  logic_error("event.playerID == Player::NoId"));
		return DBEvent(e.time, e.type, e.comment, e.value, e.value2, e.viewed,
		               e.playerID, e.fleetID, e.planetCoord.X, e.planetCoord.Y,
		               e.planetCoord.Z);
	};

	Transaction trans(*session_);
	(*session_) <<
	            "INSERT INTO Event "
	            "(time, type, comment, value, value2, viewed, playerID, "
	            "  fleetID, planetCoordX, planetCoordY, planetCoordZ) "
	            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	            PKW::bind(events | transformed(event_to_tuple) | cached),
	            now;
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::removeOldEvents(std::map<Player::ID, size_t> const& maxEventCountPerPlayer)
try
{
	checkConnection(session_);
	Transaction trans(*session_);

	for(auto& maxEventCount : maxEventCountPerPlayer)
	{
		int count = 0;
		(*session_) <<
		            "SELECT count(id) FROM Event WHERE playerID=?",
		            into(count), PKW::bind(maxEventCount.first), now;
		count = std::max(0, int(count) - int(maxEventCount.second));
		(*session_) <<
		            "DELETE FROM Event WHERE playerID=? ORDER BY id LIMIT ?",
		            PKW::bind(maxEventCount.first), PKW::bind(count), now;
	}

	for(size_t type = 0; type < ::Event::Count; ++type)
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
	            PKW::bind(time(0) - (3600 * 1)),
	            now;

	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


//! Tuple pour stoker les donnée d'un Event quand il sort de la base de donnée
typedef Poco::Tuple <::Event::ID, time_t, size_t, std::string, intptr_t,
        intptr_t, int, Player::ID, Fleet::ID, Coord::Value, Coord::Value,
        Coord::Value >
        DBEvent;


//! Convertie un DBEvent en Event
::Event toEvent(DBEvent const& ev)
{
	::Event res(ev.get<7>(), ev.get<1>(), ::Event::Type(ev.get<2>()));
	res.id = ev.get<0>();
	res.time = ev.get<1>();
	res.type = ::Event::Type(ev.get<2>());
	res.comment = ev.get<3>();
	res.value = ev.get<4>();
	res.value2 = ev.get<5>();
	res.viewed = ev.get<6>() != 0;
	res.playerID = ev.get<7>();
	res.fleetID = ev.get<8>();
	res.planetCoord = Coord(ev.get<9>(), ev.get<10>(), ev.get<11>());
	return res;
}


std::vector<::Event> DataBase::getPlayerEvents(Player::ID pid) const
try
{
	checkConnection(session_);
	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE playerID = ? "
	            " AND type IN (?, ?, ?, ?, ?, ?, ?) "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents),
	            use(pid),
	            PKW::bind((int)::Event::FleetCodeError),
	            PKW::bind((int)::Event::FleetCodeExecError),
	            PKW::bind((int)::Event::PlanetCodeError),
	            PKW::bind((int)::Event::PlanetCodeExecError),
	            PKW::bind((int)::Event::FleetLose),
	            PKW::bind((int)::Event::PlanetLose),
	            PKW::bind((int)::Event::PlayerLogGather),
	            now;

	std::vector<::Event> out;
	out.reserve(dbEvents.size());
	boost::transform(dbEvents, back_inserter(out), toEvent);
	return out;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


std::vector<::Event> DataBase::getPlanetEvents(Player::ID pid,
    Coord pcoord) const
try
{
	checkConnection(session_);

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

	return dbEvents | transformed(toEvent) | collected<std::vector<::Event>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};

std::vector<::Event> DataBase::getFleetEvents(
  Player::ID pid, Fleet::ID fid) const
try
{
	checkConnection(session_);
	std::vector<DBEvent> dbEvents;
	(*session_) <<
	            "SELECT * FROM Event "
	            "WHERE playerID = ? AND fleetID = ? "
	            "ORDER BY id DESC LIMIT 100 ",
	            into(dbEvents), use(pid), use(fid),
	            now;

	return dbEvents | transformed(toEvent) | collected<std::vector<::Event>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	            PKW::bind(time(0)), use(data), now;
	size_t id = 0;
	(*session_) << "SELECT LAST_INSERT_ID() ", into(id), now;
	trans.commit();
	return id;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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

	for(FightReport const& report : reports)
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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	ss.rdbuf()->sputn(reinterpret_cast<const char*>(data.rawContent()), data.size());
	boost::archive::text_iarchive ia(ss);
	ia& report;
	return report;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	                 AbstractBinder* pBinder,
	                 AbstractBinder::Direction dir)
	{
		poco_assert_dbg(pBinder != 0);
		TypeHandler<std::string>::bind(pos++, obj.message, pBinder, dir);
		TypeHandler<size_t>::bind(pos++, obj.codeDataId, pBinder, dir);
	}

	static std::size_t size()
	{
		return 2;
	}

	static void prepare(std::size_t pos,
	                    const DataBase::CodeError& obj,
	                    AbstractPreparator::Ptr pPrepare)
	{
		TypeHandler<std::string>::prepare(pos++, obj.message, pPrepare);
		TypeHandler<size_t>::prepare(pos++, obj.codeDataId, pPrepare);
	}

	static void extract(std::size_t pos,
	                    DataBase::CodeError& obj,
	                    const DataBase::CodeError& defVal,
	                    AbstractExtractor::Ptr pExt)
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
	            PKW::bind(errors),
	            now;
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


CodeData DataBase::getPlayerCode(Player::ID pid, CodeData::Target target) const
try
{
	checkConnection(session_);
	typedef Poco::Tuple < size_t, Player::ID, time_t, size_t,
	        Data::BLOB, Data::BLOB > ScriptTuple;
	ScriptTuple scrData;
	(*session_) <<
	            "SELECT * FROM Script "
	            "WHERE playerID = ? AND target = ? "
	            "ORDER BY id DESC "
	            "LIMIT 1 ",
	            use(pid), PKW::bind((int)target), into(scrData), now;

	typedef Poco::Tuple<size_t, Player::ID, time_t, size_t, Data::BLOB> BlocklyTuple;
	BlocklyTuple bloData;
	(*session_) <<
	            "SELECT * FROM BlocklyCode "
	            "WHERE playerID = ? AND target = ? "
	            "ORDER BY id DESC "
	            "LIMIT 1 ",
	            use(pid), PKW::bind((int)target), into(bloData), now;

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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	            useRef(login), useRef(password), use(pid),
	            now;
	addScript(pid, CodeData::Planet, "");
	addScript(pid, CodeData::Fleet, "");
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	(*session_) << ss.str(), useRef(tutoName), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::incrementTutoDisplayed(Player::ID pid,
                                      std::string const& tutoName,
                                      int32_t value)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	(*session_) <<
	            "INSERT IGNORE INTO TutoDisplayed "
	            "(playerID, tag, level) "
	            "VALUES(?, ?, ?)", use(pid), useRef(tutoName), PKW::bind(0), now;
	(*session_) << "UPDATE TutoDisplayed "
	            "SET level = level + ? "
	            "WHERE playerID = ? AND tag = ? ",
	            use(value), use(pid), useRef(tutoName), now;
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


PlayerTutoMap DataBase::getTutoDisplayed(Player::ID pid) const
try
{
	checkConnection(session_);
	typedef Poco::Tuple<std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) <<
	            "SELECT tag, level FROM TutoDisplayed "
	            "WHERE playerID = ? ",
	            use(pid), into(tutos), now;

	auto get_pair = [](TutoTuple const & t) {return make_pair(t.get<0>(), t.get<1>()); };
	return tutos | transformed(get_pair) | collected<PlayerTutoMap>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


std::map<Player::ID, PlayerTutoMap>
DataBase::getAllTutoDisplayed() const
try
{
	checkConnection(session_);
	typedef Poco::Tuple<Player::ID, std::string, size_t> TutoTuple;
	std::vector<TutoTuple> tutos;
	(*session_) << "SELECT * FROM TutoDisplayed", into(tutos), now;

	std::map<Player::ID, PlayerTutoMap> result;
	for(TutoTuple const& tuto : tutos)
		result[tuto.get<0>()].insert(make_pair(tuto.get<1>(), tuto.get<2>()));
	return result;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::updateScore(std::map<Player::ID, uint64_t> const& scoreMap)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	typedef Poco::Tuple<uint64_t, Player::ID> ScoreTuple;

	auto pair_to_poco = [](std::map<Player::ID, uint64_t>::value_type const & p)
	{
		return ScoreTuple(get<1>(p), get<0>(p));
	};

	(*session_) << "UPDATE Player SET score = ? WHERE id = ?",
	            PKW::bind(scoreMap | transformed(pair_to_poco) | cached),
	            now;
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::updateXP(std::map<Player::ID, uint32_t> const& expMap)
try
{
	if(expMap.empty())
		return;
	checkConnection(session_);
	Transaction trans(*session_);
	for(auto& nvp : expMap)
		(*session_) <<
		            "UPDATE Player SET "
		            "experience = experience + ?, "
		            "skillpoints = skillpoints + ? "
		            "WHERE id = ?",
		            PKW::bind(nvp.second),
		            PKW::bind(nvp.second),
		            PKW::bind(nvp.first),
		            now;
	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};

std::string skillTabToString(Player::SkillTab const& skillTab)
{
	std::stringstream ss;
	bool first = true;
	for(uint16_t val : skillTab)
	{
		if(not first)
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
	            PKW::bind(cost),
	            useRef(skilltabstr),
	            use(pid),
	            now;
	trans.commit();
	return true;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	            PKW::bind(skilltabstr),
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
	            use(sender), use(recip), PKW::bind(time(0)), useRef(obj), useRef(mes), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


std::vector<Message> DataBase::getMessages(Player::ID recipient)
try
{
	checkConnection(session_);
	Transaction trans(*session_);
	typedef Tuple < Message::ID, Player::ID, Player::ID, time_t, std::string,
	        Data::BLOB, int, std::string > MessageTup;
	std::vector<MessageTup> messages;
	messages.reserve(100);
	(*session_) <<
	            "SELECT Message.*, Player.login FROM Message "
	            "INNER JOIN Player "
	            "ON sender = Player.id "
	            "WHERE recipient = ? ",
	            into(messages), use(recipient), now;

	(*session_) <<
	            "UPDATE Message SET viewed = 1 "
	            "WHERE recipient = ? ",
	            use(recipient), now;

	trans.commit();

	auto tuple_to_message = [](MessageTup const & m)
	{
		return Message(m.get<0>(), m.get<1>(), m.get<2>(), m.get<3>(),
		               m.get<4>(), toString(m.get<5>()), m.get<7>());
	};

	return messages | transformed(tuple_to_message) | collected<std::vector<Message>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::eraseMesage(Message::ID mid)
try
{
	checkConnection(session_);
	(*session_) << "DELETE FROM Message WHERE id = ?", use(mid), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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

	return friends
	       | filtered([&](PlayerTmp const & fr)->bool {return fr.get<0>() != player; })
	       | transformed(playerFromTuple)
	       | collected<std::vector<Player>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
		            "JOIN FriendshipRequest ON"
		            "  FriendshipRequest.sender = Player.id AND"
		            "  FriendshipRequest.recipient = ? ",
		            into(received), use(player), now;

		result.received = received | transformed(playerFromTuple) | collected<std::vector<Player>>();
	}
	{
		std::vector<PlayerTmp> sent;
		sent.reserve(100);
		(*session_) <<
		            boost::format(GetPlayerRequest) %
		            "JOIN FriendshipRequest ON"
		            "  FriendshipRequest.sender = ? AND"
		            "  FriendshipRequest.recipient = Player.id ",
		            into(sent), use(player), now;

		result.sent = sent | transformed(playerFromTuple) | collected<std::vector<Player>>();
	}
	return result;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
	            use(pid), useRef(name), useRef(description), now;
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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};

typedef Tuple <Alliance::ID, Player::ID, std::string, Data::BLOB, std::string> AllianceTup;

Alliance toAllicance(AllianceTup const& a)
{
	return Alliance(a.get<0>(), a.get<1>(), a.get<2>(), toString(a.get<3>()), a.get<4>());
};

Alliance DataBase::getAlliance(Alliance::ID aid) const
try
{
	checkConnection(session_);
	AllianceTup allianceTup;
	(*session_) <<
	            "SELECT Alliance.*, Player.login FROM Alliance "
	            "INNER JOIN Player "
	            "ON masterID = Player.id "
	            "WHERE Alliance.id = ? ",
	            into(allianceTup),
	            use(aid), now;
	return toAllicance(allianceTup);
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};

std::vector<Alliance> DataBase::getAlliances() const
try
{
	checkConnection(session_);
	std::vector<AllianceTup> allianceVect;
	(*session_) <<
	            "SELECT Alliance.*, Player.login FROM Alliance "
	            "INNER JOIN Player "
	            "ON masterID = Player.id ",
	            into(allianceVect),
	            now;

	return allianceVect | transformed(toAllicance) | collected<std::vector<Alliance>>();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::updateAlliance(Alliance const& al)
try
{
	checkConnection(session_);
	(*session_) <<
	            "UPDATE Alliance SET "
	            "  name = ?, description = ? "
	            "WHERE id = ?",
	            useRef(al.name), useRef(al.description), PKW::bind(al.id), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::eraseAlliance(Alliance::ID aid)
try
{
	checkConnection(session_);
	(*session_) << "DELETE FROM Alliance WHERE id = ? ", use(aid), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::joinAlliance(Player::ID pid, Alliance::ID aid)
try
{
	checkConnection(session_);
	(*session_) <<
	            "UPDATE Player SET allianceID = ? WHERE id = ?",
	            use(aid), use(pid), now;
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


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
catch(Poco::Data::DataException const& ex) {DB_CATCH;};


void DataBase::clear(bool keepPlayer)
try
{
	checkConnection(session_);
	Transaction trans(*session_);

	if(keepPlayer == false)
		(*session_) << "SET FOREIGN_KEY_CHECKS = 0", now;

	if(keepPlayer == false)
		(*session_) << "TRUNCATE Player", now;
	(*session_) << "TRUNCATE Event", now;
	(*session_) << "TRUNCATE Script", now;
	(*session_) << "TRUNCATE BlocklyCode", now;
	(*session_) << "TRUNCATE FightReport", now;
	(*session_) << "TRUNCATE TutoDisplayed", now;
	if(keepPlayer == false)
		(*session_) << "TRUNCATE Message", now;
	if(keepPlayer == false)
		(*session_) << "TRUNCATE FriendshipRequest", now;
	if(keepPlayer == false)
		(*session_) << "TRUNCATE Friendship", now;
	if(keepPlayer == false)
		(*session_) << "TRUNCATE Alliance", now;

	(*session_) << "DELETE FROM Player WHERE isAI=True", now;
	(*session_) <<
	            "UPDATE Player SET score = 0, "
	            "  planetCoordX = -1, planetCoordY = -1, planetCoordZ = -1, "
	            "  experience = 0, skillpoints = 0, skilltab = \"\"", now;

	if(keepPlayer == false)
		(*session_) << "SET FOREIGN_KEY_CHECKS = 1", now;

	trans.commit();
}
catch(Poco::Data::DataException const& ex) {DB_CATCH;};

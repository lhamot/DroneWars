#ifndef _BTA_MODEL_
#define _BTA_MODEL_

//#include <boost/multi_array.hpp>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <queue>
#include <iosfwd>

#pragma warning(push)
#pragma warning(disable: 4180 4100)
#include <boost/array.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#pragma warning(pop)

#include <unordered_map>
#include "serialize_unordered_map.h"


//#define CHECK(test) if((test) == false) BOOST_THROW_EXCEPTION(std::logic_error(#test + " test failed"));


struct Event
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& id& time& type& comment& value;
	}

	enum Type
	{
		FleetCodeError,
		FleetCodeExecError,
		PlanetCodeError,
		PlanetCodeExecError,
		Upgraded,
		ShipMade,
		PlanetHarvested,
		FleetWin,
		FleetDraw,
		FleetsGather,
		PlanetColonized,
		FleetLose,
		FleetDrop,
		PlanetLose,
		PlanetWin,
		CannonMade,
		Count
	};

	typedef size_t ID;
	static ID const NoId = ID(-1);
	ID id;
	time_t time;
	Type type;
	std::string comment;
	size_t value;
	bool viewed;

	Event() {}
	Event(ID ident, time_t ti, Type ty, size_t val = size_t(-1)):
		id(ident), time(ti), type(ty), value(val), viewed(false)
	{
	}
	Event(ID ident, time_t ti, Type ty, std::string const& comm):
		id(ident), time(ti), type(ty), comment(comm), value(size_t(-1)), viewed(false)
	{
	}
};

static size_t const MaxStringSize = 256;

struct CodeData
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& code_& failCount_& lastError_;
	}

	std::string code_;
	size_t failCount_;
	std::string lastError_;

public:
	CodeData(): failCount_(0) {}

	std::string const& getCode() const {return code_;}

	size_t getFailCount() const {return failCount_;}

	std::string const& getLastError() const {return lastError_;}

	void setCode(std::string const& newCode)
	{
		code_ = newCode;
		failCount_ = 0;
		lastError_.clear();
	}

	void newError(std::string newError)//Pour profiter du NRVO quand newError est temporaire
	{
		lastError_.swap(newError);
		++failCount_;
	}
};

struct Player
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& id& login& password& fleetsCode& planetsCode& eventList;
	}

public:
	Player() {} //pour boost::serialization

	typedef size_t ID;
	static ID const NoId = ID(-1);
	ID id;
	std::string login;
	std::string password;
	CodeData fleetsCode;
	CodeData planetsCode;
	std::vector<Event> eventList;
	static size_t const MaxCodeSize = 16 * 1024;

	Player(ID i, std::string const& lg, std::string const& pass): id(i), login(lg), password(pass) {}
};


struct Coord
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& X& Y& Z;
	}

	typedef long Value;
	Value X;
	Value Y;
	Value Z;

	Coord(): X(0), Y(0), Z(0) {}

	Coord(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};

inline bool operator == (Coord const& a, Coord const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}

namespace std
{
template<>
struct hash<Coord>
{
	size_t operator()(const Coord& p) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.X);
		boost::hash_combine(seed, p.Y);
		boost::hash_combine(seed, p.Z);
		return seed;
	}
};
}

struct CompCoord
{
	bool operator()(Coord a, Coord b) const
	{
		if(a.X < b.X)
			return true;
		else if(a.X > b.X)
			return false;
		else if(a.Y < b.Y)
			return true;
		else if(a.Y > b.Y)
			return false;
		else
			return a.Z < b.Z;
	};
};

struct Ressource
{
	enum Enum
	{
		Metal,
		Carbon,
		Loicium,
		Count
	};
};

struct RessourceSet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& tab;
	}

	typedef boost::array<size_t, Ressource::Count> Tab;
	Tab tab;

	RessourceSet(Tab const& t): tab(t) {}
	RessourceSet() {tab.fill(0);}
	RessourceSet(size_t a, size_t b, size_t c) {tab[0] = a; tab[1] = b; tab[2] = c;}

	inline bool operator == (RessourceSet const& b) const
	{
		return boost::range::equal(tab, b.tab);
	}
	inline bool operator != (RessourceSet const& b) const
	{
		return !(*this == b);
	}
};

struct PlanetTask
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& value& value2& lauchTime& duration& startCost& expired;
	}

	enum Enum
	{
		UpgradeBuilding,
		MakeShip,
		MakeCannon,
		Count
	};

	Enum type;
	size_t value;
	size_t value2;
	time_t lauchTime;
	size_t duration;
	RessourceSet startCost;
	bool expired;

	PlanetTask() {}
	PlanetTask(Enum t, time_t lauch, size_t dur):
		type(t), value(0), value2(0), lauchTime(lauch), duration(dur), expired(false)
	{
	}
};

struct FleetTask
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& lauchTime& duration& position& expired;
	}

	enum Enum
	{
		Move,
		Harvest,
		Colonize,
		Count
	};

	Enum type;
	time_t lauchTime;
	size_t duration;
	Coord position;
	bool expired;

	FleetTask() {}
	FleetTask(Enum t, time_t lauch, size_t dur):
		type(t), lauchTime(lauch), duration(dur), expired(false)
	{
	}
};


struct Building
{
	enum Enum
	{
		Undefined = -1,
		CommandCenter,
		MetalMine,
		CarbonMine,
		LoiciumFilter,
		Factory,
		Laboratory,
		CarbonicCentral,
		SolarCentral,
		GeothermicCentral,
		Count
	};

	RessourceSet price;
	double coef;

	static Building const List[];
};


struct Ship
{
	enum Enum
	{
		Undefined = -1,
		Mosquito,
		Hornet,
		Vulture,
		Dragon,
		Behemoth,
		Azathoth,
		Queen,
		Cargo,
		LargeCargo,
		Count
	};

	RessourceSet price;
	size_t life;
	size_t power;


	static Ship const List[];
};


struct Cannon
{
	enum Enum
	{
		Undefined = -1,
		Cannon1,
		Cannon2,
		Cannon3,
		Cannon4,
		Cannon5,
		Cannon6,
		Count
	};

	RessourceSet price;
	size_t life;
	size_t power;

	static Cannon const List[];
};


struct Planet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		if(Archive::is_saving::value)
		{
			if(playerId >= 100 && playerId != Player::NoId)
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
			if(playerId == Player::NoId && taskQueue.empty() == false)
				BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
		}

		ar& coord& playerId& buildingMap& taskQueue& ressourceSet& eventList& cannonTab;
		if(playerId >= 100 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
		if(playerId == Player::NoId && taskQueue.empty() == false)
			BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
	}

	Coord coord;
	Player::ID playerId;
	typedef std::map<Building::Enum, size_t> BuildingMap;
	BuildingMap buildingMap;
	std::vector<PlanetTask> taskQueue;
	RessourceSet ressourceSet;
	typedef boost::array<size_t, Cannon::Count> CannonTab;
	std::vector<Event> eventList;
	CannonTab cannonTab;

	Planet() {}
	Planet(Coord c): coord(c), playerId(Player::NoId)
	{
		cannonTab.fill(0);
		if(playerId >= 100 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
	}

	bool isFree() const
	{
		return playerId == Player::NoId;
	}
};


struct PlanetAction
{
	/*template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& action& building& ship;
	}*/

	enum Type
	{
		Undefined = -1,
		Building,
		StopBuilding,
		Ship,
		Cannon,
		Count
	};

	Type action;
	Building::Enum building;
	Ship::Enum ship;
	Cannon::Enum cannon;
	size_t number;

	bool operator==(PlanetAction const& other)
	{
		return action == other.action &&
		       building == other.building &&
		       ship == other.ship &&
		       number == other.number;
	}

	PlanetAction(): action(Undefined), building(Building::Undefined), ship(Ship::Undefined), cannon(Cannon::Undefined), number(0) {}
	PlanetAction(Type a, Building::Enum b): action(a), building(b), ship(Ship::Undefined), cannon(Cannon::Undefined), number(0) {}
	PlanetAction(Type a, Ship::Enum s, size_t n): action(a), building(Building::Undefined), ship(s), cannon(Cannon::Undefined), number(n) {}
	PlanetAction(Type a, Cannon::Enum c, size_t n): action(a), building(Building::Undefined), ship(Ship::Undefined), cannon(c), number(n) {}
};
typedef std::vector<PlanetAction> PlanetActionList;


struct Fleet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		if(Archive::is_saving::value)
		{
			if(playerId >= 100)
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
		}
		ar& id& playerId& coord& origine& name& shipList& ressourceSet& taskQueue& eventList;
		if(playerId >= 100)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
	}

	typedef size_t ID;
	ID id;
	Player::ID playerId;
	Coord coord;
	Coord origine;
	std::string name;
	typedef std::vector<size_t> ShipTab;
	ShipTab shipList;
	RessourceSet ressourceSet;
	std::vector<FleetTask> taskQueue;
	std::vector<Event> eventList;

	Fleet() {}
	Fleet(ID fid, Player::ID pid, Coord c):
		id(fid), playerId(pid), coord(c), origine(c), shipList(Ship::Count)
	{
		if(playerId >= 100)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100!!"));
	}
};


struct FleetAction
{
	/*	template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar& action& building& ship;
		}*/

	enum Type
	{
		Nothing,
		Move,
		Harvest,
		Colonize,
		Drop
	};

	Type action;
	Coord target;

	bool operator==(FleetAction const& other)
	{
		return action == other.action && target == other.target;
	}

	FleetAction(Type a, Coord t = Coord()): action(a), target(t) {}
};
typedef std::vector<FleetAction> FleetActionList;


template<typename T>
struct Report
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& isDead& hasFight& enemySet& fightInfo;
	}

	bool isDead;
	bool hasFight;
	std::set<size_t> enemySet; //par index dans le FightReport
	struct FightInfo
	{
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar& before& after;
		}

		T before;
		T after;

		FightInfo() {}
		FightInfo(T const& bef, T const& aft): before(bef), after(aft) {}
	};
	FightInfo fightInfo;

	Report() {}
	Report(T const& fighter): isDead(false), hasFight(false)
	{
		FightInfo info(fighter, fighter);
		info.before.eventList.clear();
		info.after.eventList.clear();
		fightInfo = info;
	}
};


struct FightReport
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& fleetList& hasPlanet& planet;
	}

	std::vector<Report<Fleet> > fleetList;
	bool hasPlanet;
	Report<Planet> planet;

	FightReport(): hasPlanet(false) {}
};


struct Universe
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& playerMap;
		ar& planetMap;
		ar& fleetMap;
		ar& reportMap;
		ar& nextPlayerID;
		ar& nextFleetID;
		ar& nextEventID;
		ar& nextFightID;
		ar& time;
	}

	//typedef boost::multi_array<Zone, 3> ZoneMap;
	//ZoneMap zoneMap;
	static const unsigned short MapSizeX = 100;
	static const unsigned short MapSizeY = 100;
	static const unsigned short MapSizeZ = 100;

	typedef std::map<Player::ID, Player> PlayerMap;
	PlayerMap playerMap;
	typedef std::unordered_map<Coord, Planet> PlanetMap;
	PlanetMap planetMap;
	//std::multimap<Coord, Fleet, CompCoord> fleetMap;
	typedef std::map<Fleet::ID, Fleet> FleetMap;
	FleetMap fleetMap;
	typedef std::map<size_t, FightReport> ReportMap;
	ReportMap reportMap;
	Player::ID nextPlayerID;
	Fleet::ID nextFleetID;
	Event::ID nextEventID;
	size_t nextFightID;
	time_t time;

	typedef boost::shared_mutex Mutex;
	mutable Mutex mutex;

	Universe(): nextPlayerID(0), nextFleetID(0), nextEventID(0), nextFightID(0), time(0)
	{
	}
};

void construct(Universe& univ);

Player::ID createPlayer(Universe& univ, std::string const& login, std::string const& password);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream(std::istream& in, Universe& univ);

std::string getBuildingName(Building::Enum type);

bool canBuild(Planet const& planet, Ship::Enum type, size_t number);

bool canBuild(Planet const& planet, Building::Enum type);

void addTask(Planet& planet, time_t time, Building::Enum building);

void addTask(Planet& planet, time_t time, Ship::Enum ship, size_t number);

bool canBuild(Planet const& planet, Cannon::Enum type, size_t number);

void addTask(Planet& planet, time_t time, Cannon::Enum cannon, size_t number);

bool canStop(Planet const& planet, Building::Enum type);

void stopTask(Planet& planet, PlanetTask::Enum tasktype, Building::Enum building);

void planetRound(Universe& univ, Planet& planet, time_t time);

void fleetRound(Universe& univ, Fleet& fleet, time_t time);

void gather(Fleet& fleet, Fleet const& otherFleet);

bool canMove(Fleet const& fleet, Coord const& coord);

void addTask(Fleet& fleet, time_t time, Coord const& coord);

bool canHarvest(Fleet const& fleet, Planet const& planet);

void addTaskHarvest(Fleet& fleet, time_t time, Planet const& planet);

bool canColonize(Fleet const& fleet, Planet const& planet);

void addTaskColonize(Fleet& fleet, time_t time, Planet const& planet);

bool canDrop(Fleet const& fleet, Planet const& planet);

void drop(Fleet& fleet, Planet& planet);

#endif //_BTA_MODEL_
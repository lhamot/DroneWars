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
#include <boost/serialization/array.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/range/algorithm.hpp>
#pragma warning(pop)


struct Event
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& time& type& comment;
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
		Count
	};

	time_t time;
	Type type;
	std::string comment;

	Event() {}
	Event(time_t ti, Type ty, std::string const& comm):
		time(ti), type(ty), comment(comm)
	{
	}
};

struct Player
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& id& login& fleetsCode& planetsCode& eventList& eventList;
	}

public:
	Player() {} //pour boost::serialization

	typedef size_t ID;
	static ID const NoId = ID(-1);
	ID id;
	std::string login;
	std::string fleetsCode;
	std::string planetsCode;
	std::vector<Event> eventList;

	Player(ID i, std::string const& lg): id(i), login(lg) {}
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
		Apocalyps,
		Count
	};

	RessourceSet price;

	static Ship const List[];
};

struct Planet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& coord& playerId& buildingMap& taskQueue& ressourceSet;
	}

	/*struct CmpBuild
	{
		bool operator()(Building a, Building b) const
		{
			return a.type < b.type;
		}
	};*/

	Coord coord;
	Player::ID playerId;
	typedef std::map<Building::Enum, size_t> BuildingMap;
	BuildingMap buildingMap;
	std::vector<PlanetTask> taskQueue;
	RessourceSet ressourceSet;
	std::vector<Event> eventList;

	Planet() {}
	Planet(Coord c): coord(c), playerId(Player::NoId) {}

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
		Ship
	};

	Type action;
	Building::Enum building;
	Ship::Enum ship;
	size_t number;

	bool operator==(PlanetAction const& other)
	{
		return action == other.action &&
		       building == other.building &&
		       ship == other.ship &&
		       number == other.number;
	}

	PlanetAction(): action(Undefined), building(Building::Undefined), ship(Ship::Undefined), number(0) {}
	PlanetAction(Type a, Building::Enum b): action(a), building(b), ship(Ship::Undefined), number(0) {}
	PlanetAction(Type a, Ship::Enum s, size_t n): action(a), building(Building::Undefined), ship(s), number(n) {}
};
typedef std::vector<PlanetAction> PlanetActionList;


struct Fleet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& id& playerId& coord& name& shipList;
	}

	typedef size_t ID;
	ID id;
	Player::ID playerId;
	Coord coord;
	std::string name;
	typedef std::vector<size_t> ShipTab;
	ShipTab shipList;
	RessourceSet ressourceSet;
	std::vector<FleetTask> taskQueue;
	std::vector<Event> eventList;

	Fleet() {}
	Fleet(ID fid, Player::ID pid, Coord c):
		id(fid), playerId(pid), coord(c), shipList(Ship::Count)
	{
		//shipList.fill(0);
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
		Colonize
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

struct Universe
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& playerMap;
		ar& planetMap;
		ar& fleetMap;
		ar& nextPlayerID;
		ar& time;
	}

	//typedef boost::multi_array<Zone, 3> ZoneMap;
	//ZoneMap zoneMap;
	static const size_t MapSizeX = 100;
	static const size_t MapSizeY = 100;
	static const size_t MapSizeZ = 100;

	typedef std::map<Player::ID, Player> PlayerMap;
	PlayerMap playerMap;
	typedef std::map<Coord, Planet, CompCoord> PlanetMap;
	PlanetMap planetMap;
	//std::multimap<Coord, Fleet, CompCoord> fleetMap;
	std::map<Fleet::ID, Fleet> fleetMap;
	Player::ID nextPlayerID;
	Fleet::ID nextFleetID;
	time_t time;

	Universe(): nextPlayerID(0), time(0)
	{
	}
};

void construct(Universe& univ);

Player::ID createPlayer(Universe& univ, std::string const& login);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream(std::istream& in, Universe& univ);

std::string getBuildingName(Building::Enum type);

bool canBuild(Planet const& planet, Ship::Enum type, size_t number);

bool canBuild(Planet const& planet, Building::Enum type);

void addTask(Planet& planet, time_t time, Building::Enum building);

void addTask(Planet& planet, time_t time, Ship::Enum ship, size_t number);

bool canStop(Planet const& planet, Building::Enum type);

void stopTask(Planet& planet, PlanetTask::Enum tasktype, Building::Enum building);

void planetRound(Universe& univ, Planet& planet, time_t time);

void fleetRound(Universe& univ, Fleet& fleet, time_t time);

void gather(Fleet& fleet, Fleet const& otherFleet);

boost::logic::tribool fight(Fleet& fleet1, Fleet& fleet2);

bool canMove(Fleet const& fleet, Coord const& coord);

void addTask(Fleet& fleet, time_t time, Coord const& coord);

bool canHarvest(Fleet const& fleet, Planet const& planet);

void addTaskHarvest(Fleet& fleet, time_t time, Planet const& planet);

#endif //_BTA_MODEL_
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

#include <boost/array.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/array.hpp>

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
		PlanetCodeExecError
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

	RessourceSet(Tab const& t):tab(t){}
	RessourceSet(){tab.fill(0);}
	RessourceSet(size_t a, size_t b, size_t c){tab[0] = a; tab[1] = b; tab[2] = c;}
};


struct Task
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& value& value2& lauchTime& duration& startCost;
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

	Task() {}
	Task(Enum t, time_t lauch, size_t dur):
		type(t), value(0), value2(0), lauchTime(lauch), duration(dur), expired(false)
	{
	}
};


struct Building
{
	enum Enum
	{
		None = -1,
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
		None = -1,
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
	std::vector<Task> taskQueue;
	RessourceSet ressourceSet;

	Planet() {}
	Planet(Coord c): coord(c), playerId(Player::NoId) {}
};


struct PlanetAction
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& action& building& ship;
	}

	enum Type
	{
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
		return action == other.action && building == other.building;
	}

	PlanetAction(Type a, Building::Enum b): action(a), building(b), ship(Ship::None), number(0) {}
	PlanetAction(Type a, Ship::Enum s, size_t n): action(a), building(Building::None), ship(s), number(n) {}
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
	boost::array<size_t, Ship::Count> shipList;
	RessourceSet ressourceSet;

	Fleet() {}
	Fleet(ID fid, Player::ID pid, Coord c): 
		id(fid), playerId(pid), coord(c) 
	{
		shipList.fill(0);
	}
};



struct Universe
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& playerMap;
		ar& planetMap;
		ar& fleetList;
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
	std::vector<Fleet> fleetList;
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

void stopTask(Planet& planet, Task::Enum tasktype, Building::Enum building);

void planetRound(Universe& univ, Planet& planet);

#endif //_BTA_MODEL_
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

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

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

	Event(){}
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
	Player(){} //pour boost::serialization

	typedef size_t ID;
	static ID const NoId = ID(-1);
	ID id;
	std::string login;
	std::string fleetsCode;
	std::string planetsCode;
	std::vector<Event> eventList;

	Player(ID i, std::string const& lg): id(i), login(lg) {}
};

struct Task
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& value& lauchTime& duration;
	}

	enum Type
	{
		UpgradeBuilding
	};

	Type type;
	size_t value;
	time_t lauchTime;
	size_t duration;

	Task(){}
	Task(Type t, size_t val, time_t lauch, size_t dur):
		type(t),value(val),lauchTime(lauch),duration(dur)
	{
	}
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

	Coord():X(0),Y(0),Z(0){}

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

//struct Zone{};

struct Building
{
	enum Type
	{
		CommandCenter,
		MetalMine,
		CarbonMine,
		CristalMine,
		Factory,
		Laboratory,
		CarbonicCentral,
		SolarCentral,
		GeothermicCentral,
	};

	//Type type;
	//size_t level;

	//Building(Type t):type(t),level(0){}
};


struct Planet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& coord& playerId& buildingMap& taskQueue;
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
	typedef std::map<Building::Type, size_t> BuildingMap;
	BuildingMap buildingMap;
	std::vector<Task> taskQueue;

	Planet(){}
	Planet(Coord c): coord(c), playerId(Player::NoId) {}
};


struct PlanetAction
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& action& building;
	}

	enum Type
	{
		Building,
		StopBuilding
	};

	Type action;
	Building::Type building;

	bool operator==(PlanetAction const& other)
	{
		return action == other.action && building == other.building;
	}

	PlanetAction(Type a, Building::Type b): action(a), building(b) {}
};
typedef std::vector<PlanetAction> PlanetActionList;

struct Ship
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type;
	}


	enum Type
	{
		Factory,
		Laboratory,
		MetalMine,
		CarbonMine,
		CristalMine,
		CarbonicCentral,
		SolarCentral,
		GeothermicCentral,
		CommandCenter
	};

	//Fleet::ID const fleetId;
	Type type;

	Ship(){}
	Ship(
	  //Fleet::ID fid,
	  Type t):
		//fleetId(fid),
		type(t) {}
};


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
	std::vector<Ship> shipList;

	Fleet(){}
	Fleet(ID fid, Player::ID pid, Coord c): id(fid), playerId(pid), coord(c) {}
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
	time_t time;

	Universe(): nextPlayerID(0),time(0)
	{
	}
};

void construct(Universe& univ);

Player::ID createPlayer(Universe& univ, std::string const& login);

void saveToStream(Universe const& univ, std::ostream &out);
void loadFromStream(std::istream &in, Universe& univ);

std::string getBuildingName(Building::Type type);

std::string getTaskName(Task::Type type);


#endif //_BTA_MODEL_
#ifndef _BTA_MODEL_
#define _BTA_MODEL_

#include "stdafx.h"

#include "serialize_unordered_map.h"
#include "Player.h"
#include "Tools.h"


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
	typedef uint32_t Value;
	enum Enum : uint8_t
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

	typedef boost::array<Ressource::Value, Ressource::Count> Tab;
	Tab tab;

	RessourceSet(Tab const& t): tab(t) {}
	RessourceSet() {tab.fill(0);}
	RessourceSet(Ressource::Value a, Ressource::Value b, Ressource::Value c)
	{
		tab[0] = a; tab[1] = b; tab[2] = c;
	}

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

	enum Enum : uint8_t
	{
	  UpgradeBuilding,
	  MakeShip,
	  MakeCannon,
	  Count
	};

	uint32_t value;
	uint32_t value2;
	uint32_t lauchTime;
	uint32_t duration;
	RessourceSet startCost;
	bool expired;
	Enum type;

	PlanetTask() {}
	PlanetTask(Enum t, uint32_t lauch, uint32_t dur):
		value(0), value2(0), lauchTime(lauch), duration(dur), expired(false), type(t)
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

	enum Enum : uint8_t
	{
	  Move,
	  Harvest,
	  Colonize,
	  Count
	};

	uint32_t lauchTime;
	uint32_t duration;
	Coord position;
	bool expired;
	Enum type;

	FleetTask() {}
	FleetTask(Enum t, uint32_t lauch, uint32_t dur):
		lauchTime(lauch), duration(dur), expired(false), type(t)
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

	static Building const List[Count];
};


struct Ship
{
	enum Enum : int8_t
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
	uint16_t life;
	uint16_t power;
	uint16_t shield;


	static Ship const List[Count];
};


struct Cannon
{
	enum Enum : int8_t
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
	uint16_t life;
	uint16_t power;
	uint16_t shield;

	static Cannon const List[Count];
};


struct Planet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000 && playerId != Player::NoId)
			{
				using namespace log4cplus;
				Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("Planet.serialize"));
				LOG4CPLUS_ERROR(logger, "playerId : " << playerId);
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
			}
			if(playerId == Player::NoId && taskQueue.empty() == false)
				BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
			if(buildingList.size() != Building::Count)
				BOOST_THROW_EXCEPTION(std::logic_error("buildingList.size() != Building::Count"));
		});

		ar& name;
		ar& coord;
		ar& playerId;
		ar& buildingList;
		ar& taskQueue;
		ar& ressourceSet;
		ar& cannonTab;
		if(playerId >= 100000 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		if(playerId == Player::NoId && taskQueue.empty() == false)
			BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
		if(buildingList.size() != Building::Count)
			BOOST_THROW_EXCEPTION(std::logic_error("buildingList.size() != Building::Count"));
		ar& parentCoord;
	}

	std::string name;
	Coord coord;
	Player::ID playerId;
	typedef boost::array<uint16_t, Building::Count> BuildingTab;
	BuildingTab buildingList;
	std::vector<PlanetTask> taskQueue;
	RessourceSet ressourceSet;
	typedef boost::array<uint32_t, Cannon::Count> CannonTab;
	CannonTab cannonTab;
	Coord parentCoord;

	size_t heap_size() const
	{
		return
		  name.capacity() +
		  buildingList.size() * sizeof(size_t) +
		  taskQueue.capacity() * sizeof(PlanetTask);
	}

	Planet(): playerId(55555) {}
	Planet(Coord c): coord(c), playerId(Player::NoId)
	{
		buildingList.fill(0);
		cannonTab.fill(0);
		if(playerId >= 100000 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}

	bool isFree() const
	{
		return playerId == Player::NoId;
	}
};


struct PlanetAction
{
	enum Type : int8_t
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
	uint32_t number;

	bool operator==(PlanetAction const& other)
	{
		return action == other.action &&
		       building == other.building &&
		       ship == other.ship &&
		       number == other.number;
	}

	PlanetAction(): action(Undefined), building(Building::Undefined), ship(Ship::Undefined), cannon(Cannon::Undefined), number(0) {}
	PlanetAction(Type a, Building::Enum b): action(a), building(b), ship(Ship::Undefined), cannon(Cannon::Undefined), number(0) {}
	PlanetAction(Type a, Ship::Enum s, uint32_t n): action(a), building(Building::Undefined), ship(s), cannon(Cannon::Undefined), number(n) {}
	PlanetAction(Type a, Cannon::Enum c, uint32_t n): action(a), building(Building::Undefined), ship(Ship::Undefined), cannon(c), number(n) {}
};
typedef std::vector<PlanetAction> PlanetActionList;


struct Fleet
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000)
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		});
		if(version < 1)
		{
			size_t val;
			ar& val;
			id = val;
		}
		else
			ar& id;
		ar& playerId;
		ar& coord;
		ar& origin;
		ar& name;
		ar& shipList;
		ar& ressourceSet;
		ar& taskQueue;
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}

	typedef uint64_t ID;
	static const ID NoId = 0;
	ID id;
	Player::ID playerId;
	Coord coord;
	Coord origin;
	std::string name;
	typedef boost::array<uint32_t, Ship::Count> ShipTab;
	ShipTab shipList;
	RessourceSet ressourceSet;
	std::vector<FleetTask> taskQueue;


	size_t heap_size() const
	{
		return name.capacity();
	}


	Fleet() {}
	Fleet(ID fid, Player::ID pid, Coord c):
		id(fid), playerId(pid), coord(c), origin(c)
	{
		shipList.fill(0);
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}
};


struct FleetAction
{
	enum Type : uint8_t
	{
	  Nothing,
	  Move,
	  Harvest,
	  Colonize,
	  Drop
	};

	Type action;
	Direction target;

	bool operator==(FleetAction const& other)
	{
		return action == other.action && target == other.target;
	}

	FleetAction(Type a, Direction t = Direction()): action(a), target(t) {}

	//TODO: A virer quand plus persone ne l'utilisera
	FleetAction(Type a, Coord t): action(a), target(t.X, t.Y, t.Z) {}
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
	std::set<intptr_t> enemySet; //par index dans le FightReport
	struct FightInfo
	{
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar& before& after;
		}

		T before;
		T after;

		size_t heap_size() const
		{
			return before.heap_size() + after.heap_size();
		}

		FightInfo() {}
		FightInfo(T const& bef, T const& aft): before(bef), after(aft) {}
	};
	FightInfo fightInfo;

	static FightInfo makeFightInfo(T fighter)
	{
		return FightInfo(fighter, fighter);
	}

	Report() {}
	Report(T const& fighter): isDead(false), hasFight(false), fightInfo(makeFightInfo(fighter))
	{
	}

	size_t heap_size() const
	{
		return
		  enemySet.size() * sizeof(size_t) * 3 +
		  fightInfo.heap_size();
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
	boost::optional<Report<Planet> > planet;

	FightReport(): hasPlanet(false) {}

	size_t heap_size() const
	{
		size_t res = fleetList.capacity() * sizeof(Report<Fleet>);
		for(Report<Fleet> const & report : fleetList)
			res += report.heap_size();
		if(planet)
			res += planet->heap_size();
		return res;
	}
};


struct Event
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& id& time& type& comment& value;
	}

	enum Type : uint8_t
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
	ID id;
	time_t time;
	Type type;
	std::string comment;
	intptr_t value;
	bool viewed;
	Player::ID playerID;
	Fleet::ID fleetID;
	Coord planetCoord;


	size_t heap_size() const
	{
		return comment.capacity();
	}

	Event() {}

	Event(Player::ID pid, time_t ti, Type ty):
		id(0), time(ti), type(ty), value(-1), viewed(false),
		playerID(pid), fleetID(Fleet::NoId)
	{
	}

	Event& setValue(intptr_t val)       {value = val; return *this;}
	Event& setComment(std::string const& comm)    {comment = comm; return *this;}
	Event& setFleetID(Fleet::ID fid)    {fleetID = fid; return *this;}
	Event& setPlanetCoord(Coord pcoord) {planetCoord = pcoord; return *this;}
};


struct Universe
{
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& planetMap;
		ar& fleetMap;
		ar& nextFleetID;
		ar& roundCount;
		ar& roundDuration;
	}

	static const unsigned short MapSizeX = 100;
	static const unsigned short MapSizeY = 100;
	static const unsigned short MapSizeZ = 100;

	typedef std::unordered_map<Coord, Planet> PlanetMap;
	PlanetMap planetMap;
	typedef std::map<Fleet::ID, Fleet> FleetMap;
	FleetMap fleetMap;
	Fleet::ID nextFleetID;
	uint32_t roundCount;
	double roundDuration;

	size_t heap_size() const
	{
		size_t res = 0;
		for(auto const & planetKV : planetMap)
			res += sizeof(planetKV) +
			       planetKV.second.heap_size() +
			       2 * sizeof(size_t);
		for(auto const & fleetKV : fleetMap)
			res += sizeof(fleetKV) +
			       fleetKV.second.heap_size() +
			       2 * sizeof(size_t);
		return res;
	}

	typedef boost::shared_mutex Mutex;
	mutable Mutex planetsFleetsReportsmutex;

	Universe():
		nextFleetID(1),
		roundCount(0),
		roundDuration(0.)
	{
	}

	Universe(Universe const& other):
		planetMap(other.planetMap),
		fleetMap(other.fleetMap),
		nextFleetID(other.nextFleetID),
		roundCount(other.roundCount),
		roundDuration(other.roundDuration)
	{
	}

	Universe& operator=(Universe other)
	{
		swap(other);
		return *this;
	}

	void swap(Universe& other)
	{
		planetMap.swap(other.planetMap);
		fleetMap.swap(other.fleetMap);
		std::swap(nextFleetID, other.nextFleetID);
		std::swap(roundCount, other.roundCount);
		std::swap(roundDuration, other.roundDuration);
	}
};

class DataBase;

void construct(Universe& univ, DataBase& database);

RessourceSet getBuilingPrice(Building::Enum id, size_t level);

void getNewPlayerCode(std::vector<std::string>& codes);

Coord createPlayer(Universe& univ, Player::ID pid);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream_v1(std::istream& in, Universe& univ);
void loadFromStream_v2(std::istream& in, Universe& univ);

bool canBuild(Planet const& planet, Ship::Enum type, size_t number);

bool canBuild(Planet const& planet, Building::Enum type);

void addTask(Planet& planet, uint32_t roundCount, Building::Enum building);

void addTask(Planet& planet,
             uint32_t roundCount,
             Ship::Enum ship,
             uint32_t number);

bool canBuild(Planet const& planet, Cannon::Enum type, size_t number);

void addTask(Planet& planet,
             uint32_t roundCount,
             Cannon::Enum cannon,
             uint32_t number);

bool canStop(Planet const& planet, Building::Enum type);

void stopTask(Planet& planet,
              PlanetTask::Enum tasktype,
              Building::Enum building);

//! Gere l'écoulement du temps sur la planète.
//! Peut modifier la liste dse flotte et des planètes
void planetRound(Universe& univ,
                 Planet& planet,
                 std::vector<Event>& events);

//! Gere l'écoulement du temps sur la flotte.
//! Peut modifier la liste des flottes et des planètes
void fleetRound(Universe& univ,
                Fleet& fleet,
                std::vector<Event>& events,
                std::map<Player::ID, size_t> const& playersPlanetCount);

void gather(Fleet& fleet, Fleet const& otherFleet);

bool canMove(Fleet const& fleet, Coord const& coord);

void addTask(Fleet& fleet, uint32_t roundCount, Coord const& coord);

bool canHarvest(Fleet const& fleet, Planet const& planet);

void addTaskHarvest(Fleet& fleet, uint32_t roundCount, Planet const& planet);

bool canColonize(Fleet const& fleet, Planet const& planet);

void addTaskColonize(Fleet& fleet, uint32_t roundCount, Planet const& planet);

bool canDrop(Fleet const& fleet, Planet const& planet);

void drop(Fleet& fleet, Planet& planet);

bool canPay(RessourceSet const& stock, RessourceSet const& price);



#endif //_BTA_MODEL_
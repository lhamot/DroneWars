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
	FleetTask(Enum t, size_t lauch, size_t dur):
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
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000 && playerId != Player::NoId)
			{
				std::cout << playerId << std::endl;
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
			}
			if(playerId == Player::NoId && taskQueue.empty() == false)
				BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
			if(buildingList.size() != Building::Count)
				BOOST_THROW_EXCEPTION(std::logic_error("buildingList.size() != Building::Count"));
		});

		ar& name& coord& playerId& buildingList& taskQueue& ressourceSet& eventList& cannonTab;
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
	typedef std::vector<size_t> BuildingTab;
	BuildingTab buildingList;
	std::vector<PlanetTask> taskQueue;
	RessourceSet ressourceSet;
	typedef boost::array<size_t, Cannon::Count> CannonTab;
	std::vector<Event> eventList;
	CannonTab cannonTab;
	Coord parentCoord;

	size_t heap_size() const
	{
		size_t res =
		  name.capacity() +
		  buildingList.capacity() * sizeof(size_t) +
		  taskQueue.capacity() * sizeof(PlanetTask) +
		  eventList.capacity() * sizeof(Event);
		for(Event const & ev: eventList)
			res += ev.heap_size();
		return res;
	}

	Planet(): playerId(1111111111), buildingList(Building::Count) {}
	Planet(Coord c): coord(c), playerId(Player::NoId), buildingList(Building::Count)
	{
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
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000)
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		});
		ar& id& playerId& coord& origin& name& shipList& ressourceSet& taskQueue& eventList;
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}

	typedef size_t ID;
	ID id;
	Player::ID playerId;
	Coord coord;
	Coord origin;
	std::string name;
	typedef std::vector<size_t> ShipTab;
	ShipTab shipList;
	RessourceSet ressourceSet;
	std::vector<FleetTask> taskQueue;
	std::vector<Event> eventList;


	size_t heap_size() const
	{
		size_t res =
		  name.capacity() +
		  shipList.capacity() * sizeof(size_t) +
		  taskQueue.capacity() * sizeof(FleetTask) +
		  eventList.capacity() * sizeof(Event);
		for(Event const & ev: eventList)
			res += ev.heap_size();
		return res;
	}


	Fleet() {}
	Fleet(ID fid, Player::ID pid, Coord c):
		id(fid), playerId(pid), coord(c), origin(c), shipList(Ship::Count)
	{
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
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
		fighter.eventList.clear();
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
		for(Report<Fleet> const & report: fleetList)
			res += report.heap_size();
		if(planet)
			res += planet->heap_size();
		return res;
	}
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
		ar& roundCount;
		ar& roundDuration;
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
	typedef std::map<Fleet::ID, Fleet> FleetMap;
	FleetMap fleetMap;
	typedef std::map<size_t, FightReport> ReportMap;
	ReportMap reportMap;
	Player::ID nextPlayerID;
	Fleet::ID nextFleetID;
	Event::ID nextEventID;
	size_t nextFightID;
	size_t roundCount;
	double roundDuration;

	size_t heap_size() const
	{
		size_t res = 0;
		for(auto const & playerKV: playerMap)
			res += sizeof(playerKV) + playerKV.second.heap_size() + 2 * sizeof(size_t);
		for(auto const & planetKV: planetMap)
			res += sizeof(planetKV) + planetKV.second.heap_size() + 2 * sizeof(size_t);
		for(auto const & fleetKV: fleetMap)
			res += sizeof(fleetKV) + fleetKV.second.heap_size() + 2 * sizeof(size_t);
		for(auto const & reportKV: reportMap)
			res += sizeof(reportKV) + reportKV.second.heap_size() + 2 * sizeof(size_t);
	}

	typedef boost::shared_mutex Mutex;
	mutable Mutex planetsFleetsReportsmutex;
	mutable Mutex playersMutex;

	Universe():
		nextPlayerID(0),
		nextFleetID(0),
		nextEventID(0),
		nextFightID(0),
		roundCount(0),
		roundDuration(0.)
	{
	}

	Universe(Universe const& other):
		playerMap(other.playerMap),
		planetMap(other.planetMap),
		fleetMap(other.fleetMap),
		reportMap(other.reportMap),
		nextPlayerID(other.nextPlayerID),
		nextFleetID(other.nextFleetID),
		nextEventID(other.nextEventID),
		nextFightID(other.nextFightID),
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
		playerMap.swap(other.playerMap);
		planetMap.swap(other.planetMap);
		fleetMap.swap(other.fleetMap);
		reportMap.swap(other.reportMap);
		std::swap(nextPlayerID, other.nextPlayerID);
		std::swap(nextFleetID, other.nextFleetID);
		std::swap(nextEventID, other.nextEventID);
		std::swap(nextFightID, other.nextFightID);
		std::swap(roundCount, other.roundCount);
		std::swap(roundDuration, other.roundDuration);
	}
};


struct Signal
{
	Player::ID playerID;
	//Coord planetCoord;
	Event event;

	Signal(Player::ID player,
	       //Coord planet,
	       Event const& event_):
		playerID(player),
		//planetCoord(planet),
		event(event_)
	{
	}
};



void construct(Universe& univ);

Player::ID createPlayer(Universe& univ, std::string const& login, std::string const& password);

void saveToStream(Universe const& univ, std::ostream& out);
void loadFromStream(std::istream& in, Universe& univ);

bool canBuild(Planet const& planet, Ship::Enum type, size_t number);

bool canBuild(Planet const& planet, Building::Enum type);

void addTask(Planet& planet, size_t roundCount, Building::Enum building);

void addTask(Planet& planet, size_t roundCount, Ship::Enum ship, size_t number);

bool canBuild(Planet const& planet, Cannon::Enum type, size_t number);

void addTask(Planet& planet, size_t roundCount, Cannon::Enum cannon, size_t number);

bool canStop(Planet const& planet, Building::Enum type);

void stopTask(Planet& planet, PlanetTask::Enum tasktype, Building::Enum building);

//! Gere l'écoulement du temps sur la planète.
//! Peut modifier la liste dse flotte et des planètes
void planetRound(Universe& univ, Planet& planet, std::vector<Signal>& signals);

//! Gere l'écoulement du temps sur la flotte.
//! Peut modifier la liste des flottes et des planètes
void fleetRound(Universe& univ,
                Fleet& fleet,
                std::vector<Signal>& signals,
                std::map<Player::ID, size_t> const& playersPlanetCount);

void gather(Fleet& fleet, Fleet const& otherFleet);

bool canMove(Fleet const& fleet, Coord const& coord);

void addTask(Fleet& fleet, size_t roundCount, Coord const& coord);

bool canHarvest(Fleet const& fleet, Planet const& planet);

void addTaskHarvest(Fleet& fleet, size_t roundCount, Planet const& planet);

bool canColonize(Fleet const& fleet, Planet const& planet);

void addTaskColonize(Fleet& fleet, size_t roundCount, Planet const& planet);

bool canDrop(Fleet const& fleet, Planet const& planet);

void drop(Fleet& fleet, Planet& planet);

void eraseAccount(Universe& univ, Player::ID pid);

#endif //_BTA_MODEL_
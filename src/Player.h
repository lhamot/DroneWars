#ifndef __DRONEWARS_PLAYER__
#define __DRONEWARS_PLAYER__

#include "stdafx.h"

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
		ar& blocklyCode_& code_& failCount_& lastError_;
	}

	std::string blocklyCode_;
	std::string code_;
	size_t failCount_;
	std::string lastError_;

public:
	CodeData(): failCount_(0) {}

	std::string const& getBlocklyCode() const {return blocklyCode_;}

	std::string const& getCode() const {return code_;}

	size_t getFailCount() const {return failCount_;}

	std::string const& getLastError() const {return lastError_;}

	void setCode(std::string const& newCode)
	{
		code_ = newCode;
		failCount_ = 0;
		lastError_.clear();
	}

	void setBlocklyCode(std::string const& newCode)
	{
		blocklyCode_ = newCode;
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
	void serialize(Archive& ar, const unsigned int) //version
	{
		ar& id& login& password& fleetsCode& planetsCode&
		eventList& tutoDisplayed& mainPlanet;
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
	std::map<std::string, size_t> tutoDisplayed;
	Coord mainPlanet;

	Player(ID i, std::string const& lg, std::string const& pass): id(i), login(lg), password(pass) {}

	size_t getTutoLevel(char const* const tag) const
	{
		auto iter = tutoDisplayed.find(tag);
		return iter == tutoDisplayed.end() ?
		       0 :
		       iter->second;
	}
};

static char const* const CoddingLevelTag = "BlocklyCodding";

#endif //__DRONEWARS_PLAYER__
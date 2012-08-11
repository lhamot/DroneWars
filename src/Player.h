#ifndef __DRONEWARS_PLAYER__
#define __DRONEWARS_PLAYER__

#include "stdafx.h"

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


#endif //__DRONEWARS_PLAYER__
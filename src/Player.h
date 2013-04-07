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

	typedef int32_t Value;
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

inline bool operator != (Coord const& a, Coord const& b)
{
	return (a == b) == false;
}

struct Direction
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

	Direction(): X(0), Y(0), Z(0) {}

	Direction(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};

inline bool operator == (Direction const& a, Direction const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}

inline bool operator != (Direction const& a, Direction const& b)
{
	return (a == b) == false;
}


static size_t const MaxStringSize = 256;

struct CodeData
{
	enum Target
	{
	  Planet,
	  Fleet
	};

	size_t id;
	std::string code;
	std::string blocklyCode;
	std::string lastError;

	CodeData(): id(0) {}

	CodeData(CodeData const& other):
		id(other.id),
		code(other.code),
		blocklyCode(other.blocklyCode),
		lastError(other.lastError)
	{
	}

	CodeData& operator=(CodeData other)
	{
		std::swap(id, other.id);
		std::swap(code, other.code);
		std::swap(blocklyCode, other.blocklyCode);
		std::swap(lastError, other.lastError);
	}
};


struct Player
{
	static size_t const MaxCodeSize = 32 * 1024;          //TOTO: Rien a faire là
	static size_t const MaxBlocklySize = MaxCodeSize * 8; //TOTO: Rien a faire là

	typedef size_t ID;
	static ID const NoId = 0;
	ID id;
	std::string login;
	std::string password;
	Coord mainPlanet;
	uint64_t score;

	Player(ID i, std::string const& lg, std::string const& pass):
		id(i), login(lg), password(pass)
	{
	}
};


static char const* const CoddingLevelTag = "BlocklyCodding";

#endif //__DRONEWARS_PLAYER__
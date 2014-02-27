//! @file
//! @author Loïc HAMOT

#ifndef __DRONEWARS_PLAYER__
#define __DRONEWARS_PLAYER__

#include "stdafx.h"

//! Nombre de skill
static size_t const SkillCount = 13;

//! Coordonée entière tridimentionelle
struct Coord
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& X& Y& Z;
	}

	typedef int8_t Value; //!< Type pour une Coordonée monodimensionnelle
	Value X;              //!< Coordonée en X
	Value Y;              //!< Coordonée en Y
	Value Z;              //!< Coordonée en Z

	//! Constructeur par defaut
	Coord(): X(0), Y(0), Z(0) {}

	//! Constructeur prenant 3 valeurs X, Y et Z
	Coord(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};


//! Test d'égalité de deux Coord
inline bool operator == (Coord const& a, Coord const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}


//! Test d'inégalité de deux Coord
inline bool operator != (Coord const& a, Coord const& b)
{
	return (a == b) == false;
}


//! Direction tridimentionelle en valeur entière
struct Direction
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& X& Y& Z;
	}

	typedef int8_t Value; //!< Type pour une C
	Value X;			  //!< Composante X
	Value Y;			  //!< Composante Y
	Value Z;			  //!< Composante Z

	//! Constructeur par defaut : Vecteur nulle
	Direction(): X(0), Y(0), Z(0) {}

	//! Constructeur
	Direction(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};


//! Test d'égalité de deux Direction
inline bool operator == (Direction const& a, Direction const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}


//! Test d'inégalité de deux Direction
inline bool operator != (Direction const& a, Direction const& b)
{
	return (a == b) == false;
}


static size_t const MaxStringSize = 256; //!< Taille max des string courte


//! Script de planète ou flotte d'un joueur
struct CodeData
{
	//! @brief Taile max du code lua
	static size_t const MaxCodeSize = 32768;
	//! @brief Taille max du code blockly
	static size_t const MaxBlocklySize = MaxCodeSize * 8;

	//! Planète ou Flotte
	enum Target : uint8_t
	{
	  Planet,
	  Fleet,
	  Undefined
	};

	size_t   id = 0;         //!< ID de la version du code source lua
	uint32_t playerId = 0;   //!< Propriétaire
	Target   target = Undefined;  //!< Planète ou Flotte
	std::string code;        //!< Code lua
	std::string blocklyCode; //!< Code blockly (XML)
	std::string lastError;   //!< ernière erreur, si applicable
	time_t blocklyCodeDate = 0;
	time_t codeDate = 0;

	//! Constructeur par defaut
	CodeData() {}

	//! Operateur de copie
	CodeData& operator=(CodeData other)
	{
		std::swap(id, other.id);
		std::swap(playerId, other.playerId);
		std::swap(target, other.target);
		code.swap(other.code);
		blocklyCode.swap(other.blocklyCode);
		lastError.swap(other.lastError);
		std::swap(blocklyCodeDate, other.blocklyCodeDate);
		std::swap(codeDate, other.codeDate);
		return *this;
	}
};

struct Player;

//! Description d'une alliance
struct Alliance
{
	typedef uint32_t ID;      //!< Type d'identifiant
	static ID const NoId = 0; //!< Valeur indiquant l'absence d'alliance
	ID id;                    //!< Identifiant unique
	uint32_t masterID;        //!< Identifiant du propriétaire
	std::string name;         //!< Nom
	std::string description;  //!< Description
	std::string masterLogin;  //!< Login du propriétaire (Pas stoké dans SGBD)
	Player* master = nullptr; //!< Pour les script lua uniquement

	//! Constructeur
	Alliance(Alliance::ID id,
	         uint32_t playerID,
	         std::string const& name,
	         std::string const& description,
	         std::string const& masterLogin):
		id(id),
		masterID(playerID),
		name(name),
		description(description),
		masterLogin(masterLogin)
	{
	}
};


//! Donnée d'un joueur
struct Player
{
	//! Un uint8_t pour chaque Skill
	typedef boost::array<uint8_t, SkillCount> SkillTab;
	typedef uint32_t ID;      //!< Type d'identifiant
	static ID const NoId = 0; //!< Valeur indiquant l'absence de joueur
	ID id;                    //!< Identifiant unique
	std::string login;        //!< Login
	Coord mainPlanet;         //!< Coordonées de la planète principale
	uint64_t score;           //!< Scroe
	Alliance::ID allianceID;  //!< ID de l'alliance, ou Alliance::NoID
	std::string allianceName; //!< Nom de l'alliance (Pas stoké dans le SGBD)
	uint32_t experience;      //!< Experience
	uint32_t skillpoints;     //!< Points de competances
	SkillTab skilltab;        //!< Niveau du joueur dans chaque competance
	size_t unreadMessageCount = 0; //!< Nombre de message en attente de lecture
	Alliance* alliance = nullptr; //!< Pour les script lua uniquement
	size_t planetCount = 0;   //!< Pour les script lua uniquement
	size_t fleetCount = 0;    //!< Pour les script lua uniquement

	//! Constructeur
	Player(ID i, std::string const& lg):
		id(i), login(lg), allianceID(0),
		experience(0), skillpoints(0)
	{
		skilltab.assign(0);
	}
};


//! tag utiliser pour identifier le niveau dans le tutoriel avec blockly
static char const* const CoddingLevelTag = "BlocklyCodding";

#endif //__DRONEWARS_PLAYER__